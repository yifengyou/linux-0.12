/*
 * linux/kernel/math/math_emulate.c
 *
 * (C) 1991 Linus Torvalds
 */

/*
 * Limited emulation 27.12.91 - mostly loads/stores, which gcc wants
 * even for soft-float, unless you use bruce evans' patches. The patches
 * are great, but they have to be re-applied for every version, and the
 * library is different for soft-float and 80387. So emulation is more
 * practical, even though it's slower.
 *
 * 28.12.91 - loads/stores work, even BCD. I'll have to start thinking
 * about add/sub/mul/div. Urgel. I should find some good source, but I'll
 * just fake up something.
 *
 * 30.12.91 - add/sub/mul/div/com seem to work mostly. I should really
 * test every possible combination.
 */

/*
 * This file is full of ugly macros etc: one problem was that gcc simply
 * didn't want to make the structures as they should be: it has to try to
 * align them. Sickening code, but at least I've hidden the ugly things
 * in this one file: the other files don't need to know about these things.
 *
 * The other files also don't care about ST(x) etc - they just get addresses
 * to 80-bit temporary reals, and do with them as they please. I wanted to
 * hide most of the 387-specific things here.
 */

#include <signal.h>

#define __ALIGNED_TEMP_REAL 1
#include <linux/math_emu.h>
#include <linux/kernel.h>
#include <asm/segment.h>

#define bswapw(x) __asm__("xchgb %%al,%%ah":"=a" (x):"0" ((short)x))
#define ST(x) (*__st((x)))
#define PST(x) ((const temp_real *) __st((x)))

/*
 * We don't want these inlined - it gets too messy in the machine-code.
 */
static void fpop(void);
static void fpush(void);
static void fxchg(temp_real_unaligned * a, temp_real_unaligned * b);
static temp_real_unaligned * __st(int i);

