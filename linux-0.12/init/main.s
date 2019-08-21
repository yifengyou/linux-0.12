	.file	"init/main.c"
gcc_compiled.:
.text
LC0:
	.ascii "out of memory\12\15\0"
	.align 2
_sprintf:
	movl 4(%esp),%edx
	leal 12(%esp),%eax
	pushl %eax
	pushl 12(%esp)
	pushl %edx
	call _vsprintf
	addl $12,%esp
	ret
	.align 2
_time_init:
	pushl %ebp
	movl %esp,%ebp
	subl $44,%esp
L65:
	movl $128,%eax
	movl $112,%edx
/APP
	outb %al,%dx
	jmp 1f
1:	jmp 1f
1:
/NO_APP
	movl $113,%edx
/APP
	inb %dx,%al
	jmp 1f
1:	jmp 1f
1:
/NO_APP
	movb %al,-40(%ebp)
	movzbl -40(%ebp),%eax
	movl %eax,-36(%ebp)
	movl $130,%eax
	movl $112,%edx
/APP
	outb %al,%dx
	jmp 1f
1:	jmp 1f
1:
/NO_APP
	movl $113,%edx
/APP
	inb %dx,%al
	jmp 1f
1:	jmp 1f
1:
/NO_APP
	movb %al,-40(%ebp)
	movzbl -40(%ebp),%eax
	movl %eax,-32(%ebp)
	movl $132,%eax
	movl $112,%edx
/APP
	outb %al,%dx
	jmp 1f
1:	jmp 1f
1:
/NO_APP
	movl $113,%edx
/APP
	inb %dx,%al
	jmp 1f
1:	jmp 1f
1:
/NO_APP
	movb %al,-40(%ebp)
	movzbl -40(%ebp),%eax
	movl %eax,-28(%ebp)
	movl $135,%eax
	movl $112,%edx
/APP
	outb %al,%dx
	jmp 1f
1:	jmp 1f
1:
/NO_APP
	movl $113,%edx
/APP
	inb %dx,%al
	jmp 1f
1:	jmp 1f
1:
/NO_APP
	movb %al,-40(%ebp)
	movzbl -40(%ebp),%eax
	movl %eax,-24(%ebp)
	movl $136,%eax
	movl $112,%edx
/APP
	outb %al,%dx
	jmp 1f
1:	jmp 1f
1:
/NO_APP
	movl $113,%edx
/APP
	inb %dx,%al
	jmp 1f
1:	jmp 1f
1:
/NO_APP
	movb %al,-40(%ebp)
	movzbl -40(%ebp),%eax
	movl %eax,-20(%ebp)
	movl $137,%eax
	movl $112,%edx
/APP
	outb %al,%dx
	jmp 1f
1:	jmp 1f
1:
/NO_APP
	movl $113,%edx
/APP
	inb %dx,%al
	jmp 1f
1:	jmp 1f
1:
/NO_APP
	movb %al,-40(%ebp)
	movzbl -40(%ebp),%eax
	movl %eax,-16(%ebp)
	movl $128,%eax
	movl $112,%edx
/APP
	outb %al,%dx
	jmp 1f
1:	jmp 1f
1:
/NO_APP
	movl $113,%edx
/APP
	inb %dx,%al
	jmp 1f
1:	jmp 1f
1:
/NO_APP
	movb %al,-40(%ebp)
	movzbl -40(%ebp),%eax
	cmpl -36(%ebp),%eax
	jne L65
	movl -36(%ebp),%eax
	andl $15,%eax
	movl -36(%ebp),%edx
	sarl $4,%edx
	leal (%edx,%edx,4),%edx
	leal (%eax,%edx,2),%eax
	movl %eax,-36(%ebp)
	movl -32(%ebp),%eax
	andl $15,%eax
	movl -32(%ebp),%edx
	sarl $4,%edx
	leal (%edx,%edx,4),%edx
	leal (%eax,%edx,2),%eax
	movl %eax,-32(%ebp)
	movl -28(%ebp),%eax
	andl $15,%eax
	movl -28(%ebp),%edx
	sarl $4,%edx
	leal (%edx,%edx,4),%edx
	leal (%eax,%edx,2),%eax
	movl %eax,-28(%ebp)
	movl -24(%ebp),%eax
	andl $15,%eax
	movl -24(%ebp),%edx
	sarl $4,%edx
	leal (%edx,%edx,4),%edx
	leal (%eax,%edx,2),%eax
	movl %eax,-24(%ebp)
	movl -20(%ebp),%eax
	andl $15,%eax
	movl -20(%ebp),%edx
	sarl $4,%edx
	leal (%edx,%edx,4),%edx
	leal (%eax,%edx,2),%eax
	movl %eax,-20(%ebp)
	movl -16(%ebp),%eax
	andl $15,%eax
	movl -16(%ebp),%edx
	sarl $4,%edx
	leal (%edx,%edx,4),%edx
	leal (%eax,%edx,2),%eax
	movl %eax,-16(%ebp)
	decl -20(%ebp)
	leal -36(%ebp),%eax
	pushl %eax
	call _kernel_mktime
	movl %eax,_startup_time
	leave
	ret
