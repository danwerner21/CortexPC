
#include "param.h"
#include "systm.h"
#include "user.h"
#include "reg.h"

#define	EBIT	0x1000		/* user error bit in PS: C-bit */
#define SYSCAL	0x0000		/* system call */
#define BPTTRP	0x0040		/* breakpoint or trap */
#define ERRTRP	0x0080		/* Error trap */

/*
 * Call the system-entry routine f (out of the
 * sysent table). This is a subroutine for trap, and
 * not in-line, because if a signal occurs
 * during processing, an (abnormal) return is simulated from
 * the last caller to savu(qsav); if this took place
 * inside of trap, it wouldn't have a chance to clean up.
 *
 * If this occurs, the return takes place without
 * clearing u_intflg; if it's still set, trap
 * marks an error which means that a system
 * call (like read on a typewriter) got interrupted
 * by a signal.
 */
void
trap1(f)
int (*f)();
{

	u.u_intflg = 1;
	savu(u.u_qsav);
	(*f)();
	u.u_intflg = 0;
}

/*
 * Called from low.s/mch.s when a processor trap occurs.
 * The arguments are the words saved on the system stack
 * by the hardware and software during the trap processing.
 * Their order is dictated by the hardware and the details
 * of C's calling sequence. They are peculiar in that
 * this call is not 'by value' and changed user registers
 * get copied back on return.
 * xea is the kind of trap that occurred.
 */
void
trap(xea, pwp, psp, ppc, pst)
	int *pwp, *psp, *ppc;
{
	register int i, a, *p, t;
	register struct sysent *callp;
	p = (int *)((int)&u + sizeof (struct user));
	if (*p != 0xa5a5)
		panic("stack overflow");

	u.u_ar0 = &xea;
	t = xea & 0xc0;
	
	switch(t) {

	/*
	 * Trap not expected.
	 */
	default:
		printk("trap type %d\n", xea);
		panic("unknown trap");

	case BPTTRP: /* breakpoint or trap */
		i = SIGTRC;
		break;

	case SYSCAL: /* sys call */
		u.u_error = 0;
		pst &= ~EBIT;
		callp = &sysent[xea&0x3f];
		if (callp == sysent) { /* indirect */
			a = u.u_ar0[R0];
			i = fuword(a);
			
			if (i & ~0x3f)
				i = 0x3f;	/* illegal */
			callp = &sysent[i];
			for(i=0; i<callp->count; i++)
				u.u_arg[i] = fuword(a += 2);
		} else {			
			p = psp;
			for(i=0; i<callp->count; i++) {
				u.u_arg[i] = fuword(p++);
			}
		}
		u.u_ar0[R0] = 0;
		u.u_dirp = (char*) u.u_arg[0];
		trap1(callp->call);
		if(u.u_intflg)
			u.u_error = EINTR;
		if(u.u_error < 100) {
			if(u.u_error) {
				pst |= EBIT;
				u.u_ar0[R0] = u.u_error;
			}
			goto out;
		}
		i = SIGSYS;
		break;

	/* The pdp11 source has a case here to deal with page faults.
	 * Consider this for the 99105 edition. Case 2.
	 */
	case ERRTRP: /* Error trap */
		switch (xea & 0xf) {
		case 1:
			i = SIGFPT;
			break;
		case 2:
			i = SIGINS;
			break;
		case 3:
			i = SIGSEG;
			break;
		case 4:
			i = SIGBUS;
			break;
		default:
			printk("error trap type %d\n", xea & 0xf);
			panic("unknown error trap");
		}
		break;


	}
	psignal(u.u_procp, i);
out:
	if(issig())
		psig();
}

/*
 * nonexistent system call-- set fatal error code.
 */
void
nosys()
{
	u.u_error = 100;
}

/*
 * Ignored system call
 */
void
nullsys()
{
}
