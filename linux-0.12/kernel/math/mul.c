/*
 * linux/kernel/math/mul.c
 *
 * (C) 1991 Linus Torvalds
 */

/*
 * temporary real multiplication routine.
 */

#include <linux/math_emu.h>

static void shift(int * c)
{
	__asm__("movl (%0),%%eax ; addl %%eax,(%0)\n\t"
		"movl 4(%0),%%eax ; adcl %%eax,4(%0)\n\t"
		"movl 8(%0),%%eax ; adcl %%eax,8(%0)\n\t"
		"movl 12(%0),%%eax ; adcl %%eax,12(%0)"
		::"r" ((long) c):"ax");
}

static void mul64(const temp_real * a, const temp_real * b, int * c)
{
	__asm__("movl (%0),%%eax\n\t"
		"mull (%1)\n\t"
		"movl %%eax,(%2)\n\t"
		"movl %%edx,4(%2)\n\t"
		"movl 4(%0),%%eax\n\t"
		"mull 4(%1)\n\t"
		"movl %%eax,8(%2)\n\t"
		"movl %%edx,12(%2)\n\t"
		"movl (%0),%%eax\n\t"
		"mull 4(%1)\n\t"
		"addl %%eax,4(%2)\n\t"
		"adcl %%edx,8(%2)\n\t"
		"adcl $0,12(%2)\n\t"
		"movl 4(%0),%%eax\n\t"
		"mull (%1)\n\t"
		"addl %%eax,4(%2)\n\t"
		"adcl %%edx,8(%2)\n\t"
		"adcl $0,12(%2)"
		::"b" ((long) a),"c" ((long) b),"D" ((long) c)
		:"ax","dx");
}

void fmul(const temp_real * src1, const temp_real * src2, temp_real * result)
{
	int i,sign;
	int tmp[4] = {0,0,0,0};

	sign = (src1->exponent ^ src2->exponent) & 0x8000;
	i = (src1->exponent & 0x7fff) + (src2->exponent & 0x7fff) - 16383 + 1;
	if (i<0) {
		result->exponent = sign;
		result->a = result->b = 0;
		return;
	}
	if (i>0x7fff) {
		set_OE();
		return;
	}
	mul64(src1,src2,tmp);
	if (tmp[0] || tmp[1] || tmp[2] || tmp[3])
		while (i && tmp[3] >= 0) {
			i--;
			shift(tmp);
		}
	else
		i = 0;
	result->exponent = i | sign;
	result->a = tmp[2];
	result->b = tmp[3];
}