.data
	.align 2
_memory_end:
	.long 0
	.align 2
_buffer_memory_end:
	.long 0
	.align 2
_main_memory_start:
	.long 0
.text
LC1:
	.ascii "/bin/sh\0"
.data
	.align 2
_argv_rc:
	.long LC1
	.long 0
.text
LC2:
	.ascii "HOME=/\0"
.data
	.align 2
_envp_rc:
	.long LC2
	.long 0
	.long 0
.text
LC3:
	.ascii "-/bin/sh\0"
.data
	.align 2
_argv:
	.long LC3
	.long 0
.text
LC4:
	.ascii "HOME=/usr/root\0"
.data
	.align 2
_envp:
	.long LC4
	.long 0
	.long 0
.text
LC5:
	.ascii "TERM=con%dx%d\0"
	.align 2
.globl _main
_main:
	pushl %ebp
	movl %esp,%ebp
	subl $8,%esp
	pushl %edi
	pushl %esi
	movzwl 590332,%eax
	movl %eax,_ROOT_DEV
	movzwl 590330,%eax
	movl %eax,_SWAP_DEV
	movw 589838,%dx
	andl $255,%edx
	pushl %edx
	movw 589838,%ax
	andw $65280,%ax
	shrw $8,%ax
	movw %ax,-4(%ebp)
	movzwl -4(%ebp),%eax
	pushl %eax
	pushl $LC5
	pushl $_term
	call _sprintf
	movl $_term,_envp+4
	movl $_term,_envp_rc+4
	movl $_drive_info,%edi
	movl $589952,%esi
	movl $8,%ecx
	cld
	rep
	movsl
	movzwl 589826,%eax
	sall $10,%eax
	addl $1048576,%eax
	movl %eax,_memory_end
	andl $-4096,_memory_end
	addl $16,%esp
	cmpl $16777216,_memory_end
	jle L69
	movl $16777216,_memory_end
L69:
	cmpl $12582912,_memory_end
	jle L70
	movl $4194304,_buffer_memory_end
	jmp L71
	.align 2
L70:
	cmpl $6291456,_memory_end
	jle L72
	movl $2097152,_buffer_memory_end
	jmp L71
	.align 2
L72:
	movl $1048576,_buffer_memory_end
L71:
	movl _buffer_memory_end,%eax
	movl %eax,_main_memory_start
	pushl _memory_end
	pushl _buffer_memory_end
	call _mem_init
	call _trap_init
	call _blk_dev_init
	call _chr_dev_init
	call _tty_init
	call _time_init
	call _sched_init
	pushl _buffer_memory_end
	call _buffer_init
	call _hd_init
	call _floppy_init
/APP
	sti
	movl %esp,%eax
	pushl $0x17
	pushl %eax
	pushfl
	pushl $0x0f
	pushl $1f
	iret
1:	movl $0x17,%eax
	movw %ax,%ds
	movw %ax,%es
	movw %ax,%fs
	movw %ax,%gs
/NO_APP
	addl $12,%esp
	movl $2,%eax
/APP
	int $0x80
/NO_APP
	movl %eax,%edx
	testl %edx,%edx
	jge L75
	negl %edx
	movl %edx,_errno
	movl $-1,%edx
L75:
	testl %edx,%edx
	jne L74
	call _init
L74:
L77:
	movl $29,%eax
