/*
 * linux/kernel/math/ea.c
 *
 * (C) 1991 Linus Torvalds
 */

/*
 * Calculate the effective address.
 */

#include <stddef.h>

#include <linux/math_emu.h>
#include <asm/segment.h>

static int __regoffset[] = {
	offsetof(struct info,___eax),
	offsetof(struct info,___ecx),
	offsetof(struct info,___edx),
	offsetof(struct info,___ebx),
	offsetof(struct info,___esp),
	offsetof(struct info,___ebp),
	offsetof(struct info,___esi),
	offsetof(struct info,___edi)
};

#define REG(x) (*(long *)(__regoffset[(x)]+(char *) info))

static char * sib(struct info * info, int mod)
{
	unsigned char ss,index,base;
	long offset = 0;

	base = get_fs_byte((char *) EIP);
	EIP++;
	ss = base >> 6;
	index = (base >> 3) & 7;
	base &= 7;
	if (index == 4)
		offset = 0;
	else
		offset = REG(index);
	offset <<= ss;
	if (mod || base != 5)
		offset += REG(base);
	if (mod == 1) {
		offset += (signed char) get_fs_byte((char *) EIP);
		EIP++;
	} else if (mod == 2 || base == 5) {
		offset += (signed) get_fs_long((unsigned long *) EIP);
		EIP += 4;
	}
	I387.foo = offset;
	I387.fos = 0x17;
	return (char *) offset;
}

char * ea(struct info * info, unsigned short code)
{
	unsigned char mod,rm;
	long * tmp = &EAX;
	int offset = 0;

	mod = (code >> 6) & 3;
	rm = code & 7;
	if (rm == 4 && mod != 3)
		return sib(info,mod);
	if (rm == 5 && !mod) {
		offset = get_fs_long((unsigned long *) EIP);
		EIP += 4;
		I387.foo = offset;
		I387.fos = 0x17;
		return (char *) offset;
	}
	tmp = & REG(rm);
	switch (mod) {
		case 0: offset = 0; break;
		case 1:
			offset = (signed char) get_fs_byte((char *) EIP);
			EIP++;
			break;
		case 2:
			offset = (signed) get_fs_long((unsigned long *) EIP);
			EIP += 4;
			break;
		case 3:
			math_abort(info,1<<(SIGILL-1));
	}
	I387.foo = offset;
	I387.fos = 0x17;
	return offset + (char *) *tmp;
}
