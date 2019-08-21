/*
 * linux/kernel/math/get_put.c
 *
 * (C) 1991 Linus Torvalds
 */

/*
 * This file handles all accesses to user memory: getting and putting
 * ints/reals/BCD etc. This is the only part that concerns itself with
 * other than temporary real format. All other cals are strictly temp_real.
 */
#include <signal.h>

#include <linux/math_emu.h>
#include <linux/kernel.h>
#include <asm/segment.h>

void get_short_real(temp_real * tmp,
	struct info * info, unsigned short code)
{
	char * addr;
	short_real sr;

	addr = ea(info,code);
	sr = get_fs_long((unsigned long *) addr);
	short_to_temp(&sr,tmp);
}

void get_long_real(temp_real * tmp,
	struct info * info, unsigned short code)
{
	char * addr;
	long_real lr;

	addr = ea(info,code);
	lr.a = get_fs_long((unsigned long *) addr);
	lr.b = get_fs_long(1 + (unsigned long *) addr);
	long_to_temp(&lr,tmp);
}

void get_temp_real(temp_real * tmp,
	struct info * info, unsigned short code)
{
	char * addr;

	addr = ea(info,code);
	tmp->a = get_fs_long((unsigned long *) addr);
	tmp->b = get_fs_long(1 + (unsigned long *) addr);
	tmp->exponent = get_fs_word(4 + (unsigned short *) addr);
}

void get_short_int(temp_real * tmp,
	struct info * info, unsigned short code)
{
	char * addr;
	temp_int ti;

	addr = ea(info,code);
	ti.a = (signed short) get_fs_word((unsigned short *) addr);
	ti.b = 0;
	if (ti.sign = (ti.a < 0))
		ti.a = - ti.a;
	int_to_real(&ti,tmp);
}

void get_long_int(temp_real * tmp,
	struct info * info, unsigned short code)
{
	char * addr;
	temp_int ti;

	addr = ea(info,code);
	ti.a = get_fs_long((unsigned long *) addr);
	ti.b = 0;
	if (ti.sign = (ti.a < 0))
		ti.a = - ti.a;
	int_to_real(&ti,tmp);
}

void get_longlong_int(temp_real * tmp,
	struct info * info, unsigned short code)
{
	char * addr;
	temp_int ti;

	addr = ea(info,code);
	ti.a = get_fs_long((unsigned long *) addr);
	ti.b = get_fs_long(1 + (unsigned long *) addr);
	if (ti.sign = (ti.b < 0))
		__asm__("notl %0 ; notl %1\n\t"
			"addl $1,%0 ; adcl $0,%1"
			:"=r" (ti.a),"=r" (ti.b)
			:"0" (ti.a),"1" (ti.b));
	int_to_real(&ti,tmp);
}

#define MUL10(low,high) \
__asm__("addl %0,%0 ; adcl %1,%1\n\t" \
"movl %0,%%ecx ; movl %1,%%ebx\n\t" \
"addl %0,%0 ; adcl %1,%1\n\t" \
"addl %0,%0 ; adcl %1,%1\n\t" \
"addl %%ecx,%0 ; adcl %%ebx,%1" \
:"=a" (low),"=d" (high) \
:"0" (low),"1" (high):"cx","bx")

#define ADD64(val,low,high) \
__asm__("addl %4,%0 ; adcl $0,%1":"=r" (low),"=r" (high) \
:"0" (low),"1" (high),"r" ((unsigned long) (val)))

void get_BCD(temp_real * tmp, struct info * info, unsigned short code)
{
	int k;
	char * addr;
	temp_int i;
	unsigned char c;

	addr = ea(info,code);
	addr += 9;
	i.sign = 0x80 & get_fs_byte(addr--);
	i.a = i.b = 0;
	for (k = 0; k < 9; k++) {
		c = get_fs_byte(addr--);
		MUL10(i.a, i.b);
		ADD64((c>>4), i.a, i.b);
		MUL10(i.a, i.b);
		ADD64((c&0xf), i.a, i.b);
	}
	int_to_real(&i,tmp);
}

void put_short_real(const temp_real * tmp,
	struct info * info, unsigned short code)
{
	char * addr;
	short_real sr;

	addr = ea(info,code);
	verify_area(addr,4);
	temp_to_short(tmp,&sr);
	put_fs_long(sr,(unsigned long *) addr);
}

void put_long_real(const temp_real * tmp,
	struct info * info, unsigned short code)
{
	char * addr;
	long_real lr;

	addr = ea(info,code);
	verify_area(addr,8);
	temp_to_long(tmp,&lr);
	put_fs_long(lr.a, (unsigned long *) addr);
	put_fs_long(lr.b, 1 + (unsigned long *) addr);
}

void put_temp_real(const temp_real * tmp,
	struct info * info, unsigned short code)
{
	char * addr;

	addr = ea(info,code);
	verify_area(addr,10);
	put_fs_long(tmp->a, (unsigned long *) addr);
	put_fs_long(tmp->b, 1 + (unsigned long *) addr);
	put_fs_word(tmp->exponent, 4 + (short *) addr);
}

void put_short_int(const temp_real * tmp,
	struct info * info, unsigned short code)
{
	char * addr;
	temp_int ti;

	addr = ea(info,code);
	real_to_int(tmp,&ti);
	verify_area(addr,2);
	if (ti.sign)
		ti.a = -ti.a;
	put_fs_word(ti.a,(short *) addr);
}

void put_long_int(const temp_real * tmp,
	struct info * info, unsigned short code)
{
	char * addr;
	temp_int ti;

	addr = ea(info,code);
	real_to_int(tmp,&ti);
	verify_area(addr,4);
	if (ti.sign)
		ti.a = -ti.a;
	put_fs_long(ti.a,(unsigned long *) addr);
}

void put_longlong_int(const temp_real * tmp,
	struct info * info, unsigned short code)
{
	char * addr;
	temp_int ti;

	addr = ea(info,code);
	real_to_int(tmp,&ti);
	verify_area(addr,8);
	if (ti.sign)
		__asm__("notl %0 ; notl %1\n\t"
			"addl $1,%0 ; adcl $0,%1"
			:"=r" (ti.a),"=r" (ti.b)
			:"0" (ti.a),"1" (ti.b));
	put_fs_long(ti.a,(unsigned long *) addr);
	put_fs_long(ti.b,1 + (unsigned long *) addr);
}

#define DIV10(low,high,rem) \
__asm__("divl %6 ; xchgl %1,%2 ; divl %6" \
	:"=d" (rem),"=a" (low),"=b" (high) \
	:"0" (0),"1" (high),"2" (low),"c" (10))

void put_BCD(const temp_real * tmp,struct info * info, unsigned short code)
{
	int k,rem;
	char * addr;
	temp_int i;
	unsigned char c;

	addr = ea(info,code);
	verify_area(addr,10);
	real_to_int(tmp,&i);
	if (i.sign)
		put_fs_byte(0x80, addr+9);
	else
		put_fs_byte(0, addr+9);
	for (k = 0; k < 9; k++) {
		DIV10(i.a,i.b,rem);
		c = rem;
		DIV10(i.a,i.b,rem);
		c += rem<<4;
		put_fs_byte(c,addr++);
	}
}
