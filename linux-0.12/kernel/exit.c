/*
 *  linux/kernel/exit.c
 *
 *  (C) 1991  Linus Torvalds
 */

#define DEBUG_PROC_TREE

#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/tty.h>
#include <asm/segment.h>

int sys_pause(void);
int sys_close(int fd);

void release(struct task_struct * p)
{
	int i;

	if (!p)
		return;
	if (p == current) {
		printk("task releasing itself\n\r");
		return;
	}
	for (i=1 ; i<NR_TASKS ; i++)
		if (task[i]==p) {
			task[i]=NULL;
			/* Update links */
			if (p->p_osptr)
				p->p_osptr->p_ysptr = p->p_ysptr;
			if (p->p_ysptr)
				p->p_ysptr->p_osptr = p->p_osptr;
			else
				p->p_pptr->p_cptr = p->p_osptr;
			free_page((long)p);
			schedule();
			return;
		}
	panic("trying to release non-existent task");
}

#ifdef DEBUG_PROC_TREE
/*
 * Check to see if a task_struct pointer is present in the task[] array
 * Return 0 if found, and 1 if not found.
 */
int bad_task_ptr(struct task_struct *p)
{
	int 	i;

	if (!p)
		return 0;
	for (i=0 ; i<NR_TASKS ; i++)
		if (task[i] == p)
			return 0;
	return 1;
}
	
/*
 * This routine scans the pid tree and make sure the rep invarient still
 * holds.  Used for debugging only, since it's very slow....
 *
 * It looks a lot scarier than it really is.... we're doing ænothing more
 * than verifying the doubly-linked list foundæin p_ysptr and p_osptr, 
 * and checking it corresponds with the process tree defined by p_cptr and 
 * p_pptr;
 */
void audit_ptree()
{
	int	i;

	for (i=1 ; i<NR_TASKS ; i++) {
		if (!task[i])
			continue;
		if (bad_task_ptr(task[i]->p_pptr))
			printk("Warning, pid %d's parent link is bad\n",
				task[i]->pid);
		if (bad_task_ptr(task[i]->p_cptr))
			printk("Warning, pid %d's child link is bad\n",
				task[i]->pid);
		if (bad_task_ptr(task[i]->p_ysptr))
			printk("Warning, pid %d's ys link is bad\n",
				task[i]->pid);
		if (bad_task_ptr(task[i]->p_osptr))
			printk("Warning, pid %d's os link is bad\n",
				task[i]->pid);
		if (task[i]->p_pptr == task[i])
			printk("Warning, pid %d parent link points to self\n");
		if (task[i]->p_cptr == task[i])
			printk("Warning, pid %d child link points to self\n");
		if (task[i]->p_ysptr == task[i])
			printk("Warning, pid %d ys link points to self\n");
		if (task[i]->p_osptr == task[i])
			printk("Warning, pid %d os link points to self\n");
		if (task[i]->p_osptr) {
			if (task[i]->p_pptr != task[i]->p_osptr->p_pptr)
				printk(
			"Warning, pid %d older sibling %d parent is %d\n",
				task[i]->pid, task[i]->p_osptr->pid,
				task[i]->p_osptr->p_pptr->pid);
			if (task[i]->p_osptr->p_ysptr != task[i])
				printk(
		"Warning, pid %d older sibling %d has mismatched ys link\n",
				task[i]->pid, task[i]->p_osptr->pid);
		}
		if (task[i]->p_ysptr) {
			if (task[i]->p_pptr != task[i]->p_ysptr->p_pptr)
				printk(
			"Warning, pid %d younger sibling %d parent is %d\n",
				task[i]->pid, task[i]->p_osptr->pid,
				task[i]->p_osptr->p_pptr->pid);
			if (task[i]->p_ysptr->p_osptr != task[i])
				printk(
		"Warning, pid %d younger sibling %d has mismatched os link\n",
				task[i]->pid, task[i]->p_ysptr->pid);
		}
		if (task[i]->p_cptr) {
			if (task[i]->p_cptr->p_pptr != task[i])
				printk(
			"Warning, pid %d youngest child %d has mismatched parent link\n",
				task[i]->pid, task[i]->p_cptr->pid);
			if (task[i]->p_cptr->p_ysptr)
				printk(
			"Warning, pid %d youngest child %d has non-NULL ys link\n",
				task[i]->pid, task[i]->p_cptr->pid);
		}
	}
}
#endif /* DEBUG_PROC_TREE */

