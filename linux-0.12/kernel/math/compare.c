/*
 * linux/kernel/math/compare.c
 *
 * (C) 1991 Linus Torvalds
 */

/*
 * temporary real comparison routines
 */

#include <linux/math_emu.h>

#define clear_Cx() (I387.swd &= ~0x4500)

static void normalize(temp_real * a)
{
	int i = a->exponent & 0x7fff;
	int sign = a->exponent & 0x8000;

	if (!(a->a || a->b)) {
		a->exponent = 0;
		return;
	}
	while (i && a->b >= 0) {
		i--;
		__asm__("addl %0,%0 ; adcl %1,%1"
			:"=r" (a->a),"=r" (a->b)
			:"0" (a->a),"1" (a->b));
	}
	a->exponent = i | sign;
}

void ftst(const temp_real * a)
{
	temp_real b;

	clear_Cx();
	b = *a;
	normalize(&b);
	if (b.a || b.b || b.exponent) {
		if (b.exponent < 0)
			set_C0();
	} else
		set_C3();
}

void fcom(const temp_real * src1, const temp_real * src2)
{
	temp_real a;

	a = *src1;
	a.exponent ^= 0x8000;
	fadd(&a,src2,&a);
	ftst(&a);
}

void fucom(const temp_real * src1, const temp_real * src2)
{
	fcom(src1,src2);
}
