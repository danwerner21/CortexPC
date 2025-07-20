
#include "param.h"
#include "systm.h"
#include "user.h"
#include "proc.h"
#include "inode.h"
#include "reg.h"

/*
 * Priority for tracing
 */
#define	IPCPRI	(-1)

extern int maxproc;

/*
 * Structure to access an array of integers.
 */
struct array
{
	int	inta[1];
};

/*
 * Tracing variables.
 * Used to pass trace command from
 * parent to child being traced.
 * This data base cannot be
 * shared and is locked
 * per user.
 */
struct
{
	int	ip_lock;
	int	ip_req;
	int	ip_addr;
	int	ip_data;
} ipc;

/*
 * Send the specified signal to
 * all processes with 'tp' as its
 * controlling teletype.
 * Called by tty.c for quits and
 * interrupts.
 */
void
signal(tp, sig)
{
	register struct proc *p;

	for(p = &proc[0]; p < &proc[maxproc]; p++)
		if(p->p_ttyp == (struct tty *)tp)
			psignal(p, sig);
}

/*
 * Send the specified signal to
 * the specified process.
 */
void
psignal(p, sig)
	register struct proc *p;
	int sig;
{
	if(sig >= NSIG)
		return;
	if(p->p_sig != SIGKIL)
		p->p_sig = sig;
	if(p->p_pri > PUSER)
		p->p_pri = PUSER;
	if(p->p_stat == SWAIT)
		setrun(p);
}

/*
 * Enter the tracing STOP state.
 * In this state, the parent is
 * informed and the process is able to
 * receive commands from the parent.
 */
void
stop()
{
	register struct proc *pp, *cp;

loop:
	cp = u.u_procp;
	if(cp->p_ppid != 1)
		for (pp = &proc[0]; pp < &proc[maxproc]; pp++)
			if (pp->p_pid == cp->p_ppid) {
				wakeup(pp);
				cp->p_stat = SSTOP;
				swtch();
				if ((cp->p_flag&STRC)==0 || procxmt())
					return;
				goto loop;
			}
	pexit();
}

/*
 * Returns true if the current
 * process has a signal to process.
 * This is asked at least once
 * each time a process enters the
 * system.
 * A signal does not do anything
 * directly to a process; it sets
 * a flag that asks the process to
 * do something to itself.
 */
int
issig()
{
	register int n;
	register struct proc *p;

	p = u.u_procp;
	if(n = p->p_sig) {
		if (p->p_flag&STRC) {
			stop();
			if ((n = p->p_sig) == 0)
				return(0);
		}
		if((u.u_signal[n]&1) == 0)
			return(n);
	}
	return(0);
}

/*
 * Perform the action specified by
 * the current signal.
 * The usual sequence is:
 *	if(issig())
 *		psig();
 */
void
psig()
{
	register int n, p;
	register struct proc *rp;

	rp = u.u_procp;
	n = rp->p_sig;
	rp->p_sig = 0;
	p = u.u_signal[n];
	if(p != 0) {
		u.u_error = 0;
		if(n != SIGINS && n != SIGTRC)
			u.u_signal[n] = 0;

		n = u.u_ar0[PSP] - 10;
		suword((int*)(n+8), u.u_ar0[PST]);
		suword((int*)(n+6), u.u_ar0[PPC]);
		suword((int*)(n+4), u.u_ar0[PWP]);
		suword((int*)(n+2), u.u_ar0[R0] );
		suword((int*)(n+0), u.u_ar0[R1] );

		u.u_ar0[PSP] = n;
		u.u_ar0[PPC] = p;
/*		u.u_ar0[XEA] &= ~DLYTRP; /* cancel a pending trace trap */
		return;
	}
	switch(n) {

	case SIGQIT:
	case SIGINS:
	case SIGTRC:
	case SIGIOT:
	case SIGEMT:
	case SIGFPT:
	case SIGBUS:
	case SIGSEG:
	case SIGSYS:
		u.u_arg[0] = n;
#ifdef COREOPT
		if(core())
			n += 0200;
#endif
		break;
	default:
		u.u_arg[0] = (u.u_ar0[R0]<<8) | n;
	}
	pexit();
}

