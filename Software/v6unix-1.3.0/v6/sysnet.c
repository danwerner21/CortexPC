/*#include <sys/socket.h>*/
#include <sys/ioctl.h>

#include "param.h"
#include "systm.h"
#include "user.h"
#include "proc.h"
#include "reg.h"
#include "file.h"

#include "sysnet.h"

/* Network integration code */

struct netrequest nrequestq[NNETREQ];
struct netrequest *nfreeq;
struct netrequest *nworkq;
int nwrkcnt;
int nwrkget;
int nwrkput;
static int hasnet;

void
netinit()
{
	struct netrequest *np;

	np = nrequestq;
	nfreeq = NULL;
	nwrkcnt = 0;
	nwrkget = 0;
	nwrkput = 0;
	for (; np <= &nrequestq[NNETREQ-1]; np++) {
		np->n_next = nfreeq;
		nfreeq = np;
	}
}

static struct netrequest *
netqget()
{
	struct netrequest *np;

	while ((np = nfreeq) == NULL) {
		sleep (&nrequestq, PRINET);
	}
	nfreeq = np->n_next;
	np->n_next = NULL;
	nwrkcnt++;
	nwrkget++;
	return np;
}

static void
netqput(pp)
	struct netrequest *pp;
{
	struct netrequest *cp;

	cp = nfreeq;
	pp->n_next = cp;
	nfreeq = pp;
	nwrkcnt--;
	nwrkput++;
	if (cp == NULL) {
		wakeup (&nrequestq);
	}
}

/*
 * sysnet() is a syscall to integrate the network stack with the kernel
 */

void
sysnet()
{
        register struct proc *pp;
#ifdef TI_CTX
	register int i, *t;
#else
	struct netrequest *wp;
#endif
        
        /* only accept this syscall from the network process */
        pp = u.u_procp;
        if(!suser() /*|| pp->p_pid!=2*/) {
                u.u_error = 100;
                return;
        }
        
        u.u_ar0[R0] = 0;
        switch (u.u_arg[0]) {

        case SYSINIT: /* init (exchange thunking pointers) */
#ifdef TI_CTX
                t = (int *)u.u_arg[1];
                for(i=0; i<5;)
                        netthunk[i++] = fuword(t++);
                t = (int *)u.u_arg[2];        
                suword(t,   ((int*) &tsleep));
                suword(t+1, ((int*) &twakeup));
                pp->p_ttyp  = NULL;     /* no controlling tty */
                pp->p_flag |= SSYS;     /* make system process */
#endif
                hasnet = 1;
                break;

        case SYSSLEEP: /* call kernel sleep */
                sleep (u.u_arg[1], u.u_arg[2]);
                break;

        case SYSWAKEUP: /* call kernel wakeup */
                wakeup (u.u_arg[1]);
		break;

#ifndef TI_CTX
	case SYSREQUEST: /* net request */
		wp = nworkq;
		if (wp != NULL) {
			nworkq = wp->n_next;
			copyout (u.u_arg[1], (char *)wp, u.u_arg[2]);
			netqput (wp);
		}
                u.u_ar0[R0] = (int)wp;
		break;

	case SYSCOPYIN: /* net copyin */
		netcopybuf (u.u_arg[1], pp->p_slot, u.u_arg[2], u.u_arg[3],
			u.u_arg[4]);
		break;

	case SYSCOPYOUT: /* net copyout */
		netcopybuf (u.u_arg[1], u.u_arg[2], u.u_arg[3], pp->p_slot,
			u.u_arg[4]);
		break;

	case SYSREPLY: /* net reply */
		pp = (struct proc *)u.u_arg[2];
		pp->p_nerror = u.u_arg[3];
		pp->p_nretval = u.u_arg[4];
		wakeup (u.u_arg[1]);
		break;
#endif
        }
}

/* net device support */

int
netread(dev)
	int dev;
{
}

int
netwrite(dev)
	int dev;
{
}

int
netcontrol(dev)
	int dev;
{
}

/* user net suppport */

