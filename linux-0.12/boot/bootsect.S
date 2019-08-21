!
! SYS_SIZE is the number of clicks (16 bytes) to be loaded.
! SYS_SIZE 是要加载的系统模块长度（单位是节，每节有16字节）。
! 0x3000 is 0x30000 bytes = 196kB, more than enough for current
! 即 0x30000 bytes = 192 KB（估算，以1000为单位，则是196KB），对于当前的
! 版本空间已足够了。
! 例如，当该值为 0x8000 时，表示内核最大为512KB。
! versions of linux
!
#include <linux/config.h>
!
! 该头文件里定义了内核用到的一些常数符号和 Linus 自己使用的默认硬盘默认参数块。
! 例如:
! DEF_SYSSIZE = 0X3000 系统模块长度
! DEF_INITSEG = 0x9000 该程序将要移动到的目的段位置
! DEF_SETUPSEG = 0x9020 setup程序代码的段位置
! DEF_SYSSEG = 0x1000 从磁盘加载系统模块到内存的段位置
!
SYSSIZE = DEF_SYSSIZE
!
!	bootsect.s		(C) 1991 Linus Torvalds
!	modified by Drew Eckhardt
!
! bootsect.s is loaded at 0x7c00 by the bios-startup routines, and moves
! iself out of the way to address 0x90000, and jumps there.
!
! It then loads 'setup' directly after itself (0x90200), and the system
! at 0x10000, using BIOS interrupts. 
!
! NOTE! currently system is at most 8*65536 bytes long. This should be no
! problem, even in the future. I want to keep it simple. This 512 kB
! kernel size should be enough, especially as this doesn't contain the
! buffer cache as in minix
!
! The loader has been made as simple as possible, and continuos
! read errors will result in a unbreakable loop. Reboot by hand. It
! loads pretty fast by getting whole sectors at a time whenever possible.

! 以下是前面这些文字的翻译：
! 	bootsect.s 	(C) 1991 Linus Torvalds 版权所有
!	Drew Eckhardt 修改
!
!  bootsect.s 被BIOS启动子程序加载至 0x7c00 (31KB)处，并将自己移到了地
! 址 0x90000(576KB)处，并跳转至那里。
!
! 它然后使用BIOS 中断将'setup'直接加载到自己的后面(0x90200)(576.5KB)，
! 并将 system 加载到地址 0x10000 处。
!
! 注意! 目前的内核系统最大长度限制为(8*65536)(512KB)字节，即使是在将来
! 这也应该没有问题的。我想让它保持简单明了。这样512KB的最大内核长度应该
! 足够了，尤其是这里没有像MINIX中一样包含缓冲区高速缓冲。
!
! 加载程序已经做得够简单了，所以持续的读操作出错将导致死循环。只能手工重启。
! 只要可能，通过一次读取所有的扇区，加载过程可以做的很快的。

.globl begtext, begdata, begbss, endtext, enddata, endbss
.text
begtext:
.data
begdata:
.bss
begbss:
.text

SETUPLEN = 4				! nr of setup-sectors 
							! setup 占用的磁盘扇区数
BOOTSEG  = 0x07c0			! original address of boot-sector
							! bootsect 代码所在的原地址（被BIOS子程序加载至此处）
INITSEG  = DEF_INITSEG			! we move boot here - out of the way
								! bootsect将要移动到的目的段位置，为了避开系统模块占用处
SETUPSEG = DEF_SETUPSEG			! setup starts here
								! setup程序代码从该段位置开始
SYSSEG   = DEF_SYSSEG			! system loaded at 0x10000 (65536).
								! system 模块将被加载到 0x10000
ENDSEG   = SYSSEG + SYSSIZE		! where to stop loading
								! 停止加载的段地址