static inline int send_sig(long sig,struct task_struct * p,int priv)
{
	if (!p)
		return -EINVAL;
	if (!priv && (current->euid!=p->euid) && !suser())
		return -EPERM;
	if ((sig == SIGKILL) || (sig == SIGCONT)) {
		if (p->state == TASK_STOPPED)
			p->state = TASK_RUNNING;
		p->exit_code = 0;
		p->signal &= ~( (1<<(SIGSTOP-1)) | (1<<(SIGTSTP-1)) |
				(1<<(SIGTTIN-1)) | (1<<(SIGTTOU-1)) );
	} 
	/* If the signal will be ignored, don't even post it */
	if ((int) p->sigaction[sig-1].sa_handler == 1)
		return 0;
	/* Depends on order SIGSTOP, SIGTSTP, SIGTTIN, SIGTTOU */
	if ((sig >= SIGSTOP) && (sig <= SIGTTOU)) 
		p->signal &= ~(1<<(SIGCONT-1));
	/* Actually deliver the signal */
	p->signal |= (1<<(sig-1));
	return 0;
}

int session_of_pgrp(int pgrp)
{
	struct task_struct **p;

 	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p)
		if ((*p)->pgrp == pgrp)
			return((*p)->session);
	return -1;
}

int kill_pg(int pgrp, int sig, int priv)
{
	struct task_struct **p;
	int err,retval = -ESRCH;
	int found = 0;

	if (sig<1 || sig>32 || pgrp<=0)
		return -EINVAL;
 	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p)
		if ((*p)->pgrp == pgrp) {
			if (sig && (err = send_sig(sig,*p,priv)))
				retval = err;
			else
				found++;
		}
	return(found ? 0 : retval);
}

int kill_proc(int pid, int sig, int priv)
{
 	struct task_struct **p;

	if (sig<1 || sig>32)
		return -EINVAL;
	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p)
		if ((*p)->pid == pid)
			return(sig ? send_sig(sig,*p,priv) : 0);
	return(-ESRCH);
}

/*
 * POSIX specifies that kill(-1,sig) is unspecified, but what we have
 * is probably wrong.  Should make it like BSD or SYSV.
 */
int sys_kill(int pid,int sig)
{
	struct task_struct **p = NR_TASKS + task;
	int err, retval = 0;

	if (!pid)
		return(kill_pg(current->pid,sig,0));
	if (pid == -1) {
		while (--p > &FIRST_TASK)
			if (err = send_sig(sig,*p,0))
				retval = err;
		return(retval);
	}
	if (pid < 0) 
		return(kill_pg(-pid,sig,0));
	/* Normal kill */
	return(kill_proc(pid,sig,0));
}

/*
 * Determine if a process group is "orphaned", according to the POSIX
 * definition in 2.2.2.52.  Orphaned process groups are not to be affected
 * by terminal-generated stop signals.  Newly orphaned process groups are 
 * to receive a SIGHUP and a SIGCONT.
 * 
 * "I ask you, have you ever known what it is to be an orphan?"
 */
int is_orphaned_pgrp(int pgrp)
{
	struct task_struct **p;

	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
		if (!(*p) ||
		    ((*p)->pgrp != pgrp) || 
		    ((*p)->state == TASK_ZOMBIE) ||
		    ((*p)->p_pptr->pid == 1))
			continue;
		if (((*p)->p_pptr->pgrp != pgrp) &&
		    ((*p)->p_pptr->session == (*p)->session))
			return 0;
	}
	return(1);	/* (sighing) "Often!" */
}

static int has_stopped_jobs(int pgrp)
{
	struct task_struct ** p;

	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
		if ((*p)->pgrp != pgrp)
			continue;
		if ((*p)->state == TASK_STOPPED)
			return(1);
	}
	return(0);
}

