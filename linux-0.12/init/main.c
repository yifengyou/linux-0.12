/*
 *  linux/init/main.c
 *
 *  (C) 1991  Linus Torvalds
 */
// #define __LIBRARY__ 为了包括定义在 unistad.h 中的内嵌汇编代码等信息。
#define __LIBRARY__
#include <unistd.h>
#include <time.h>

/*
 * we need this inline - forking from kernel space will result
 * in NO COPY ON WRITE (!!!), until an execve is executed. This
 * is no problem, but for the stack. This is handled by not letting
 * main() use the stack at all after fork(). Thus, no function
 * calls - which means inline code for fork too, as otherwise we
 * would use the stack upon exit from 'fork()'.
 *
 * Actually only pause and fork are needed inline, so that there
 * won't be any messing with the stack from main(), but we define
 * some others too.
 */

 // 实际上只有 pause 和 fork 需要使用内联方式，以保证从 main() 中不会弄乱
 // 堆栈，但我们同时定义了其他一些函数。

 // _syscall0() 是 unistd.h 中的内嵌宏代码。
static inline _syscall0(int,fork)
static inline _syscall0(int,pause)
static inline _syscall1(int,setup,void *,BIOS)
static inline _syscall0(int,sync)

#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <asm/system.h>
#include <asm/io.h>

#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include <linux/fs.h>

#include <string.h>

static char printbuf[1024];				//静态字符串数组，用作内核显示信息的缓存。

extern char *strcpy();
extern int vsprintf();
extern void init(void);				//函数原型，初始化
extern void blk_dev_init(void);			//块设备初始化子 blk_drv/ll_re_blk.c
extern void chr_dev_init(void);			//字符设备初始化 chr_drv/tty_io.c
extern void hd_init(void);					//硬盘初始化	blk_drv/hd.c
extern void floppy_init(void);			//软驱初始化 blk_drv/floppy.c
extern void mem_init(long start, long end);			//内存管理初始化 mm/memory.c
extern long rd_init(long mem_start, int length);	//虚拟盘初始化 blk_drv/ramdisk.c
extern long kernel_mktime(struct tm * tm);		//计算系统开机启动时间(秒)

// 内核专用 sprintf() 函数。产生格式化信息并输出到指定缓冲区 str 中。
static int sprintf(char * str, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsprintf(str, fmt, args);
	va_end(args);
	return i;
}

/*
 * This is set up by the setup-routine at boot-time
 */
 // 这些数据由内核引导期间的 setup.s 程序设置。
#define EXT_MEM_K (*(unsigned short *)0x90002)	//1MB 以后的扩展内存大小(KB)
#define CON_ROWS ((*(unsigned short *)0x9000e) & 0xff)	//选定的控制台屏幕行列数
#define CON_COLS (((*(unsigned short *)0x9000e) & 0xff00) >> 8)
#define DRIVE_INFO (*(struct drive_info *)0x90080)	//硬盘参数表32字节内容
#define ORIG_ROOT_DEV (*(unsigned short *)0x901FC)	//根文件系统所在设备号
#define ORIG_SWAP_DEV (*(unsigned short *)0x901FA)	//交换文件所在设备号

/*
 * Yeah, yeah, it's ugly, but I cannot find how to do this correctly
 * and this seems to work. I anybody has more info on the real-time
 * clock I'd be interested. Most of this was trial and error, and some
 * bios-listing reading. Urghh.
 */
// 这段宏读取 CMOS 实时时钟数据。outb_p 和 inb_p 是 include/asm/io.h 中定义的
// 端口输入输出宏。
#define CMOS_READ(addr) ({ \
outb_p(0x80|addr,0x70); \
inb_p(0x71); \
})

//将 BCD 码转换成二进制数值。
#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

// CMOS 的访问时间很慢。为了减小时间误差，在读取了下面循环中所有数值后，若此时 CMOS
// 中秒值发生了变化，则重新读取。这样能控制误差在1s内。
static void time_init(void)
{
	struct tm time;

	do {
		time.tm_sec = CMOS_READ(0);
		time.tm_min = CMOS_READ(2);
		time.tm_hour = CMOS_READ(4);
		time.tm_mday = CMOS_READ(7);
		time.tm_mon = CMOS_READ(8);
		time.tm_year = CMOS_READ(9);
	} while (time.tm_sec != CMOS_READ(0));
	BCD_TO_BIN(time.tm_sec);
	BCD_TO_BIN(time.tm_min);
	BCD_TO_BIN(time.tm_hour);
	BCD_TO_BIN(time.tm_mday);
	BCD_TO_BIN(time.tm_mon);
	BCD_TO_BIN(time.tm_year);
	time.tm_mon--;		//ti_mon 中的月份范围是 0 ~ 11。
	startup_time = kernel_mktime(&time);	//计算开机时间。
}

