
#include "param.h"
#include "user.h"
#include "proc.h"
#include "systm.h"
#include "file.h"
#include "inode.h"
#include "buf.h"
#include "mmu.h"
#include "reg.h"

extern int maxproc;

/*
 * Wake up process if sleeping on chan.
 */
void
wakeup(chan)
	int chan;
{
	register struct proc *p;
	register c, i;

	c = chan;
	p = &proc[0];
	i = maxproc;
	do {
		if(p->p_wchan == c) {
			setrun(p);
		}
		p++;
	} while(--i);
}

/*
 * Give up the processor till a wakeup occurs
 * on chan, at which time the process
 * enters the scheduling queue at priority pri.
 * The most important effect of pri is that when
 * pri<0 a signal cannot disturb the sleep;
 * if pri>=0 signals will be processed.
 * Callers of this routine must be prepared for
 * premature return, and check that the reason for
 * sleeping has gone away.
 */
void
sleep(chan, pri)
{
	register struct proc *rp;

	rp = u.u_procp;
	if(pri >= 0) {
		if(issig())
			goto psig;
		spl7();
		rp->p_wchan = chan;
		rp->p_stat  = SWAIT;
		rp->p_pri   = pri;
		spl0();
		if(runin != 0) {
			runin = 0;
			wakeup(&runin);
		}
		swtch();
		if(issig())
			goto psig;
	} else {
		spl7();
		rp->p_wchan = chan;
		rp->p_stat  = SSLEEP;
		rp->p_pri   = pri;
		spl0();
		swtch();
	}
	return;

	/*
	 * If priority was low (>=0) and
	 * there has been a signal,
	 * execute non-local goto to
	 * the qsav location.
	 * (see trap1/trap.c)
	 */
psig:
	aretu(u.u_qsav);
}

/*
 * Set the process running;
 * arrange for it to be swapped in if necessary.
 */
void
setrun(p)
	register struct proc *p;
{
	p->p_wchan = 0;
	p->p_stat = SRUN;
	if(p->p_pri < curpri)
		runrun++;
	if(runout != 0 && (p->p_flag&SLOAD) == 0) {
		runout = 0;
		wakeup(&runout);
	}
}

/*
 * Set user priority.
 * The rescheduling flag (runrun)
 * is set if the priority is higher
 * than the currently running process.
 */
setpri(up)
	register struct proc *up;
{
	register pri;

	pri = (up->p_cpu & 0377)/16;
	pri += PUSER + up->p_nice;
	if(pri > 127)
		pri = 127;
	if(pri > curpri)
		runrun++;
	up->p_pri = pri;
}

/*
 * Swap out process p.
 * The ff flag causes its core to be freed--
 * it is off when called to create an image for a
 * child process in newproc.
 *
 * panic: out of swap space
 * panic: swap error -- IO error
 */
void
xswap(rp, ff)
	struct proc *rp;
{
	register unsigned int swpslot;

	swpslot = swpget(0);
	if(swpslot == NULL)
		panic("out of swap space");
	rp->p_flag |= SLOCK;
	if(swap(swpslot, rp->p_slot, B_WRITE))
		panic("swap error");
	if(ff)
		memfree(rp->p_slot);
	rp->p_slot  = swpslot;
	rp->p_flag &= ~(SLOAD|SLOCK);
	rp->p_time  = 0;
	if(runout) {
		runout = 0;
		wakeup(&runout);
	}
}

/*
 * The main loop of the scheduling (swapping)
 * process.
 * The basic idea is:
 *  see if anyone wants to be swapped in;
 *  swap out processes until there is room;
 *  swap him in;
 *  repeat.
 * Although it is not remarkably evident, the basic
 * synchronization here is on the runin flag, which is
 * slept on and is set once per second by the clock routine.
 * Core shuffling therefore takes place once per second.
 *
 * panic: swap error -- IO error while swapping.
 *	this is the one panic that should be
 *	handled in a less drastic way. Its
 *	very hard.
 */
