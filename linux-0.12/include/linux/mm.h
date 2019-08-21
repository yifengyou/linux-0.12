#ifndef _MM_H
#define _MM_H

#define PAGE_SIZE 4096

#include <linux/kernel.h>
#include <signal.h>

extern int SWAP_DEV;

#define read_swap_page(nr,buffer) ll_rw_page(READ,SWAP_DEV,(nr),(buffer));
#define write_swap_page(nr,buffer) ll_rw_page(WRITE,SWAP_DEV,(nr),(buffer));

extern unsigned long get_free_page(void);
extern unsigned long put_dirty_page(unsigned long page,unsigned long address);
extern void free_page(unsigned long addr);
void swap_free(int page_nr);
void swap_in(unsigned long *table_ptr);

extern inline volatile void oom(void)
{
	printk("out of memory\n\r");
	do_exit(SIGSEGV);
}

#define invalidate() \
__asm__("movl %%eax,%%cr3"::"a" (0))

/* these are not to be changed without changing head.s etc */
#define LOW_MEM 0x100000
extern unsigned long HIGH_MEMORY;
#define PAGING_MEMORY (15*1024*1024)
#define PAGING_PAGES (PAGING_MEMORY>>12)
#define MAP_NR(addr) (((addr)-LOW_MEM)>>12)
#define USED 100

extern unsigned char mem_map [ PAGING_PAGES ];

#define PAGE_DIRTY	0x40
#define PAGE_ACCESSED	0x20
#define PAGE_USER	0x04
#define PAGE_RW		0x02
#define PAGE_PRESENT	0x01

#endif