static long memory_end = 0;			//机器所具有的物理内存容量
static long buffer_memory_end = 0;		//高速缓冲区末端地址
static long main_memory_start = 0;		//主内存开始的位置
static char term[32];			//终端设置字符串

// 读取并执行 /etc/rc 文件时所使用的命令行参数和环境参数。
static char * argv_rc[] = { "/bin/sh", NULL };		//调用执行程序时参数的字符串数值。
static char * envp_rc[] = { "HOME=/", NULL ,NULL };//调用执行程序时的环境字符串数值。

//运行登录shell时所使用的命令行和环境参数。
//argv[0]中的字符“-”是传递shell程序sh的一个标示位，通过这个标示位，sh程序会作为
//shell程序执行。
static char * argv[] = { "-/bin/sh",NULL };
static char * envp[] = { "HOME=/usr/root", NULL, NULL };

//用于存放硬盘参数表
struct drive_info { char dummy[32]; } drive_info;

//内核初始化主程序。
// 这里是 void，因为在 head.s 就是这么假设的(把 main 的地址压入堆栈的时候)。
void main(void)		/* This really IS void, no error here. */
{			/* The startup routine assumes (well, ...) this */
/*
 * Interrupts are still disabled. Do necessary setups, then
 * enable them
 */
/*
 * 此时中断还被禁止的，做完必要的设置后就将其开启。没有完备的中断响应
 * 又或者说不可或缺的中断响应，系统一跑起来无法正常响应中断，基本JJ了，
 * 又或者做法是不去处理，当然还是等准备好了再开启比较合适
 */
 	ROOT_DEV = ORIG_ROOT_DEV;		// ROOT_DEV 定义在 fs/super.c 中。
 	SWAP_DEV = ORIG_SWAP_DEV;		// SWAP_DEV 定义在 mm/swap.c 中。
	sprintf(term, "TERM=con%dx%d", CON_COLS, CON_ROWS);
	envp[1] = term;
	envp_rc[1] = term;
 	drive_info = DRIVE_INFO;

	// 接着根据机器物理内存容量设置高速缓冲区和主内存区的位置和范围。
	// 高速缓冲末端地址 > buffer_memory_end
	// 机器内存容量 > memory_end
	// 主内存开始地址 > main_memory_start
	memory_end = (1<<20) + (EXT_MEM_K<<10); //1M + 扩展内存大小
	memory_end &= 0xfffff000;			//忽略不到4K(1页)的内存数
	if (memory_end > 16*1024*1024)
		memory_end = 16*1024*1024;
	if (memory_end > 12*1024*1024)
		buffer_memory_end = 4*1024*1024;
	else if (memory_end > 6*1024*1024)
		buffer_memory_end = 2*1024*1024;
	else
		buffer_memory_end = 1*1024*1024;
	main_memory_start = buffer_memory_end;	//主内存开始地址 = 高速缓冲区结束地址
#ifdef RAMDISK	//如果定义了虚拟盘，则主内存还得相应减少
	main_memory_start += rd_init(main_memory_start, RAMDISK*1024);
#endif

// 以下是内核进行所有方面的初始化工程。
	mem_init(main_memory_start,memory_end);		//主内存区初始化
	trap_init();		//陷阱门初始化
	blk_dev_init();	//块设备初始化
	chr_dev_init();	//字符设备初始化
	tty_init();		//tty初始化
	time_init();	//设置开机启动时间
	sched_init();	//调度程序初始化
	buffer_init(buffer_memory_end);	//缓冲管理初始化，建内存链表等
	hd_init();	//硬盘初始化
	floppy_init();	//软驱初始化
	sti();		//开启中断
//下面过程通过在堆栈中设置的参数，利用中断返回指令启动任务 0 执行。
	move_to_user_mode();
	if (!fork()) {		/* we count on this going ok */
		init();
	}
/*
 *   NOTE!!   For any other task 'pause()' would mean we have to get a
 * signal to awaken, but task0 is the sole exception (see 'schedule()')
 * as task 0 gets activated at every idle moment (when no other tasks
 * can run). For task0 'pause()' just means we go check if some other
 * task can run, and if not we return here.
 */
/*
 * 主要!! 对于任何其他的任务，“pause()”将意味着我们必须等待收到yoga信号才会返回
 * 就绪态，但任务 0 是唯一例外的情况(参见“schedule()”)，因为任务 0 在任何空闲时
 * 间里都会被激活,因此对于任务 0 'pause()'仅意味着我们返回来查看是否有其他任务可
 * 以运行，如果没有的话，我们就在这里一直循环执行‘pause()’。
 */

 // pause()系统调用会把任务 0 转换成可中断等待状态，再执行调度函数。但是调度函数
 // 发现系统中没有其他程序可以运行就会切换到任务 0，而不依赖任务 0 的状态。
	for(;;)
		__asm__("int $0x80"::"a" (__NR_pause):"ax");//执行系统调用pause()
}