! ROOT_DEV & SWAP_DEV are now written by "build".
! 根文件系统设备号 ROOT_DEV 和 交换设备号 SWAP_DEV 现在由 tools 目录下的 build 程序写入。
! 设备号具体值的含义如下：
! 设备号=主设备号*256 + 次设备号（也即 dev_no = ( major <<8 ) + minor ）
! （主设备号：1-内存,2-磁盘,3-硬盘,4-ttyx,5-tty,6-并行口,7-非命名管道）
! 0x300 - /dev/hd0 - 代表整个第 1 个硬盘；
! 0x301 - /dev/hd1 - 第 1 个盘的第 1 个分区；
! …
! 0x304 - /dev/hd4 - 第 1 个盘的第 4 个分区；
! 0x305 - /dev/hd5 - 代表整个第 2 个硬盘盘；
! 0x306 - /dev/hd6 - 第 2 个盘的第 1 个分区；
! …
! 0x309 - /dev/hd9 - 第 2 个盘的第 4 个分区；
! 从 Linux 内核 0.95 版后已经使用与现在相同的命名方法了。
ROOT_DEV = 0	! 根文件系统设备使用与系统引导时同样的设备。
SWAP_DEV = 0	! 交换设备使用与引导时同样的设备。

entry start 	! 告知链接程序，程序从 start 标号处开始执行。
start:
	! 将自身(bootsect)从当前段位置(即 0x7c00 )移动到 0x90000 处，共256个字节。
	mov	ax,#BOOTSEG
	mov	ds,ax
	mov	ax,#INITSEG
	mov	es,ax
	mov	cx,#256
	sub	si,si 		! 源地址 ds:si = 0x07c0:0x0000;
	sub	di,di 		! 目标地址 es:di = 0x9000:0x0000;
	rep
	movw			! 此处结束后，代码已经成功移动到0x9000,虽然go标识就
					! 在jump下面，但代码已完成转移，所以需要通过段间跳转，跳转到0x9000:go处。
	jmpi	go,INITSEG 		! 段间跳转(Jump Intersegment)。INITSEG 为跳转到的段地址，
							! 标号 go 是段内偏移地址;
	!
	! 从 go 处开始，CPU 在已移动到0x90000处的代码中继续执行。
	! 以下几行代码设置了几个段寄存器，包括栈寄存器 ss 和 sp。
	! 注意：实际上 BIOS 把引导扇区加载到0x7c00处并把执行权交给引导程序时，ss = 0x00，sp = 0xfffe；
	! 这里的 push ax 的作用是想暂时把段值保留在栈中，然后等下面执行完判断磁道扇区数后再弹出栈，并
	! 给 fs 和 gs 赋值，但接下来的2个指令改变了栈段位置,所以需要先恢复到原来的栈段，再执行弹出栈
	! 操作。这更像是一个bug，改正： 去掉 push ax，把之后的 pop ax 改成 mov ax,cs 。
	! 个人觉得 可以把 push ax 放在接下来2个指令(即改变完栈段)之后。
go:	mov	ax,cs		
	mov	dx,#0xfef4	! arbitrary value >>512 - disk parm size
					! 栈指针要远大于512字节偏移(即 0x90200 )处都可以;
					! 一般setup程序大概占用4个扇区 所以 sp 要大于 
					! ( 0x90200 +0x200 * 4 + 堆栈大小)。这里 sp 被设置成
					! 了 0x9ff00 - 12(参数表长度)，即 sp = 0xfef4。
	mov	ds,ax
	mov	es,ax
	push	ax		! 临时保存段值( 0x9000 )，供 后面 使用。

	mov	ss,ax		! put stack at 0x9ff00 - 12.
	mov	sp,dx
/*
 *	Many BIOS's default disk parameter tables will not 				'
 *	recognize multi-sector reads beyond the maximum sector number
 *	specified in the default diskette parameter tables - this may
 *	mean 7 sectors in some cases.
 *
 *	Since single sector reads are slow and out of the question,
 *	we must take care of this by creating new parameter tables
 *	(for the first disk) in RAM.  We will set the maximum sector
 *	count to 18 - the most we will encounter on an HD 1.44.  
 *
 *	High doesn't hurt.  Low does.
 *
 *	Segments are as follows: ds=es=ss=cs - INITSEG,
 *		fs = 0, gs = parameter table segment
 */