void
sched()
{
	struct proc *p1;
	register struct proc *rp;
	register int slot, n;

	/*
	 * find user to swap in
	 * of users ready, select one out longest
	 */

	goto loop;

sloop:
	runin++;
	sleep(&runin, PSWP);

loop:
	spl7();
	n = -1;
	for(rp = &proc[0]; rp < &proc[maxproc]; rp++)
		if(rp->p_stat==SRUN && (rp->p_flag&SLOAD)==0 &&
		    rp->p_time > n) {
			p1 = rp;
			n = rp->p_time;
		}
	if(n == -1) {
		runout++;
		sleep(&runout, PSWP);
		goto loop;
	}

	/*
	 * see if there is core for that process
	 */

	spl0();
	rp = p1;
	if((slot=memget()) != NULL)
		goto found2;

	/*
	 * none found,
	 * look around for easy core
	 */

	spl7();
	for(rp = &proc[0]; rp < &proc[maxproc]; rp++)
		if((rp->p_flag&(SSYS|SLOCK|SLOAD))==SLOAD &&
		    (rp->p_stat == SWAIT || rp->p_stat==SSTOP))
			goto found1;

	/*
	 * no easy core,
	 * if this process is deserving,
	 * look around for
	 * oldest process in core
	 */

	if(n < 3)
		goto sloop;
	n = -1;
	for(rp = &proc[0]; rp < &proc[maxproc]; rp++)
		if((rp->p_flag&(SSYS|SLOCK|SLOAD))==SLOAD &&
		   (rp->p_stat==SRUN || rp->p_stat==SSLEEP) &&
		    rp->p_time > n) {
			p1 = rp;
			n = rp->p_time;
	}
	if(n < 2)
		goto sloop;
	rp = p1;

	/*
	 * swap user out
	 */

found1:
	spl0();
	rp->p_flag &= ~SLOAD;
	xswap(rp, 1);
	goto loop;

	/*
	 * swap user in
	 */

found2:
	/* Swap in the process at swap slot 'rp->p_slot' to
	 * memory slot 'slot'. Free the swap slot.
	 */
	rp = p1;
	if(swap(rp->p_slot, slot, B_READ))
		goto swaper;
	swpfree(rp->p_slot);
	rp->p_slot  = slot;
	rp->p_flag |= SLOAD;
	rp->p_time  = 0;
	goto loop;

swaper:
	panic("swap error");
}

/*
 * This routine is called to reschedule the CPU.
 * if the calling process is not in RUN state,
 * arrangements for it to restart must have
 * been made elsewhere, usually by calling via sleep.
 */
int
swtch()
{
	static struct proc *p;
	register i, n;
	register struct proc *rp;

	if(p == NULL)
		p = &proc[0];
	/*
	 * Remember stack of caller
	 */
	savu(u.u_rsav);

	/*
	 * Switch to scheduler's stack
	 */
	retu(proc[0].p_slot);

loop:
	runrun = 0;
	rp = p;
	p = NULL;
	n = 128;
	/*
	 * Search for highest-priority runnable process
	 */
	i = maxproc;
	do {
		rp++;
		if(rp >= &proc[maxproc])
			rp = &proc[0];
		if(rp->p_stat==SRUN && (rp->p_flag&SLOAD)!=0) {
			if(rp->p_pri < n) {
				p = rp;
				n = rp->p_pri;
			}
		}
	} while(--i);
	/*
	 * If no process is runnable, idle.
	 */
	if(p == NULL) {
		p = rp;
		idle();
		goto loop;
	}
	rp = p;
	curpri = n;
	/*
	 * Switch to the u area (and stack) of the new process and
	 * set up its MMU registers.
	 */
	retu(rp->p_slot);
	sureg();
	/*
	 * If the new process paused because it was
	 * swapped out, set the stack level to the last call
	 * to savu(u_ssav).  This means that the return
	 * which is executed immediately after the call to aretu
	 * actually returns from the last routine which did
	 * the savu (newproc, expand or xalloc).
	 *
	 * You are not expected to understand this.
	 */
	if(rp->p_flag&SSWAP) {
		rp->p_flag &= ~SSWAP;
		aretu(u.u_ssav);
	}

	/*
	 * If network process request, copy error and return values to u area.
	 */
	if (rp->p_flag & SNET) {
		rp->p_flag &= ~SNET;
		u.u_ar0[R0] = rp->p_nretval;
		u.u_error = rp->p_nerror;;
	}

	/*
	 * The value returned here has many subtle implications.
	 * See the newproc comments.
	 */
	return(1);
}