#ifdef COREOPT
/*
 * Create a core image on the file "core"
 * If you are looking for protection glitches,
 * there are probably a wealth of them here
 * when this occurs to a suid command.
 *
 * It writes USIZE block of the
 * user.h area followed by the entire
 * data+stack segments.
 */
int
core()
{
	register s, *ip;
	extern int schar();

	u.u_error = 0;
	u.u_dirp = "core";
	ip = namei(&schar, 1);
	if(ip == NULL) {
		if(u.u_error)
			return(0);
		ip = maknode(0666);
		if(ip == NULL)
			return(0);
	}
	if(!access(ip, IWRITE) &&
	   (ip->i_mode&IFMT) == 0 &&
	   u.u_uid == u.u_ruid) {
		itrunc(ip);
		u.u_offset = 0;
		u.u_base   = &u;
		u.u_count  = USIZE;
		u.u_segflg = 1;
		writei(ip);
		u.u_base   = 0x1000;
		u.u_count  = 0xe000;
		u.u_segflg = 0;
		writei(ip);
	}
	iput(ip);
	return(u.u_error==0);
}
#endif

/*
 * sys-trace system call.
 */
void
ptrace()
{
	register struct proc *p;

	if (u.u_arg[0] <= 0) {
		u.u_procp->p_flag |= STRC;
		return;
	}
	for (p=proc; p < &proc[maxproc]; p++) 
		if (p->p_stat==SSTOP
		 && p->p_pid==u.u_arg[1]
		 && p->p_ppid==u.u_procp->p_pid)
			goto found;
	u.u_error = ESRCH;
	return;

    found:
	while (ipc.ip_lock)
		sleep(&ipc, IPCPRI);
	ipc.ip_lock = p->p_pid;
	ipc.ip_data = u.u_arg[3];
	ipc.ip_addr = u.u_arg[2] & ~01;
	ipc.ip_req  = u.u_arg[0];
	p->p_flag &= ~SWTED;
	setrun(p);
	while (ipc.ip_req > 0)
		sleep(&ipc, IPCPRI);
	u.u_ar0[R0] = ipc.ip_data;
	if (ipc.ip_req < 0)
		u.u_error = EIO;
	ipc.ip_lock = 0;
	wakeup(&ipc);
}

/*
 * Code that the child process
 * executes to implement the command
 * of the parent process in tracing.
 */
int
procxmt()
{
	register int i;
	register int *p;
	struct array *a;

	if (ipc.ip_lock != u.u_procp->p_pid)
		return(0);
	i = ipc.ip_req;
	ipc.ip_req = 0;
	wakeup(&ipc);
	a = (struct array *)&u;
	switch (i) {

	/* read user I, D */
	case 1:
	case 2:
		ipc.ip_data = fuword(ipc.ip_addr);
		break;

	/* read u */
	case 3:
		i = ipc.ip_addr;
		if (i<0 || i >= 1024)
			goto error;
		ipc.ip_data = a->inta[i>>1];
		break;

	/* write user I, D */
	case 4:
	case 5:
		suword(ipc.ip_addr, ipc.ip_data);
		break;

	/* write u, only registers allowed */
	case 6:
		p = &a->inta[ipc.ip_addr>>1];
		if (p >= &u.u_ar0[-11] && p <= &u.u_ar0[10])
			goto ok;
		goto error;
	ok:
		if (p == &u.u_ar0[PST]) {
			ipc.ip_data |= 15;	/* interrupt mask 15 */
		}
		*p = ipc.ip_data;
		break;

	/* set signal and continue */
	case 7:
		u.u_procp->p_sig = ipc.ip_data;
		return(1);

	/* force exit */
	case 8:
		pexit();

	default:
	error:
		ipc.ip_req = -1;
	}
	return(0);
}