/APP
	int $0x80
/NO_APP
	jmp L77
	.align 2
	leal -16(%ebp),%esp
	popl %esi
	popl %edi
	leave
	ret
	.align 2
_printf:
	pushl %ebx
	leal 12(%esp),%eax
	pushl %eax
	pushl 12(%esp)
	pushl $_printbuf
	call _vsprintf
	movl %eax,%ebx
	pushl %ebx
	pushl $_printbuf
	pushl $1
	call _write
	movl %ebx,%eax
	addl $24,%esp
	popl %ebx
	ret
LC6:
	.ascii "/dev/tty1\0"
LC7:
	.ascii "%d buffers = %d bytes buffer space\12\15\0"
LC8:
	.ascii "Free mem: %d bytes\12\15\0"
LC9:
	.ascii "/etc/rc\0"
LC10:
	.ascii "Fork failed in init\15\12\0"
LC11:
	.ascii "\12\15child %d died with code %04x\12\15\0"
	.align 2
.globl _init
_init:
	pushl %ebp
	movl %esp,%ebp
	subl $4,%esp
	pushl %edi
	pushl %esi
	pushl %ebx
	xorl %eax,%eax
	movl $_drive_info,%ebx
/APP
	int $0x80
/NO_APP
	testl %eax,%eax
	jge L82
	negl %eax
	movl %eax,_errno
L82:
	pushl $0
	pushl $2
	pushl $LC6
	call _open
	pushl $0
	call _dup
	pushl $0
	call _dup
	movl _nr_buffers,%eax
	sall $10,%eax
	pushl %eax
	pushl _nr_buffers
	pushl $LC7
	call _printf
	addl $32,%esp
	movl _memory_end,%eax
	subl _main_memory_start,%eax
	pushl %eax
	pushl $LC8
	call _printf
	addl $8,%esp
	movl $2,%eax
/APP
	int $0x80
/NO_APP
	testl %eax,%eax
	jl L86
	movl %eax,%edi
	jmp L85
	.align 2
L86:
	negl %eax
	movl %eax,_errno
	movl $-1,%edi
L85:
	testl %edi,%edi
	jne L84
	pushl $0
	call _close
	pushl $0
	pushl $0
	pushl $LC9
	call _open
	addl $16,%esp
	testl %eax,%eax
	je L87
	pushl $1
	call __exit
	.align 2
L87:
	pushl $_envp_rc
	pushl $_argv_rc
	pushl $LC1
	call _execve
	pushl $2
	call __exit
	.align 2
L84:
	testl %edi,%edi
	jle L88
	leal -4(%ebp),%esi
L89:
	pushl %esi
	call _wait
	addl $4,%esp
	cmpl %edi,%eax
	jne L89
L88:
	leal -4(%ebp),%esi
L91:
	movl $2,%eax
/APP
	int $0x80
/NO_APP
	testl %eax,%eax
	jge L94
	negl %eax
	movl %eax,_errno
	movl $-1,%eax
L94:
	movl %eax,%edi
	testl %edi,%edi
	jge L93
	pushl $LC10
	call _printf
	addl $4,%esp
	jmp L91
	.align 2
L93:
	testl %edi,%edi
	jne L96
	pushl $0
	call _close
	pushl $1
	call _close
	pushl $2
	call _close
	call _setsid
	pushl $0
	pushl $2
	pushl $LC6
	call _open
	pushl $0
	call _dup
	pushl $0
	call _dup
	addl $32,%esp
	pushl $_envp
	pushl $_argv
	pushl $LC1
	call _execve
	pushl %eax
	call __exit
	.align 2
L96:
L97:
	pushl %esi
	call _wait
	addl $4,%esp
	cmpl %edi,%eax
	jne L97
	pushl -4(%ebp)
	pushl %edi
	pushl $LC11
	call _printf
	addl $12,%esp
	movl $36,%eax
/APP
	int $0x80
/NO_APP
	testl %eax,%eax
	jge L91
	negl %eax
	movl %eax,_errno
	jmp L91
	.align 2
	leal -16(%ebp),%esp
	popl %ebx
	popl %esi
	popl %edi
	leave
	ret
.comm _drive_info,32
.lcomm _term,32
.lcomm _printbuf,1024