//产生格式化信息并输出到标准输出设备stdout(1)，这里是指屏幕上显示。
//write()中 第一个参数 1 -> stdout，表示将缓冲区的内容输出到标准设备。
static int printf(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	write(1,printbuf,i=vsprintf(printbuf, fmt, args));
	va_end(args);
	return i;
}

// init()函数运行在任务 0 第一次创建的子程序（ 任务 1）中。首先对第一个将要执行的程
//序(shell)的环境进行初始化，然后以登录shell方式加载该程序并执行。
void init(void)
{
	int pid,i;

// setup() 是一个系统调用。用来读取硬盘参数包括分区表信息并加载虚拟盘(如果存在的话)
//和安装根文件系统设备。
	setup((void *) &drive_info);

// 下面以读写方式打开“/dev/tty1”,它对应终端控制台。因为这是第一次打开文件操作，
//所以产生的文件句柄号(文件描述符)肯定是 0。
	(void) open("/dev/tty1",O_RDWR,0);
	(void) dup(0);		//复制句柄，产生句柄1号 -- stdout 标准输出设备
	(void) dup(0);		//复制句柄，产生句柄2号 -- stderr 标准出错输出设备

//下面打印缓冲区块数和总字节数，每块1024字节，已经主内存区空闲内存字节数。
	printf("%d buffers = %d bytes buffer space\n\r",NR_BUFFERS,
		NR_BUFFERS*BLOCK_SIZE);
	printf("Free mem: %d bytes\n\r",memory_end-main_memory_start);

//下面通过fork()用于创建一个子进程(任务 2)。对于被创建的子进程，fork()将返回 0 值，对于
//原进程则返回子进程的进程号 pid。
//首先子进程(任务 2)关闭句柄 0 (stdin)，以只读方式打开 /etc/rc 文件，并使用execve()函
//数将进程自身替换成 /bin/sh 程序，然后执行 /bin/sh 程序。所携带的参数和环境变量分别由
//argv_rc 和 envp_rc 数组给出。关闭句柄 0 立即打开 /etc/rc 文件 是为了把 stdin 重定向
//到 /etc/rc 文件。这样 shell 程序 /bin/sh 就可以运行 /etc/rc 中设置的命令了。由于这里
//的 sh 的运行方式是非交互式的，所以执行完rc文件中的命令后立即退出。
	if (!(pid=fork())) {
		close(0);
		if (open("/etc/rc",O_RDONLY,0))
			_exit(1);
		execve("/bin/sh",argv_rc,envp_rc);
		_exit(2);
	}

//下面是父进程(1)执行的语句。wait()等待子进程停止或终止，返回值是相应的子进程的进程号。
//这里是父进程在等待子进程(进程 2)退出。&i 是存放返回状态信息的位置。
	if (pid>0)
		while (pid != wait(&i))
			/* nothing */;

//如果执行到这里则说明刚才创建的子进程已经结束了。在下面循环中首先再创建一个子进程。
//创建，退出，打印信息，重复循环下去，一直会在这个大循环里。
	while (1) {
//如果出错，则显示“初始化创建子程序失败”信息并继续执行。
		if ((pid=fork())<0) {
			printf("Fork failed in init\r\n");
			continue;
		}
//新的子进程，关闭句柄(0,1,2)，新创建一个会话并设置进程组号，然后重新打开 /dev/tty0 作为
// stdin，并复制成 stdout 和 stderr。再次执行/bin/sh，这次使用另一套参数和环境变量组。
		if (!pid) {
			close(0);close(1);close(2);
			setsid();
			(void) open("/dev/tty1",O_RDWR,0);
			(void) dup(0);
			(void) dup(0);
			_exit(execve("/bin/sh",argv,envp));
		}
		// 然后父进程再次运行 wait() 等待。
		while (1)
			if (pid == wait(&i))
				break;
		printf("\n\rchild %d died with code %04x\n\r",pid,i);
		sync();				//同步操作，刷新缓冲区
	}
	_exit(0);	/* NOTE! _exit, not exit() */
	//_exit 和 exit 都能用于正常终止一个函数。但_exit()直接是一个 sys_exit的系统调用，而
	//exit() 则是普通函数库中的一个函数。它会先进行一些清除操作，例如调用执行各终止处理程序，
	//关闭所有标准IO等，然后调用sys_exit。
}
