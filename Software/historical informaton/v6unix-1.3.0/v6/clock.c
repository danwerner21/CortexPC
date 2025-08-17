
#include "param.h"
#include "systm.h"
#include "user.h"
#include "proc.h"

#define USER	0x100
#define SCHMAG	10

extern int maxproc;

/*
 * clock is called straight from
 * the real time clock interrupt.
 *
 * Functions:
 *	maintain user/system times
 *	profile
 *	maintain date
 *	tout wakeup (sys sleep)
 *	alarm clock signals
 *	jab the scheduler
 */

void clock(xea)
{
	register struct proc *pp;

	/* update process time variables */
	/* FIXME handle this at 60Hz in low.s? */
	if(xea & USER) {
		u.u_utime += HZ;
	} else
		u.u_stime += HZ;

	pp = u.u_procp;
	if ((pp->p_cpu & 0377) < 240)
		pp->p_cpu += HZ;

	++time;

	/* network main loop runs at least once per second */
	wakeup (&netmain);

	for (pp = &proc[0]; pp < &proc[maxproc]; pp++) {

		/* Wake up any sleepers */
		if(pp->p_tout && time > pp->p_tout)
			wakeup(&pp->p_tout);

		/* update scheduling variables */
		if (pp->p_stat) {
			if(pp->p_time != 127)
				pp->p_time++;
			if((pp->p_cpu & 0377) > SCHMAG)
				pp->p_cpu -= SCHMAG;
			else
				pp->p_cpu = 0;
			if(pp->p_pri > PUSER)
				setpri(pp);
		}
	}

	/* if it is ready, run the scheduler */
	if(runin!=0) {
		runin = 0;
		wakeup(&runin);
	}

	/* if we interrupted a user mode process, handle signals if any */
	if(xea & USER) {
		u.u_ar0 = &xea;
		if(issig())
			psig();
		setpri(u.u_procp);
	}
}