/*
 * 对于多扇区读操作所读的扇区数超过默认磁盘参数表中指定的最大扇区数时，很多 BIOS
 * 将不能进行正确识别。在某些情况下是7个扇区。
 *
 * 由于单扇区读操作太慢，不予考虑。我们必须通过在内存中重创建新的参数表(为第1个驱动器)
 * 来解决这个问题。我们将把其中最大扇区数设置为18，即在 1.44MB 磁盘上会碰到的最大值。
 *
 * 数值大不会出问题，但太小就不行了。
 *
 * 段寄存器将被设置成：ds = es = ss = cs 都为 INITSEG (0x9000),
 * fs = 0,gs = 参数表所在段值。
 */

 ! BIOS 设置的中断 0x1e 的中断向量值是软驱参数表地址。该向量值位于内存 0x1e * 4 = 0x78 处。
 ! 这段代码首先从内存 0x0000:0x0078 处复制原软驱参数表到 0x9000:0xfef4 处，然后修改表中的每
 ! 磁道最大扇区数为 18。
	push	#0			! 置段寄存器 fs = 0
	pop	fs 				! fs:bx 指向存有软驱参数表地址处(指针的指针)。
	mov	bx,#0x78		! fs:bx is parameter table address
	! seg fs只影响接下来的一条语句，表示下一条语句的操作数在fs段寄存器所指的段中。这里即把 
	! fs:bx 所指内存位置处的表地址放到 gs:si 中作为源地址，寄存器对 es:di = 0x9000:0xfef4 
	! 处作为目的地址。 
	seg fs
	lgs	si,(bx)			! gs:si is source

	mov	di,dx			! es:di is destination 		! dx = 0xfef4
	mov	cx,#6			! copy 12 bytes
	cld 				! 清方向标志。复制时指针递增。

	rep 				! 复制12字节的软驱参数表到0x9000:0xfef4 处。
	seg gs
	movw

	mov	di,dx			! es:di 指向新表，然后修改表中偏移4处的最大扇区数。
	movb	4(di),*18		! patch sector count

	seg fs 				! 让中断向量 0x1e 的值指向新表。
	mov	(bx),di
	seg fs
	mov	2(bx),es

	pop	ax 				!ax = 0x9000
	mov	fs,ax
	mov	gs,ax
	
	xor	ah,ah			! reset FDC ! 复位软盘控制器，让其采用新参数。
	xor	dl,dl 			! dl = 0; 第1个软驱
	int 	0x13	

! load the setup-sectors directly after the bootblock.
! Note that 'es' is already set up.
! 在 bootsect 程序块后紧跟着加载 setup 模块的代码数据。
! 在移动代码时，es的值已被设置好。 
! 利用BIOS 中断INT 0x13 将 setup 模块从磁盘第 2 个扇区
! 开始读到0x90200 开始处，共读4 个扇区。如果读出错，显示磁盘上出错扇区位置，则复位驱动器，并
! 重试，没有退路。INT 0x13 的使用方法如下：
! 读扇区：
! ah = 0x02 - 读磁盘扇区到内存；al = 需要读出的扇区数量；
! ch = 磁道(柱面)号的低8 位； cl = 开始扇区(0-5 位)，磁道号高2 位(6-7)；
! dh = 磁头号； dl = 驱动器号（如果是硬盘则要置位7）；
! es:bx 指向数据缓冲区； 如果出错则 CF 标志置位，并且ah中是出错码。
load_setup:
	xor	dx, dx			! drive 0, head 0
	mov	cx,#0x0002		! sector 2, track 0
	mov	bx,#0x0200		! address = 512, in INITSEG
	mov	ax,#0x0200+SETUPLEN	! service 2, nr of sectors
	int	0x13			! read it
	jnc	ok_load_setup 	! ok - continue

	push	ax			! dump error code	!显示出错信息。出错码入栈。
	call	print_nl 	! 屏幕光标回车
	mov	bp, sp 			! ss:bp 指向欲显示的字(word)
	call	print_hex	! 显示十六进制值
	pop	ax	
	
	xor	dl, dl			! reset FDC !复位磁盘控制器，重试。
	xor	ah, ah
	int	0x13
	j	load_setup 		! j 即 jmp

ok_load_setup:

! Get disk drive parameters, specifically nr of sectors/track
! 这段代码取磁盘驱动器的参数，特别是每道的扇区数量，并保存在位置 sectors 处。
! 取磁盘驱动器参数 INT 0x13 调用格式和返回信息如下：
! ah = 0x08 dl = 驱动器号（如果是硬盘则要置位7 为1）。
! 返回信息：
! 如果出错则 CF 置位，并且 ah = 状态码。
! ah = 0， al = 0， bl = 驱动器类型（AT/PS2）
! ch = 最大磁道号的低 8 位，cl = 每磁道最大扇区数(位 0-5)，最大磁道号高 2 位(位 6-7)
! dh = 最大磁头数， dl = 驱动器数量，
! es:di -> 软驱磁盘参数表。

	xor	dl,dl
	mov	ah,#0x08		! AH=8 is get drive parameters
	int	0x13
	xor	ch,ch
	seg cs 				! 因为原本就处于代码段，可以不用这句
	mov	sectors,cx		! 保存每磁道扇区数。
	mov	ax,#INITSEG
	mov	es,ax 			! 因为上面取磁盘参数中断改了es的值，这里需要改回来