/*
 * Create a new process-- the internal version of
 * sys fork.
 * It returns 1 in the new process.
 * How this happens is rather hard to understand.
 * The essential fact is that the new process is created
 * in such a way that appears to have started executing
 * in the same call to newproc as the parent;
 * but in fact the code that runs is that of swtch.
 * The subtle implication of the returned value of swtch
 * (see above) is that this is the value that newproc's
 * caller in the new process sees.
 */
int
newproc()
{
	int a1, a2;
	struct proc *p, *up;
	register struct proc *rpp, *rip;
	register struct file **rfp;
	register int n;

	p = NULL;
	/*
	 * First, just locate a pid and a slot for a process
	 * and copy the useful info from this process into it.
	 * The panic "cannot happen" because fork has already
	 * checked for the existence of a slot.
	 */
retry:
	mpid++;
	if (mpid == &netmain) mpid++; /* skip network channel */
	if(mpid < 0) {
		mpid = 0;
		goto retry;
	}
	for(rpp = &proc[0]; rpp < &proc[maxproc]; rpp++) {
		if(rpp->p_stat == NULL && p==NULL)
			p = rpp;
		if (rpp->p_pid==mpid)
			goto retry;
	}
	if ((rpp = p)==NULL)
		panic("no procs");

	/*
	 * make proc entry for new proc
	 */
	rip = u.u_procp;
	up = rip;
	rpp->p_stat  = SRUN;
	rpp->p_flag  = SLOAD;
	rpp->p_uid   = rip->p_uid;
	rpp->p_ttyp  = rip->p_ttyp;
	rpp->p_nice  = rip->p_nice;
	/*rpp->p_textp = rip->p_textp; */
	rpp->p_pid   = mpid;
	rpp->p_ppid  = rip->p_pid;
	rpp->p_time  = 0;

	/*
	 * make duplicate entries
	 * where needed
	 */
	for(rfp = &u.u_ofile[0]; rfp < &u.u_ofile[NOFILE]; rfp++)
		if(*rfp)
			(*rfp)->f_count++;

	u.u_cdir->i_count++;

	/*
	 * Partially simulate the environment
	 * of the new process so that when it is actually
	 * created (by copying) it will look right.
	 */
	savu(u.u_rsav);
	rpp = p;
	u.u_procp = rpp;
	rip = up;
	a1 = rip->p_slot;
	a2 = memget();

	/*
	 * If there is not enough core for the
	 * new process, swap out the current process to generate the
	 * copy.
	 */
	if(a2 == NULL) {
		rip->p_stat = SIDL;
		rpp->p_slot = a1;
		savu(u.u_ssav);
		xswap(rpp, 0);
		rpp->p_flag |= SSWAP;
		rip->p_stat  = SRUN;
	} else {
	/*
	 * There is core, so just copy.
	 */
		rpp->p_slot = a2;
		copyimg(a2, a1);
	}
	u.u_procp = rip;
	return(0);
}

extern int user;
#include "reg.h"

void
prproc()
{
	int i;
	register struct proc *p;
	
	printk("\nPROC\t\tSLOT\tPID\tSTATE\tFLAGS\tSIGNAL\tSLEEP\tSIGINT\n");
	for(i=0;i<maxproc;i++) {
		p = &proc[i];
		if(p->p_stat==0) continue;
		printk("%d\t%x\t%d\t%d\t%x\t%x\t%d\t%x\t%x\n", i, p, p->p_slot, p->p_pid, p->p_stat, p->p_flag, p->p_sig, p->p_wchan, u.u_signal[SIGINT]);
	}
	printk("Current process is %d (PC=%x, %s mode)\n", u.u_procp->p_pid, u.u_ar0[PPC], user?"user":"kernel");
}