volatile void do_exit(long code)
{
	struct task_struct *p;
	int i;

	free_page_tables(get_base(current->ldt[1]),get_limit(0x0f));
	free_page_tables(get_base(current->ldt[2]),get_limit(0x17));
	for (i=0 ; i<NR_OPEN ; i++)
		if (current->filp[i])
			sys_close(i);
	iput(current->pwd);
	current->pwd = NULL;
	iput(current->root);
	current->root = NULL;
	iput(current->executable);
	current->executable = NULL;
	iput(current->library);
	current->library = NULL;
	current->state = TASK_ZOMBIE;
	current->exit_code = code;
	/* 
	 * Check to see if any process groups have become orphaned
	 * as a result of our exiting, and if they have any stopped
	 * jobs, send them a SIGUP and then a SIGCONT.  (POSIX 3.2.2.2)
	 *
	 * Case i: Our father is in a different pgrp than we are
	 * and we were the only connection outside, so our pgrp
	 * is about to become orphaned.
 	 */
	if ((current->p_pptr->pgrp != current->pgrp) &&
	    (current->p_pptr->session == current->session) &&
	    is_orphaned_pgrp(current->pgrp) &&
	    has_stopped_jobs(current->pgrp)) {
		kill_pg(current->pgrp,SIGHUP,1);
		kill_pg(current->pgrp,SIGCONT,1);
	}
	/* Let father know we died */
	current->p_pptr->signal |= (1<<(SIGCHLD-1));
	
	/*
	 * This loop does two things:
	 * 
  	 * A.  Make init inherit all the child processes
	 * B.  Check to see if any process groups have become orphaned
	 *	as a result of our exiting, and if they have any stopped
	 *	jons, send them a SIGUP and then a SIGCONT.  (POSIX 3.2.2.2)
	 */
	if (p = current->p_cptr) {
		while (1) {
			p->p_pptr = task[1];
			if (p->state == TASK_ZOMBIE)
				task[1]->signal |= (1<<(SIGCHLD-1));
			/*
			 * process group orphan check
			 * Case ii: Our child is in a different pgrp 
			 * than we are, and it was the only connection
			 * outside, so the child pgrp is now orphaned.
			 */
			if ((p->pgrp != current->pgrp) &&
			    (p->session == current->session) &&
			    is_orphaned_pgrp(p->pgrp) &&
			    has_stopped_jobs(p->pgrp)) {
				kill_pg(p->pgrp,SIGHUP,1);
				kill_pg(p->pgrp,SIGCONT,1);
			}
			if (p->p_osptr) {
				p = p->p_osptr;
				continue;
			}
			/*
			 * This is it; link everything into init's children 
			 * and leave 
			 */
			p->p_osptr = task[1]->p_cptr;
			task[1]->p_cptr->p_ysptr = p;
			task[1]->p_cptr = current->p_cptr;
			current->p_cptr = 0;
			break;
		}
	}
	if (current->leader) {
		struct task_struct **p;
		struct tty_struct *tty;

		if (current->tty >= 0) {
			tty = TTY_TABLE(current->tty);
			if (tty->pgrp>0)
				kill_pg(tty->pgrp, SIGHUP, 1);
			tty->pgrp = 0;
			tty->session = 0;
		}
	 	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p)
			if ((*p)->session == current->session)
				(*p)->tty = -1;
	}
	if (last_task_used_math == current)
		last_task_used_math = NULL;
#ifdef DEBUG_PROC_TREE
	audit_ptree();
#endif
	schedule();
}

int sys_exit(int error_code)
{
	do_exit((error_code&0xff)<<8);
}

int sys_waitpid(pid_t pid,unsigned long * stat_addr, int options)
{
	int flag;
	struct task_struct *p;
	unsigned long oldblocked;

	verify_area(stat_addr,4);
repeat:
	flag=0;
	for (p = current->p_cptr ; p ; p = p->p_osptr) {
		if (pid>0) {
			if (p->pid != pid)
				continue;
		} else if (!pid) {
			if (p->pgrp != current->pgrp)
				continue;
		} else if (pid != -1) {
			if (p->pgrp != -pid)
				continue;
		}
		switch (p->state) {
			case TASK_STOPPED:
				if (!(options & WUNTRACED) || 
				    !p->exit_code)
					continue;
				put_fs_long((p->exit_code << 8) | 0x7f,
					stat_addr);
				p->exit_code = 0;
				return p->pid;
			case TASK_ZOMBIE:
				current->cutime += p->utime;
				current->cstime += p->stime;
				flag = p->pid;
				put_fs_long(p->exit_code, stat_addr);
				release(p);
#ifdef DEBUG_PROC_TREE
				audit_ptree();
#endif
				return flag;
			default:
				flag=1;
				continue;
		}
	}
	if (flag) {
		if (options & WNOHANG)
			return 0;
		current->state=TASK_INTERRUPTIBLE;
		oldblocked = current->blocked;
		current->blocked &= ~(1<<(SIGCHLD-1));
		schedule();
		current->blocked = oldblocked;
		if (current->signal & ~(current->blocked | (1<<(SIGCHLD-1))))
			return -ERESTARTSYS;
		else
			goto repeat;
	}
	return -ECHILD;
}