static void do_emu(struct info * info)
{
	unsigned short code;
	temp_real tmp;
	char * address;

	if (I387.cwd & I387.swd & 0x3f)
		I387.swd |= 0x8000;
	else
		I387.swd &= 0x7fff;
	ORIG_EIP = EIP;
/* 0x0007 means user code space */
	if (CS != 0x000F) {
		printk("math_emulate: %04x:%08x\n\r",CS,EIP);
		panic("Math emulation needed in kernel");
	}
	code = get_fs_word((unsigned short *) EIP);
	bswapw(code);
	code &= 0x7ff;
	I387.fip = EIP;
	*(unsigned short *) &I387.fcs = CS;
	*(1+(unsigned short *) &I387.fcs) = code;
	EIP += 2;
	switch (code) {
		case 0x1d0: /* fnop */
			return;
		case 0x1d1: case 0x1d2: case 0x1d3:
		case 0x1d4: case 0x1d5: case 0x1d6: case 0x1d7:
			math_abort(info,1<<(SIGILL-1));
		case 0x1e0:
			ST(0).exponent ^= 0x8000;
			return;
		case 0x1e1:
			ST(0).exponent &= 0x7fff;
			return;
		case 0x1e2: case 0x1e3:
			math_abort(info,1<<(SIGILL-1));
		case 0x1e4:
			ftst(PST(0));
			return;
		case 0x1e5:
			printk("fxam not implemented\n\r");
			math_abort(info,1<<(SIGILL-1));
		case 0x1e6: case 0x1e7:
			math_abort(info,1<<(SIGILL-1));
		case 0x1e8:
			fpush();
			ST(0) = CONST1;
			return;
		case 0x1e9:
			fpush();
			ST(0) = CONSTL2T;
			return;
		case 0x1ea:
			fpush();
			ST(0) = CONSTL2E;
			return;
		case 0x1eb:
			fpush();
			ST(0) = CONSTPI;
			return;
		case 0x1ec:
			fpush();
			ST(0) = CONSTLG2;
			return;
		case 0x1ed:
			fpush();
			ST(0) = CONSTLN2;
			return;
		case 0x1ee:
			fpush();
			ST(0) = CONSTZ;
			return;
		case 0x1ef:
			math_abort(info,1<<(SIGILL-1));
		case 0x1f0: case 0x1f1: case 0x1f2: case 0x1f3:
		case 0x1f4: case 0x1f5: case 0x1f6: case 0x1f7:
		case 0x1f8: case 0x1f9: case 0x1fa: case 0x1fb:
		case 0x1fc: case 0x1fd: case 0x1fe: case 0x1ff:
			printk("%04x fxxx not implemented\n\r",code + 0xc800);
			math_abort(info,1<<(SIGILL-1));
		case 0x2e9:
			fucom(PST(1),PST(0));
			fpop(); fpop();
			return;
		case 0x3d0: case 0x3d1:
			return;
		case 0x3e2:
			I387.swd &= 0x7f00;
			return;
		case 0x3e3:
			I387.cwd = 0x037f;
			I387.swd = 0x0000;
			I387.twd = 0x0000;
			return;
		case 0x3e4:
			return;
		case 0x6d9:
			fcom(PST(1),PST(0));
			fpop(); fpop();
			return;
		case 0x7e0:
			*(short *) &EAX = I387.swd;
			return;
	}
	switch (code >> 3) {
		case 0x18:
			fadd(PST(0),PST(code & 7),&tmp);
			real_to_real(&tmp,&ST(0));
			return;
		case 0x19:
			fmul(PST(0),PST(code & 7),&tmp);
			real_to_real(&tmp,&ST(0));
			return;
		case 0x1a:
			fcom(PST(code & 7),&tmp);
			real_to_real(&tmp,&ST(0));
			return;
		case 0x1b:
			fcom(PST(code & 7),&tmp);
			real_to_real(&tmp,&ST(0));
			fpop();
			return;
		case 0x1c:
			real_to_real(&ST(code & 7),&tmp);
			tmp.exponent ^= 0x8000;
			fadd(PST(0),&tmp,&tmp);
			real_to_real(&tmp,&ST(0));
			return;
		case 0x1d:
			ST(0).exponent ^= 0x8000;
			fadd(PST(0),PST(code & 7),&tmp);
			real_to_real(&tmp,&ST(0));
			return;
		case 0x1e:
			fdiv(PST(0),PST(code & 7),&tmp);
			real_to_real(&tmp,&ST(0));
			return;
		case 0x1f:
			fdiv(PST(code & 7),PST(0),&tmp);
			real_to_real(&tmp,&ST(0));
			return;
		case 0x38:
			fpush();
			ST(0) = ST((code & 7)+1);
			return;
		case 0x39:
			fxchg(&ST(0),&ST(code & 7));
			return;
		case 0x3b:
			ST(code & 7) = ST(0);
			fpop();
			return;
		case 0x98:
			fadd(PST(0),PST(code & 7),&tmp);
			real_to_real(&tmp,&ST(code & 7));
			return;
		case 0x99:
			fmul(PST(0),PST(code & 7),&tmp);
			real_to_real(&tmp,&ST(code & 7));
			return;
		case 0x9a:
			fcom(PST(code & 7),PST(0));
			return;
		case 0x9b:
			fcom(PST(code & 7),PST(0));
			fpop();
			return;			
		case 0x9c:
			ST(code & 7).exponent ^= 0x8000;
			fadd(PST(0),PST(code & 7),&tmp);
			real_to_real(&tmp,&ST(code & 7));
			return;
		case 0x9d:
			real_to_real(&ST(0),&tmp);
			tmp.exponent ^= 0x8000;
			fadd(PST(code & 7),&tmp,&tmp);
			real_to_real(&tmp,&ST(code & 7));
			return;
		case 0x9e:
			fdiv(PST(0),PST(code & 7),&tmp);
			real_to_real(&tmp,&ST(code & 7));
			return;
		case 0x9f:
			fdiv(PST(code & 7),PST(0),&tmp);
			real_to_real(&tmp,&ST(code & 7));
			return;
		case 0xb8:
			printk("ffree not implemented\n\r");
			math_abort(info,1<<(SIGILL-1));
		case 0xb9:
			fxchg(&ST(0),&ST(code & 7));
			return;
		case 0xba:
			ST(code & 7) = ST(0);
			return;
		case 0xbb:
			ST(code & 7) = ST(0);
			fpop();
			return;
		case 0xbc:
			fucom(PST(code & 7),PST(0));
			return;
		case 0xbd:
			fucom(PST(code & 7),PST(0));
			fpop();
			return;
		case 0xd8:
			fadd(PST(code & 7),PST(0),&tmp);
			real_to_real(&tmp,&ST(code & 7));
			fpop();
			return;
		case 0xd9:
			fmul(PST(code & 7),PST(0),&tmp);
			real_to_real(&tmp,&ST(code & 7));
			fpop();
			return;
		case 0xda:
			fcom(PST(code & 7),PST(0));
			fpop();
			return;
		case 0xdc:
			ST(code & 7).exponent ^= 0x8000;
			fadd(PST(0),PST(code & 7),&tmp);
			real_to_real(&tmp,&ST(code & 7));
			fpop();
			return;
		case 0xdd:
			real_to_real(&ST(0),&tmp);
			tmp.exponent ^= 0x8000;
			fadd(PST(code & 7),&tmp,&tmp);
			real_to_real(&tmp,&ST(code & 7));
			fpop();
			return;
		case 0xde:
			fdiv(PST(0),PST(code & 7),&tmp);
			real_to_real(&tmp,&ST(code & 7));
			fpop();
			return;
		case 0xdf:
			fdiv(PST(code & 7),PST(0),&tmp);
			real_to_real(&tmp,&ST(code & 7));
			fpop();
			return;
		case 0xf8:
			printk("ffree not implemented\n\r");
			math_abort(info,1<<(SIGILL-1));
			fpop();
			return;
		case 0xf9:
			fxchg(&ST(0),&ST(code & 7));
			return;
		case 0xfa:
		case 0xfb:
			ST(code & 7) = ST(0);
			fpop();
			return;
	}
	switch ((code>>3) & 0xe7) {
		case 0x22:
			put_short_real(PST(0),info,code);
			return;
		case 0x23:
			put_short_real(PST(0),info,code);
			fpop();
			return;
		case 0x24:
			address = ea(info,code);
			for (code = 0 ; code < 7 ; code++) {
				((long *) & I387)[code] =
				   get_fs_long((unsigned long *) address);
				address += 4;
			}
			return;
		case 0x25:
			address = ea(info,code);
			*(unsigned short *) &I387.cwd =
				get_fs_word((unsigned short *) address);
			return;
		case 0x26:
			address = ea(info,code);
			verify_area(address,28);
			for (code = 0 ; code < 7 ; code++) {
				put_fs_long( ((long *) & I387)[code],
					(unsigned long *) address);
				address += 4;
			}
			return;
		case 0x27:
			address = ea(info,code);
			verify_area(address,2);
			put_fs_word(I387.cwd,(short *) address);
			return;
		case 0x62:
			put_long_int(PST(0),info,code);
			return;
		case 0x63:
			put_long_int(PST(0),info,code);
			fpop();
			return;
		case 0x65:
			fpush();
			get_temp_real(&tmp,info,code);
			real_to_real(&tmp,&ST(0));
			return;
		case 0x67:
			put_temp_real(PST(0),info,code);
			fpop();
			return;
		case 0xa2:
			put_long_real(PST(0),info,code);
			return;
		case 0xa3:
			put_long_real(PST(0),info,code);
			fpop();
			return;
		case 0xa4:
			address = ea(info,code);
			for (code = 0 ; code < 27 ; code++) {
				((long *) & I387)[code] =
				   get_fs_long((unsigned long *) address);
				address += 4;
			}
			return;
		case 0xa6:
			address = ea(info,code);
			verify_area(address,108);
			for (code = 0 ; code < 27 ; code++) {
				put_fs_long( ((long *) & I387)[code],
					(unsigned long *) address);
				address += 4;
			}
			I387.cwd = 0x037f;
			I387.swd = 0x0000;
			I387.twd = 0x0000;
			return;
		case 0xa7:
			address = ea(info,code);
			verify_area(address,2);
			put_fs_word(I387.swd,(short *) address);
			return;
		case 0xe2:
			put_short_int(PST(0),info,code);
			return;
		case 0xe3:
			put_short_int(PST(0),info,code);
			fpop();
			return;
		case 0xe4:
			fpush();
			get_BCD(&tmp,info,code);
			real_to_real(&tmp,&ST(0));
			return;
		case 0xe5:
			fpush();
			get_longlong_int(&tmp,info,code);
			real_to_real(&tmp,&ST(0));
			return;
		case 0xe6:
			put_BCD(PST(0),info,code);
			fpop();
			return;
		case 0xe7:
			put_longlong_int(PST(0),info,code);
			fpop();
			return;
	}
	switch (code >> 9) {
		case 0:
			get_short_real(&tmp,info,code);
			break;
		case 1:
			get_long_int(&tmp,info,code);
			break;
		case 2:
			get_long_real(&tmp,info,code);
			break;
		case 4:
			get_short_int(&tmp,info,code);
	}
	switch ((code>>3) & 0x27) {
		case 0:
			fadd(&tmp,PST(0),&tmp);
			real_to_real(&tmp,&ST(0));
			return;
		case 1:
			fmul(&tmp,PST(0),&tmp);
			real_to_real(&tmp,&ST(0));
			return;
		case 2:
			fcom(&tmp,PST(0));
			return;
		case 3:
			fcom(&tmp,PST(0));
			fpop();
			return;
		case 4:
			tmp.exponent ^= 0x8000;
			fadd(&tmp,PST(0),&tmp);
			real_to_real(&tmp,&ST(0));
			return;
		case 5:
			ST(0).exponent ^= 0x8000;
			fadd(&tmp,PST(0),&tmp);
			real_to_real(&tmp,&ST(0));
			return;
		case 6:
			fdiv(PST(0),&tmp,&tmp);
			real_to_real(&tmp,&ST(0));
			return;
		case 7:
			fdiv(&tmp,PST(0),&tmp);
			real_to_real(&tmp,&ST(0));
			return;
	}
	if ((code & 0x138) == 0x100) {
			fpush();
			real_to_real(&tmp,&ST(0));
			return;
	}
	printk("Unknown math-insns: %04x:%08x %04x\n\r",CS,EIP,code);
	math_abort(info,1<<(SIGFPE-1));
}