! Print some inane message
! 在显示一些信息('Loading' + 回车 + 换行，共 9 个字符)。

	mov	ah,#0x03		! read cursor pos 	! 读光标位置
	xor	bh,bh			! bh 页号
	int	0x10
	
	mov	cx,#9 			! cx 显示的字符串个数
	mov	bx,#0x0007		! page 0, attribute 7 (normal)
						! bh = 页号，bl = 字符属性
	mov	bp,#msg1
	mov	ax,#0x1301		! write string, move cursor
						! ah = 0x13 - 显示字符串。al = 放置光标的方式及规定属性。0x01 - 表示使用bl中的属性值，光标
						! 停在字符串末尾
	int	0x10
! ok, we've written the message, now						'

! we want to load the system (at 0x10000)
! 现在开始将 system 模块加载到 0x10000 ( 64K )处。

	mov	ax,#SYSSEG
	mov	es,ax		! segment of 0x010000
	call	read_it ! 读磁盘上 system 模块，es 为输入参数。
	call	kill_motor ! 关闭驱动器马达，这样就可以知道驱动器的状态了。
	call	print_nl	! 光标回车换行，我感觉可以放在加载 system 模块前的 int 0x10 指令后。

! After that we check which root-device to use. If the device is
! defined (!= 0), nothing is done and the given device is used.
! Otherwise, either /dev/PS0 (2,28) or /dev/at0 (2,8), depending
! on the number of sectors that the BIOS reports currently.
! 此后，我们检查要使用哪个根文件系统设备（简称根设备）。如果已经指定了设备(!= 0)就直
! 接使用给定的设备。否则就需要根据 BIOS 报告的每磁道扇区数来确定到底使用/dev/PS0 (2,28) 
! 还是 /dev/at0 (2,8)。
! 上面一行中两个设备文件的含义：
! 在 Linux 中软驱的主设备号是2(参见第 78 行的注释)，次设备号 = type * 4 + nr，其中
! nr 为 0 - 3 分别对应软驱A、B、C 或D；type 是软驱的类型（2->1.2M 或 7->1.44M 等）。
! 因为 7*4 + 0 = 28，所以 /dev/PS0 (2,28)指的是1.44M A 驱动器,其设备号是 0x021c
! 同理 /dev/at0 (2,8)指的是1.2M A 驱动器，其设备号是0x0208。

	seg cs
	mov	ax,root_dev 	! 取出 root_dev 的值，判断根设备号是否被定义
	or	ax,ax
	jne	root_defined
	seg cs 				! 取出 sectors 的值(每磁道扇区数);sectors = 15 则说明是 1.2MB 的驱动器;
	mov	bx,sectors 		! sectors = 18 则说明是 1.44MB 的软驱。因为是可引导的驱动器，所以是A驱。
	mov	ax,#0x0208		! /dev/PS0 - 1.2Mb
	cmp	bx,#15
	je	root_defined
	mov	ax,#0x021c		! /dev/PS0 - 1.44Mb
	cmp	bx,#18
	je	root_defined
undef_root:				!都不等于的情况下则进入死循环
	jmp undef_root
root_defined:			!将检查过的设备号保存到 root_dev 中。
	seg cs
	mov	root_dev,ax

! after that (everyting loaded), we jump to
! the setup-routine loaded directly after
! the bootblock:
! 到此，所有程序都加载完毕，我们就跳转到被加载在 bootsect 后面的 setup 程序去。

	jmpi	0,SETUPSEG

!!!!!!!!!!!!!!!!!!!!! bootsect.S 程序到此就结束了。