static struct netrequest *
initnreq (op, fd, fp)
int op;
int fd;
struct file *fp;
{
	register struct netrequest *pp;
        register struct proc *pr;

	if ((pp = netqget ()) != NULL) {
        	pr = u.u_procp;
		pr->p_flag |= SNET;
		pr->p_nerror = 0;
		pr->p_nretval = 0;
		pp->n_request = op;
		pp->n_proc = (int)pr;
		pp->n_pid = (int)pr->p_pid;
		pp->n_ucb = (struct ucb *)0;
		pp->n_tcb = (struct tcb *)0;
		pp->n_usrrequest = 0;
		pp->n_retval = 0;
		pp->n_usrfd = fd;
		pp->n_file = fp;
		pp->n_slot = u.u_procp->p_slot;
		pp->n_buffer = 0;
		pp->n_len = 0; 
		pp->n_count = 0; 
		pp->n_eol = 0; 
		pp->n_datawait = 0; 
	}
	return pp;
}

static void
quenreq (pp)
register struct netrequest *pp;
{
	pp->n_next = nworkq;
	nworkq = pp;
	wakeup (&netmain);
	sleep (pp->n_pid, PUSER);
}

void
netopen ()
{
	register struct netrequest *pp;
	struct file *fp;

	if (!hasnet) {
		u.u_error = ENETDOWN;
		return;
	}

	if ((fp = falloc()) == NULL)
		return;
	fp->f_flag = (FNET | FREAD | FWRITE);

	if ((pp = initnreq (SYSOPEN, u.u_ar0[R0], fp)) == NULL) {
		u.u_error = EBUSY;
		return;
	}
	pp->n_buffer = (char *)u.u_arg[0];

	quenreq (pp);
}

void
closen (fp, fd)
	struct file *fp;
	int fd;
{
	register struct netrequest *pp;

	if (!hasnet) {
		u.u_error = ENETDOWN;
		return;
	}

	if ((pp = initnreq (SYSCLOSE, fd, fp)) == NULL) {
		u.u_error = EBUSY;
		return;
	}

	quenreq (pp);
}

void
readn(fp)
	struct file *fp;
{
	register struct netrequest *pp;

	if (!hasnet) {
		u.u_error = ENETDOWN;
		return;
	}

	if ((pp = initnreq (SYSREAD, u.u_arg[0], fp)) == NULL) {
		u.u_error = EBUSY;
		return;
	}
	pp->n_buffer = (char *)u.u_arg[1];
	pp->n_len = u.u_arg[2];

	quenreq (pp);
}

void
writen(fp)
	struct file *fp;
{
	register struct netrequest *pp;

	if (!hasnet) {
		u.u_error = ENETDOWN;
		return;
	}

	if ((pp = initnreq (SYSWRITE, u.u_arg[0], fp)) == NULL) {
		u.u_error = EBUSY;
		return;
	}
	pp->n_buffer = (char *)u.u_arg[1];
	pp->n_len = u.u_arg[2];

	quenreq (pp);
}

/*
 * ioctl system call
 * Check legality, execute common code, and switch out to individual
 * device routine.
 */
void
sioctl()
{
	register struct file *fp;
        register char *ucbp;

	if (!hasnet) {
		u.u_error = ENETDOWN;
		return;
	}

	if ((fp = getf(u.u_arg[0])) == NULL)
		return;

#ifdef TI_CTX
        if (fp->f_flag & FNET) {
                ucbp = (char*)(fp->f_offset);
                netioctl(ucbp, u.u_procp->p_slot);
                u.u_ar0[R0] = 0;
	}
#else
	{
		register struct netrequest *pp;

		if ((pp = initnreq (SYSIOCTL, u.u_arg[0], fp)) == NULL) {
			u.u_error = EBUSY;
			return;
		}
		pp->n_usrrequest = u.u_arg[1];
		pp->n_buffer = (char *)u.u_arg[2];
		pp->n_len = u.u_arg[3];

		quenreq (pp);
	}
#endif
}