void math_emulate(long ___false)
{
	if (!current->used_math) {
		current->used_math = 1;
		I387.cwd = 0x037f;
		I387.swd = 0x0000;
		I387.twd = 0x0000;
	}
/* &___false points to info->___orig_eip, so subtract 1 to get info */
	do_emu((struct info *) ((&___false) - 1));
}

void __math_abort(struct info * info, unsigned int signal)
{
	EIP = ORIG_EIP;
	current->signal |= signal;
	__asm__("movl %0,%%esp ; ret"::"g" ((long) info));
}

static void fpop(void)
{
	unsigned long tmp;

	tmp = I387.swd & 0xffffc7ff;
	I387.swd += 0x00000800;
	I387.swd &= 0x00003800;
	I387.swd |= tmp;
}

static void fpush(void)
{
	unsigned long tmp;

	tmp = I387.swd & 0xffffc7ff;
	I387.swd += 0x00003800;
	I387.swd &= 0x00003800;
	I387.swd |= tmp;
}

static void fxchg(temp_real_unaligned * a, temp_real_unaligned * b)
{
	temp_real_unaligned c;

	c = *a;
	*a = *b;
	*b = c;
}

static temp_real_unaligned * __st(int i)
{
	i += I387.swd >> 11;
	i &= 7;
	return (temp_real_unaligned *) (i*10 + (char *)(I387.st_space));
}