! This routine loads the system at address 0x10000, making sure
! no 64kB boundaries are crossed. We try to load it as fast as
! possible, loading whole tracks whenever we can.
!
! in:	es - starting address segment (normally 0x1000)
!
! 该子程序将系统模块加载到内存地址 0x10000 处，并确定没有跨越 64KB 的内存边界。我们试图尽快
! 地进行加载，只要可能，就每次加载整条磁道的数据。
! 输入：es – 开始内存地址段值（通常是 0x1000）
sread:	.word 1+SETUPLEN	! sectors read of current track  ! bootsect 和 setup 程序所占的扇区数。
head:	.word 0			! current head !当前磁头号
track:	.word 0			! current track !当前磁道号

read_it:
! 首先测试输入的段值。必须位于内存地址64KB 边界处，否则进入死循环。清bx 寄存器，用于表示当前段内
! 存放数据的开始位置。
	mov ax,es
	test ax,#0x0fff
die:	jne die			! es must be at 64kB boundary ! es 值必须位于64KB 地址边界。
	xor bx,bx		! bx is starting address within segment ! bx 为段内偏移位置。
rp_read:
	mov ax,es
	cmp ax,#ENDSEG		! have we loaded all yet? ! 是否已经加载了全部数据？
	jb ok1_read
	ret
ok1_read:
! 计算和验证当前磁道需要读取的扇区数，放在 ax 寄存器中。
! 根据当前磁道还未读取的扇区数以及段内数据字节开始偏移位置，计算如果全部读取这些未读扇区，所
! 读总字节数是否会超过64KB 段长度的限制。若会超过，则根据此次最多能读入的字节数(64KB – 段内
! 偏移位置)，反算出此次需要读取的扇区数。
	seg cs
	mov ax,sectors
	sub ax,sread	! bootsect 和 setup 程序所占的扇区数
	mov cx,ax 		! cx = ax = 当前磁道未读扇区数。
	shl cx,#9 		! cx = cx * 512 字节。
	add cx,bx 		! cx + bx = 此次读操作后，段内共读入的字节数(偏移地址)。
	jnc ok2_read	! 若没有超过 64KB 字节，则跳转至 ok2_read 处执行。
	je ok2_read
	! 若加上此次将读磁道上所有未读扇区时会超过64KB，则计算此时最多能读入的
	! 字节数(64KB – 段内读偏移位置)，再转换成需要读取的扇区数。
	xor ax,ax
	sub ax,bx 		! 0 - bx 就是取补数，就等于这次还能读入的字节数
	shr ax,#9 		! 右移9位等同于除以512，转换成扇区数
ok2_read:
	call read_track ! 读当前磁道上指定扇区和需读扇区数的数据
	mov cx,ax
	add ax,sread
	seg cs
	cmp ax,sectors 	! 若当前磁道还有扇区未读完，则跳转到ok3_read。
	jne ok3_read
	! 若该磁道的当前磁头面所有扇区都被读完，则读该磁道的下一磁头面(1号磁头)上的数据。
	! 如果完成，则去读下一磁道
	mov ax,#1
	sub ax,head
	jne ok4_read	! 如果是0磁头，则去读下一磁头面的扇区数据
	inc track 		! 否则读下一磁道。
ok4_read:
	mov head,ax
	xor ax,ax
	! 当前磁道上还有未读扇区，先保存当前磁道已读扇区数，并调整存放数据处的开始位置。
ok3_read:
	mov sread,ax 		! 保存当前磁道已读扇区数。
	shl cx,#9 			! 上次已读扇区数*512 字节。
	add bx,cx 			! 调整当前段内数据开始位置。
	jnc rp_read 		! 若小于 64KB 边界值，则跳转到rp_read处，继续读数据。
						! 否则调整当前段，为读下一段数据作准备。
	mov ax,es 			! 调整段寄存器值
	add ah,#0x10
	mov es,ax
	xor bx,bx 			! 清除段内数据开始偏移量
	jmp rp_read

! 读当前磁道上指定开始扇区和需读扇区数的数据到es:bx 开始处。参见第67 行下对BIOS 磁盘读中断
! int 0x13，ah=2 的说明。
! al – 需读扇区数；es:bx – 缓冲区开始位置。
read_track:
	pusha			! 压人所有寄存器(push all)
	! 首先调用 BIOS 中断，功能 ah = 0x0e (以电传方式传字符)，光标前移一位置。
	pusha			
	mov	ax, #0xe2e 	! loading... message 2e = .
	mov	bx, #7
 	int	0x10
	popa		

