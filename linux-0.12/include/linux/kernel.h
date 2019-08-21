/*
 * 'kernel.h' contains some often-used function prototypes etc
 */
void verify_area(void * addr,int count);
volatile void panic(const char * str);
volatile void do_exit(long error_code);
int printf(const char * fmt, ...);
int printk(const char * fmt, ...);
void console_print(const char * str);
int tty_write(unsigned ch,char * buf,int count);
void * malloc(unsigned int size);
void free_s(void * obj, int size);
extern void hd_times_out(void);
extern void sysbeepstop(void);
extern void blank_screen(void);
extern void unblank_screen(void);

extern int beepcount;
extern int hd_timeout;
extern int blankinterval;
extern int blankcount;

#define free(x) free_s((x), 0)

/*
 * This is defined as a macro, but at some point this might become a
 * real subroutine that sets a flag if it returns true (to do
 * BSD-style accounting where the process is flagged if it uses root
 * privs).  The implication of this is that you should do normal
 * permissions checks first, and check suser() last.
 */
#define suser() (current->euid == 0)