! 开始进行磁道扇区读操作。
	mov dx,track 	! 当前磁道号
	mov cx,sread 	! 当前磁道上已读扇区
	inc cx
	mov ch,dl 		! ch - 磁道号 ，cl - 开始读的扇区
	mov dx,head 	! 取当前磁头号
	mov dh,dl 		! dh = 磁头号，dl = 驱动器号( 0 代表 A驱)。
	and dx,#0x0100 	! 磁头号不大于1.
	mov ah,#2
	
	push	dx				! save for error dump
	push	cx				! 保留出错情况
	push	bx
	push	ax

	int 0x13
	jc bad_rt
	add	sp, #8				! 若没有出错，丢弃出错情况保存的信息。   	
	popa
	ret

! 读磁盘操作出错。则先显示出错信息，然后执行驱动复位操作(磁盘中断功能号0)，再跳转到read_track处重试。
bad_rt:	push	ax				! save error code
	call	print_all			! ah = error, al = read
	
	
	xor ah,ah
	xor dl,dl
	int 0x13
	

	add	sp, #10					! 丢弃为出错情况保存的信息 ax + ax,bx,cx,dx
	popa	
	jmp read_track

/*
 *	print_all is for debugging purposes.  
 *	It will print out all of the registers.  The assumption is that this is
 *	called from a routine, with a stack frame like
 *	dx 
 *	cx
 *	bx
 *	ax
 *	error
 *	ret <- sp
 *
*/
 ! print_all 用于调试目的，前提是从一个子程序中调用。并栈帧结构如上所示
print_all:
	mov	cx, #5		! error code + 4 registers
	mov	bp, sp	

print_loop:
	push	cx		! save count left
	call	print_nl	! nl for readability
	jae	no_reg		! see if register name is needed
	
	mov	ax, #0xe05 + 0x41 - 1
	sub	al, cl
	int	0x10

	mov	al, #0x58 	! X
	int	0x10

	mov	al, #0x3a 	! :
	int	0x10

no_reg:
	add	bp, #2		! next register
	call	print_hex	! print it
	pop	cx
	loop	print_loop
	ret

! 调用 BIOS 中断 0x10，以电传方式显示回车换行
print_nl:
	mov	ax, #0xe0d	! CR
	int	0x10
	mov	al, #0xa	! LF
	int 	0x10
	ret

/*
 *	print_hex is for debugging purposes, and prints the word
 *	pointed to by ss:bp in hexadecmial.
*/

print_hex:
	mov	cx, #4		! 4 hex digits
	mov	dx, (bp)	! load word into dx
print_digit:
	rol	dx, #4		! rotate so that lowest 4 bits are used 左旋4位
	mov	ah, #0xe	
	mov	al, dl		! mask off so we have only next nibble
	and	al, #0xf 	! 只取低四位显示
	add	al, #0x30	! convert to 0 based digit, '0'
	cmp	al, #0x39	! check for overflow ! 大于9的处理，转换成A-F
	jbe	good_digit
	add	al, #0x41 - 0x30 - 0xa 	! 'A' - '0' - 0xa 

good_digit:
	int	0x10
	loop	print_digit			! cx--。如cx>0 则显示下一个值
	ret


/*
 * This procedure turns off the floppy drive motor, so
 * that we enter the kernel in a known state, and
 * don't have to worry about it later.					'
 */
 ! 这个子程序用于关闭软驱的马达，这样我们进入内核后它处于已知状态，以后也就无须担心它了。
kill_motor:
	push dx
	mov dx,#0x3f2		! 软驱控制卡的数字输出寄存器端口，只读。
	xor al, al
	outb				! 将al中的值输出到dx指定的端口去。
	pop dx
	ret

sectors:
	.word 0

msg1:
	.byte 13,10
	.ascii "Loading"

.org 506
! 表示下面语句从地址 506 (0x1FC)开始，所以 swap_dev 在启动扇区的第 506 开
! 始的 2 个字节中，root_dev 在启动扇区的第 508 开始的 2 个字节中。
swap_dev:
	.word SWAP_DEV
root_dev:
	.word ROOT_DEV
! 下面是启动盘具有有效引导扇区的标志。
boot_flag:
	.word 0xAA55

.text
endtext:
.data
enddata:
.bss
endbss:

