#define DEBUG
#include <sys/types.h>
#include <sys/ioctl.h>

#include "param.h"
#include "user.h"
#include "proc.h"
#include "sysnet.h"

#include "mbuf.h"
#include "net.h"
#include "ifcb.h"
#include "ip.h"
#include "tcp.h"
#include "raw.h"
#include "udp.h"
#include "ucb.h"
#include "fsm.h"

#define f_ucb f_offset

extern struct gway gates[NGATE], *gateway, *gateNGATE;
extern int ngate;

struct nfile {
	int n_flags;
	struct ucb *n_ucb;
};

static struct nfile nfiles[CMAPSIZ][10];
static struct netrequest requests[CMAPSIZ];

/*
 * Generate default local port number, if necessary and check if specified
 * connection is unique.
 */
u_short
uniqport(we, up, lport, fport)
struct netrequest *we;
register struct ucb *up;
register u_short lport, fport;
{
	register struct ucb *q;
	register i;
	int deflt = FALSE;

#ifdef DEBUGUNIQ
	printf ("uniqport: lport = %d, fport = %d, up = 0x%x\n",
		lport, fport, up);
#endif
	/* if local port is zero, make up a default */

	if (lport == 0) {
		for (q = netcb.n_ucb_hd, i = 0; q != NULL; q = q->uc_next, i++);
		lport = (i << 8) | ((int)up->uc_proc & 0xff);
#ifdef DEBUGUNIQ
		printf ("   lport-1 = %d, i = %d, pid = %d\n",
			lport, i, up->uc_proc);
#endif
		deflt++;
	}

	/* make sure port number is unique, for defaults, just increment
	   original choice until it is unique. */

tryagain:
	for (q = netcb.n_ucb_hd; q != NULL; q = q->uc_next) {
#ifdef DEBUGUNIQ
		printf ("   q = 0x%x\n", q);
#endif
		if (q != up && q->uc_host.s_addr == up->uc_host.s_addr &&
		    q->uc_local.s_addr == up->uc_local.s_addr &&
		    ((q->uc_flags & UTCP && q->uc_tcb != NULL &&
		      q->uc_tcb->t_lport == lport &&
		      q->uc_tcb->t_fport == fport) ||
		     (q->uc_flags & UUDP &&
		      q->uc_udp.u_lport == lport &&
		      q->uc_udp.u_fport == fport))) {
#ifdef DEBUGUNIQ
			printf ("   lport-2 = %d\n", lport);
#endif
			if (deflt)  {
				if (++lport < 0x100)
					lport = 0x100;
				goto tryagain;
			}
			break;
		}
	}

	/* if connection is unique, return local port, otherwise set error */

#ifdef DEBUGUNIQ
	printf ("   q = 0x%x, lport = %d\n", q, lport);
#endif
	if (q != NULL) {
		we->n_error = ENETRNG;
		return(0);
	}
	return(lport);
}

/*
 * TCP specific open routine.  Allocate a TCB and set up an open work request
 * for the finite state machine.
 */
int
tcpopen(we, up, cp)
struct netrequest *we;
register struct ucb *up;
register struct con *cp;
{
	register struct mbuf *m;
	register struct tcb *tp;
	register char *p;
	register i;

	/* allocate a buffer for a tcb */

	if ((m = m_get(1)) == NULL) {           /* no buffers */
		we->n_error = ENETBUF;
		return 0;
	}
	m->m_off = MHEAD;
	tp = mtod(m, struct tcb *);		/* -> tcb */
	we->n_tcb = tp;

	/* zero tcb for initialization */

	for (p = (char *)tp, i = MLEN; i > 0; i--)
		*p++ = 0;

	/* fill in ucb and tcb fields */

	tp->t_ucb = up;
	up->uc_tcb = tp;
	up->uc_flags |= UTCP;
	tp->t_fport = cp->c_fport;
	tp->t_lport = uniqport(we, up, cp->c_lport, cp->c_fport);

	/* if local port not unique, free tcb */

	if (we->n_error) {
		up->uc_flags &= ~UTCP;
		up->uc_tcb = NULL;
		m_free(m);
		return 0;
	}

	/*  make a user open work request for tcp */

	w_alloc((cp->c_mode & CONACT ? IUOPENR : IUOPENA), 0, tp, NULL, we);
	return 1;
}

/*
 * TCP read routine.  Copy data from the receive queue to user space.
 */
int
tcp_read(we, up, reent)
struct netrequest *we;
register struct ucb *up;
int reent;
{
	register struct tcb *tp;

	if ((tp = up->uc_tcb) == NULL) {
		we->n_error = ENETERR;
		return 0;
	}
	we->n_tcb = tp;

	if (!tp->syn_acked)		/* block if not connected */
		return 1;

	if (!reent) {
		we->n_datawait = FALSE;
		we->n_eol = FALSE;
		we->n_count = we->n_len;
	}

	if (we->n_datawait) {
	   if (up->uc_rbuf == NULL) {	/* no data available */
	      if (up->uc_state == 0) {
		 if (tp->fin_rcvd && rcv_empty(tp)) {
		    we->n_retval = we->n_count - we->n_len;
		    return 0;
		 }
	      }
	      return 1;
	   }
	   we->n_datawait = FALSE;
	}

#ifdef DEBUG
	printf ("tcp_read: n_slot = %d, n_len = %d(%d), buffer = 0x%x\n",
		we->n_slot, we->n_count, we->n_len, we->n_buffer);
#endif
	while (we->n_len > 0 && !we->n_eol) {

	        if (up->uc_rbuf != NULL) {	/* data available */
			register struct mbuf *m;
			int len;

			/* lock recieve buffer */

                        if (up->uc_flags & ULOCK) {
#ifdef DEBUG
				printf ("   ULOCKed\n");
#endif
                        	return 1;
			}
                        up->uc_flags |= ULOCK;

			/* copy the data an mbuf at a time */

			m = up->uc_rbuf;
                        while (m != NULL && we->n_len > 0 && !we->n_eol) {
#ifdef DEBUGDATA
				int ii, jj;
				unsigned char *jp;
#endif

                        	len = MIN(m->m_len, we->n_len);
#ifdef DEBUG
				printf ("   copyout: len = %d\n", len);
#ifdef DEBUGDATA
				jp = (unsigned char *)((int)m + m->m_off);
				for (ii = jj = 0; jj < len; ii++, jj++) {
				   printf (" 0x%x", *jp++ );
				   if (ii == 10) {printf("\n"); ii = 0;}
				}
				if (ii) printf ("\n");
#endif
#endif
                        	sysnet (SYSCOPYOUT, we->n_buffer, we->n_slot,
					(caddr_t)((int)m + m->m_off), len);
				we->n_buffer += len;
				we->n_len -= len;

				/* free or adjust mbuf */

                        	if (len == m->m_len) {
        			        we->n_eol = (int)m->m_act;
                        		m = m_free(m);
                        		--up->uc_rsize;
                        	} else {
                        		m->m_off += len;
                        		m->m_len -= len;
                        	}
                        }
			up->uc_rbuf = m;
			up->uc_flags &= ~ULOCK; /* unlock buffer */

			/*
			 * If any data was read, return even if the
			 * full read request was not satisfied
			 */
			if (we->n_count != we->n_len) {
				we->n_retval = we->n_count - we->n_len;
#ifdef DEBUG
				printf ("   len-1 = %d\n", we->n_retval);
#endif
				return 0;
			}
		} else if (up->uc_state == 0) {
			/*
			 * If no data left and foreign FIN rcvd, just
			 * return, otherwise make work request and wait
			 * for more data
			 */
			if (tp->fin_rcvd && rcv_empty(tp))
				break;

#ifdef DEBUG
			printf ("   w_alloc: len = %d\n", we->n_len);
#endif
        		w_alloc(IURECV, 0, tp, NULL, we);
			we->n_datawait = TRUE;
			return 1;
		} else {
			/*
			 * Net status to return.
			 */
			we->n_error = ENETSTAT;
			break;
		}
	}
	we->n_retval = we->n_count - we->n_len;
#ifdef DEBUG
	printf ("   len-2 = %d\n", we->n_retval);
#endif
	return 0;
}

/*
 * TCP write routine.  Copy data from user space to a net buffer chain.  Put
 * this chain on a send work request for the tcp fsm.
 */
int
tcp_write(we, up, reent)
struct netrequest *we;
register struct ucb *up;
int reent;
{
	register struct tcb *tp;

	if ((tp = up->uc_tcb) == NULL) {
		we->n_error = ENETERR;
		return 0;
	}
	we->n_tcb = tp;

	if (!tp->syn_acked)		/* block if not connected */
		return 1;

	if (!reent) {
		we->n_datawait = FALSE;
		we->n_count = we->n_len;
	}
#ifdef DEBUG
	printf ("tcp_write: n_slot = %d, n_len = %d(%d)\n",
		we->n_slot, we->n_count, we->n_len);
#endif

	while (we->n_len > 0 && up->uc_state == 0) {
		register struct mbuf *n;
		register int bufs;
		struct {
			struct mbuf *top;
		} nn;

		/* #bufs can send now */

		bufs = (int)up->uc_snd - (int)up->uc_ssize;
#ifdef DEBUG
		printf ("   bufs = %d\n", bufs);
#endif

		/* top of chain to send */

		nn.top = NULL;
		n = (struct mbuf *)&nn;

		/* copy data from user to mbuf chain for send work req */

        	while (bufs-- > 0 && we->n_len > 0) {
			register struct mbuf *m;
			register int len;
#ifdef DEBUGDATA
			int ii, jj;
			unsigned char *jp;
#endif

			/* set up an mbuf */

			we->n_datawait = FALSE;
        		if ((m = m_get(1)) == NULL) {
				/* send what we have, make work req */
				if (nn.top)
        				w_alloc(IUSEND, 0, tp, nn.top, we);
#ifdef DEBUG
				printf ("   w_alloc-1: len = %d\n", we->n_len);
#endif
				return 1;
			}
        		m->m_off = MHEAD;

			/* copy user data in */

        		len = MIN(MLEN, we->n_len);
        		sysnet (SYSCOPYIN, (caddr_t)((int)m + m->m_off),
				we->n_buffer, we->n_slot, len);
#ifdef DEBUG
				printf ("   copyin: len = %d\n", len);
#ifdef DEBUGDATA
				jp = (unsigned char *)((int)m + m->m_off);
				for (ii = jj = 0; jj < len; ii++, jj++) {
				   printf (" 0x%x", *jp++ );
				   if (ii == 10) {printf("\n"); ii = 0;}
				}
				if (ii) printf ("\n");
#endif
#endif
        		we->n_buffer += len;
			we->n_len -= len;

			/* chain to work request mbufs */

        		m->m_len = len;
        		n->m_next = m;
        		n = m;
        	}

		if (!we->n_datawait) {
		   w_alloc(IUSEND, 0, tp, nn.top, we);         /* make work req */
#ifdef DEBUG
		   printf ("   w_alloc-2: len = %d, bufs = %d\n",
				we->n_len, bufs + 1);
#endif
		}

		/* if more to send, sleep on more buffers */

		if (we->n_len > 0) {
		        we->n_datawait = TRUE;
			return 1;
		}
	}
	we->n_retval = we->n_count;
	return 0;
}

int
tcp_close (we, up, reent)
struct netrequest *we;
register struct ucb *up;
int reent;
{
	register struct tcb *tp;

	if (up->uc_flags & UTCP) {      /* tcp connection */

		tp = up->uc_tcb;

		/* if connection has not already been closed, by user or
		   system, enqueue work request.  otherwise, if the tcb
		   has been deleted (completed close), then release the ucb */

		if (tp != NULL) {	/* tcb still exists */
			tp->usr_abort = TRUE;	/* says we've closed */
			if (!tp->usr_closed) {
				w_alloc(IUCLOSE, 0, tp, up, we);
				return 1;
			}
		}

	} else if (up->uc_flags&(UIP+URAW+UUDP)) {        /* raw connection */

	/* free ucb and lower system buffer allocation */

        	netcb.n_lowat -= up->uc_snd + up->uc_rcv + 2;
		netcb.n_hiwat = (netcb.n_lowat * 3) >> 1;
		if (up->uc_route != NULL) {
        		h_free(up->uc_route);
			up->uc_route = NULL;
		}
#ifdef SUPPORTRAW
		if (!(up->uc_flags & UUDP))
			raw_free(up);
#endif
	}
	up->uc_proc = NULL;
	if (up->uc_next != NULL)
		up->uc_next->uc_prev = up->uc_prev;
	if (up->uc_prev != NULL)
		up->uc_prev->uc_next = up->uc_next;
	else
		netcb.n_ucb_hd = up->uc_next;
	m_free(dtom(up));
	return 0;
}

#ifdef SUPPORTRAW
/*
 * Raw IP and local net read routine.  Copy data from raw read queue to user
 * space.
 */
int
raw_read(we, up)
struct netrequest *we;
register struct ucb *up;
{
	register struct mbuf *m, *next;
	register len;
	int incount;

#ifdef DEBUG
	printf ("raw_read: entered\n");
#endif
	incount = we->n_len;
	while (up->uc_rbuf == NULL && up->uc_state == 0)
		return 1;

	/* lock recieve buffer */

        while (up->uc_flags & ULOCK)
        	return 1;

        up->uc_flags |= ULOCK;

	/* copy the data an mbuf at a time */

        if ((m = up->uc_rbuf) != NULL)
		next = m->m_act;
	else
		next = NULL;

        while (m != NULL && we->n_len > 0) {

        	len = MIN(m->m_len, we->n_len);
        	sysnet (SYSCOPYOUT, we->n_buffer, we->n_slot,
			(caddr_t)((int)m + m->m_off), len);
        	we->n_len -= len;
		we->n_buffer += len;

		/* free or adjust mbuf */

		if (len == m->m_len) {
			m = m_free (m);
			--up->uc_rsize;
		} else {
        		m->m_off += len;
        		m->m_len -= len;
		}
        }

	if (m == NULL)
		up->uc_rbuf = next;
	else {
		m->m_act = next;
		up->uc_rbuf = m;
	}
	up->uc_flags &= ~ULOCK; /* unlock buffer */
	return 0;
}

/*
 * Raw IP or local net write routine.  Copy data from user space to a network
 * buffer chain.  Call the appropriate format routine to set up a packet
 * header and send the data.
 */
void
raw_write(we, up)
struct netrequest *we;
register struct ucb *up;
{
	register struct mbuf *m, *n, *top;
	register len, bufs, ulen;

	/* get header mbuf */

#ifdef DEBUG
	printf ("raw_write: entered\n");
#endif
	if ((top = n = m_get(1)) == NULL) {
		we->n_error = ENETBUF;
		return;
	}

	bufs = up->uc_snd;                      /* #bufs can send now */
	ulen = we->n_len;

	/* copy data from user to mbuf chain for send */

        while (bufs-- > 0 && we->n_len > 0) {

		/* set up an mbuf */

        	if ((m = m_get(1)) == NULL) {
			we->n_error = ENETBUF;
			return;
		}
        	m->m_off = MHEAD;

		/* copy user data in */

        	len = MIN(MLEN, we->n_len);
		sysnet (SYSCOPYIN, (caddr_t)((int)m + m->m_off), we->n_buffer,
			we->n_slot, len);

        	m->m_len = len;
        	n->m_next = m;
        	n = m;
		we->n_len -= len;
		we->n_buffer += len;
        }

	/* only send raw message if all of it could be read */

	if (we->n_len > 0) {
		we->n_error = ERAWBAD;
		m_freem(top);
		return;
	}

	/* send to net */

	switch (up->uc_flags & CONMASK) {

	case UUDP:
		udp_output(up, top, ulen);
		break;

	case UIP:
        	ip_raw(up, top, ulen);
		break;

	case URAW:
		(*up->uc_srcif->if_raw)(up, top, ulen);
		break;

	default:
		m_freem(top);
		we->n_error = ENETPARM;
	}
}
#endif

/*
 * Network user i/f routines
 */

/*
 * Open a network connection.  Allocate an open file structure and a network
 * user control block (ucb).  Verify user open parameters and do protocol
 * specific open functions.
 */
int
netopen(we)
struct netrequest *we;
{
	register struct ucb *up;
	register struct con *cp;
	register struct nfile *fp;
	register struct ifcb *lp;
	register struct mbuf *m;
	int snd, rcv, mask, i;
	int retval;
	struct con const;
	net_t net;

	retval = 0;
	/* get user connection structure parm */

#ifdef DEBUGOPEN
	printf ("netopen: n_slot = %d, n_usrfd = %d\n",
		we->n_slot, we->n_usrfd);
#endif
	sysnet (SYSCOPYIN, (caddr_t)&const, we->n_buffer,
		we->n_slot, sizeof(const));
	cp = (struct con *)&const;
	fp = &nfiles[we->n_slot][we->n_usrfd];

	/* calculate send and receive buffer allocations */

	snd = cp->c_sbufs << 3;
	rcv = cp->c_rbufs << 3;
	if (snd < 8 || snd > 64)
		snd = 8;
	if (rcv < 8 || rcv > 64)
		rcv = 8;

#ifdef DEBUGOPEN
	printf ("   lport = %d, fport = %d\n", cp->c_lport, cp->c_fport);
	printf ("   snd = %d, rcv = %d\n", snd, rcv);
#endif
	/* make sure system has enough buffers for connection */

	if (!(cp->c_mode & CONCTL) &&
	    (snd + rcv + 2) > (NMBUF-16-netcb.n_lowat)) {
		we->n_error = ENETBUF;
		return retval;
	}

	/* allocate ucb */

	if ((m = m_get(1)) == NULL) {
		we->n_error = ENETBUF;
		return retval;
	}
	m->m_off = MHEAD;
	up = mtod(m, struct ucb *);
	we->n_ucb = up;
	up->uc_next = netcb.n_ucb_hd;
	up->uc_prev = NULL;
	if (netcb.n_ucb_hd != NULL)
		netcb.n_ucb_hd->uc_prev = up;
	netcb.n_ucb_hd = up;

	/* init common ucb fields */

	up->uc_proc = (struct proc *)we->n_pid;
	up->uc_proto = NULL;
	up->uc_snd = snd;
	up->uc_rcv = rcv;
	up->uc_sbuf = NULL;
	up->uc_rbuf = NULL;
	up->uc_timeo = cp->c_timeo;             /* overlays uc_ssize */
	up->uc_rsize = 0;
	up->uc_state = 0;
	up->uc_xstat = 0;
	up->uc_flags = cp->c_mode & ~(UTCP+ULOCK);

	fp->n_flags = 0;
	fp->n_ucb = up;

	/* control connections are all done here */

	if (up->uc_flags&UCTL)
		return retval;

	up->uc_flags ^= CONACT;
	netcb.n_lowat += snd + rcv + 2;
	netcb.n_hiwat = (netcb.n_lowat * 3) >> 1;

	/* save foreign host address, route will be filled in later */

	up->uc_route = NULL;
	up->uc_host = cp->c_fcon;

	/* if local address is specified, make sure its one of ours and use
	   it if it is, otherwise pick the "best" local address from the
	   local i/f table */

	if (cp->c_lcon.s_addr != 0L) {
		up->uc_local = cp->c_lcon;
		for (lp = netcb.n_ifcb_hd; lp != NULL; lp = lp->if_next)
			if (lp->if_addr.s_addr == cp->c_lcon.s_addr)
				goto gotaddr;
		we->n_error = ENETRNG;	/* spec local address not in table */
		goto conerr;
gotaddr:
		/* make sure specified i/f is available */

		if (lp->if_avail)
			up->uc_srcif = lp;
		else {
			we->n_error = ENETDOWN;
			goto conerr;
		}
	} else {

		/* find default 'preferred' address, look for first
		   available i/f */

		up->uc_srcif = NULL;
		for (lp = netcb.n_ifcb_hd; lp != NULL; lp = lp->if_next) {
			if (lp->if_avail) {
				up->uc_srcif = lp;
				net = iptonet(cp->c_fcon);
				break;
			}
		}

		/* error if none available */

		if (up->uc_srcif == NULL) {
			we->n_error = ENETDOWN;
			goto conerr;
		}

		/* if foreign host is on a net that we are, use the address
		   of the i/f on that net */

		for (lp = netcb.n_ifcb_hd; lp != NULL; lp = lp->if_next)
			if (iptonet(lp->if_addr)==net && lp->if_avail) {
				up->uc_srcif = lp;
				break;
			}

		/* use default local addr for active opens, wild card for
		   passive. */

		if (cp->c_mode & CONACT)
			up->uc_local = up->uc_srcif->if_addr;
		else
			up->uc_local.s_addr = NULL;
	}

	/* call protocol open routine for rest of init */

#ifdef DEBUGOPEN
	printf ("   call proto: mode = 0x%x\n", cp->c_mode);
#endif
	switch (cp->c_mode & CONMASK) {

	case CONTCP:
		retval = tcpopen(we, up, cp);
		if (we->n_error)
			goto conerr;
		break;

#ifdef SUPPORTRAW
	case CONUDP:
		up->uc_udp.u_fport = cp->c_fport;
		up->uc_udp.u_lport = uniqport(we, up, cp->c_lport, cp->c_fport);
		if (we->n_error)
			goto conerr;
		break;

	case CONRAW:
	case CONIP:
		raw_add(up, cp->c_proto);
		if (we->n_error)
			goto conerr;
		break;
#endif

       	default:
		we->n_error = ENETPARM;
	conerr:
		netclose(we);
	}
	return retval;
}

/*
 * Network close routine.  For TCP connections, issue a close work request to
 * the tcp fsm.  Otherwise, just clean up the ucb and the file structure
 */
int
netclose(we)
struct netrequest *we;
{
	register struct file *fp;
	register struct ucb *up;

#ifdef DEBUGCLOSE
	printf ("netclose: n_slot = %d, n_usrfd = %d\n",
		we->n_slot, we->n_usrfd);
#endif
	up = nfiles[we->n_slot][we->n_usrfd].n_ucb;
	we->n_ucb = up;

	return tcp_close (we, up, 0);
}

/*
 * Network read routine. Just call protocol specific read routine.
 */
int
netread(we)
struct netrequest *we;
{
	register struct ucb *up;
	int retval;

#ifdef DEBUG
	printf ("netread: n_slot = %d, n_usrfd = %d\n",
		we->n_slot, we->n_usrfd);
#endif
	retval = 0;
	up = nfiles[we->n_slot][we->n_usrfd].n_ucb;
	we->n_ucb = up;

	switch (up->uc_flags & CONMASK) {

	case UTCP:
		retval = tcp_read(we, up, 0);
		break;

#ifdef SUPPORTRAW
	case UIP:
	case URAW:
	case UUDP:
		raw_read(we, up);
		break;
#endif
	default:
		we->n_error = ENETPARM;
	}
	return retval;
}

/*
 * Network write routine. Just call protocol specific write routine.
 */
int
netwrite(we)
struct netrequest *we;
{
	register struct ucb *up;
	int retval;

#ifdef DEBUG
	printf ("netwrite: n_slot = %d, n_usrfd = %d\n",
		we->n_slot, we->n_usrfd);
#endif
	retval = 0;
	up = nfiles[we->n_slot][we->n_usrfd].n_ucb;
	we->n_ucb = up;

	if (up->uc_state)
		we->n_error = ENETSTAT;		/* new status to report */
	else {
		switch (up->uc_flags & CONMASK) {

		case UTCP:
			retval = tcp_write(we, up, 0);
			break;
#ifdef SUPPORTRAW
		case UIP:
		case URAW:
		case UUDP:
			raw_write(we, up);
			break;
#endif
		default:
			we->n_error = ENETPARM;
		}
	}
	return retval;
}

/*
 * Network control functions
 */
int
netioctl(we)
	struct netrequest *we;
{
#if 0
	register struct inode *ip;
	register struct mbuf *m;
	char *p, *q;
	struct host *route;
	short proto;
	int len;
	struct socket addr;
	struct netopt opt;
	struct netinit init;
	caddr_t cmdp;
#endif
	register struct tcb *tp;
	register struct ucb *up;
	register struct gway *gp;
	register struct ifcb *lp;
	struct netstate nstat;
	int cnt;
	char *bp;
	int cmd;

	up = nfiles[we->n_slot][we->n_usrfd].n_ucb;
	tp = up->uc_tcb;

	cmd = we->n_usrrequest;
#ifdef DEBUGIOC
	printf ("netioctl: cmd = %d, usrfd = %d\n", cmd, we->n_usrfd);
#endif
	switch (cmd) {

	case NETGETS:                   /* get net status */
		memcpy((caddr_t)&nstat, (caddr_t)&up->uc_snd,
			sizeof(struct netstate));
		if (up->uc_flags & UTCP && tp != NULL) {
			nstat.n_lport = tp->t_lport;
			nstat.n_fport = tp->t_fport;
		} else if (up->uc_flags & UUDP) {
			nstat.n_lport = up->uc_udp.u_lport;
			nstat.n_fport = up->uc_udp.u_fport;
		}
		else {
			nstat.n_lport = 0;
			nstat.n_fport = 0;
		}

		nstat.n_fcon = up->uc_host;
		nstat.n_lcon = up->uc_local;
#ifdef DEBUGIOC
		if (up->uc_rsize)
			printf ("netioctl: usrfd = %d, rsize = %d\n",
				we->n_usrfd, up->uc_rsize);
#endif

		sysnet (SYSCOPYOUT, we->n_buffer, we->n_slot,
			(caddr_t)&nstat, sizeof(struct netstate));

		up->uc_xstat = 0;
		up->uc_state = 0;
		break;

	case NETSETU:                   /* set urgent mode */
		up->uc_flags |= UURG;
		break;

	case NETRSETU:                  /* reset urgent mode */
		up->uc_flags &= ~UURG;
		break;

	case NETSETE:                   /* set eol mode */
		up->uc_flags |= UEOL;
		break;

	case NETRSETE:                  /* reset eol mode */
		up->uc_flags &= ~UEOL;
		break;

	case NETCLOSE:                  /* tcp close but continue to rcv */
		if ((up->uc_flags & UTCP) && tp != NULL && !tp->usr_closed)
			w_alloc(IUCLOSE, 0, tp, up, we);
		break;

	case NETABORT:                  /* tcp user abort */
		if ((up->uc_flags & UTCP) && tp != NULL)
			w_alloc(IUABORT, 0, tp, up, we);
		break;

	case NETOWAIT:			/* tcp wait until connected */
		if (up->uc_flags&UTCP && tp != NULL && !tp->syn_acked)
			return 1;
		break;

#if 0
	case NETSETD:                   /* set debugging file */
		if (!suser()) {
			we->n_error = EPERM;
			break;
		}
		if (cmdp == NULL) {
			if (netcb.n_debug) {
				plock(netcb.n_debug);
				iput(netcb.n_debug);
				netcb.n_debug = NULL;
			}
		} else {
			if (netcb.n_debug != NULL) {
				we->n_error = EBUSY;
				break;
			}
			u.u_dirp = cmdp;
			if ((ip = namei(uchar, 0)) != NULL) {
				if ((ip->i_mode & IFMT) != IFREG) {
					we->n_error = EACCES;
					iput(ip);
				} else {
					netcb.n_debug = ip;
					prele(ip);
				}
			}
		}
		break;

	case NETRESET:                  /* forced tcp reset */
		if (!suser()) {
			we->n_error = EPERM;
			break;
		}
		for (up = netcb.n_ucb_hd; up != NULL; up = up->uc_next)
			if ((up->uc_flags & UTCP) &&
			     up->uc_tcb != NULL &&
			     up->uc_tcb == (struct tcb *)cmdp) {
				w_alloc(INCLEAR, 0, cmdp, NULL);
				return 0;
			}
		we->n_error = EINVAL;
		break;

	case NETDEBUG:                  /* toggle debugging flag */
		for (up = netcb.n_ucb_hd; up != NULL; up = up->uc_next)
			if ((up->uc_flags & UTCP) && up->uc_tcb != NULL &&
			    up->uc_tcb == (struct tcb *)cmdp) {
				up->uc_flags ^= UDEBUG;
				return 0;
			}
		we->n_error = EINVAL;
		break;

	case NETTCPOPT:			/* copy ip options to tcb */
		if ((up->uc_flags & UTCP) && tp != NULL && cmdp != NULL) {

			if (copyin((caddr_t)cmdp, &opt, sizeof opt)) {
				we->n_error = EFAULT;
				break;
			}

			/* maximum option string is 40 bytes */

			if (opt.nio_len > IPMAXOPT) {
				we->n_error = EINVAL;
				break;
			}

			/* get a buffer to hold the option string */

			if ((m = m_get(1)) == NULL) {
				we->n_error = ENETBUF;
				break;
			}
			m->m_off = MHEAD;
			m->m_len = opt.nio_len;

			if (copyin((caddr_t)opt.nio_opt,
				(caddr_t)((int)m + m->m_off), opt.nio_len)) {
				m_free(m);
				we->n_error = EFAULT;
				break;
			}
			tp->t_opts = mtod(m, char *);
			tp->t_optlen = opt.nio_len;
		}
		break;

	case NETGINIT:			/* reread the gateway file */
		if (!suser()) {
			we->n_error = EPERM;
			break;
		}
		if (cmdp == NULL)	/* reset the table */
			gateNGATE = gateway;
		else {
			gp = gateNGATE;
			/*
			 * too many entries
			 */
			if (ngate <= gp - gateway) {
				we->n_error = ETABLE;
				break;
			}
			if (copyin((caddr_t)cmdp, (caddr_t)gp,
				   sizeof(struct gway))) {
				we->n_error = EFAULT;
				break;
			}
			/*
			 * can we use this entry?
			 */
			if (gp->g_flags&GWFORMAT) {
				for (lp=netcb.n_ifcb_hd;
						  lp != NULL; lp=lp->if_next)
					if (iptonet(lp->if_addr) ==
					    gp->g_lnet) {
						gp->g_ifcb = lp;
						gateNGATE = ++gp;
						return 0;
					}
			}
			we->n_error = EINVAL;
		}
		break;

	case NETPRADD:			/* add protocol numbers to ucb chain */
	case NETPRDEL:			/* delete protocol numbers from chain */
		/* get arg pointer and arg length and verify */

		if (copyin((caddr_t)cmdp, &opt, sizeof opt)) {
			we->n_error = EFAULT;
			break;
		}

		/* add or delete each proto number on ucb chain */

		for (len=opt.nio_len; len > 0; len -= sizeof(short)) {

			/* get next proto number from user */

			if (copyin((caddr_t)opt.nio_opt, proto, sizeof(short))){
				we->n_error = EFAULT;
				break;
			}
			if (cmd == NETPRADD)
				raw_add(up, proto);
			else
				raw_del(up, proto);
			opt.nio_opt += sizeof(short);
		}
		break;

	case NETPRSTAT:			/* return current list of proto nos. */
		/* get arg pointer and arg length */

		if (copyin((caddr_t)cmdp, &opt, sizeof opt)) {
			we->n_error = EFAULT;
			break;
		}
		raw_stat(up, opt.nio_opt, opt.nio_len);
		break;

	case NETROUTE:			/* change ip route */
		route = up->uc_route;
		if (cmdp == NULL)
			up->uc_route = NULL;
		else {
			if (copyin((caddr_t)cmdp, &addr, sizeof addr)) {
				we->n_error = EFAULT;
				break;
			}

			/* substitute new route */

			up->uc_route = h_make(&addr, FALSE);
		}

		/* free old entry */

		if (route != NULL)
			h_free(route);
		break;

	case NETINIT:			/* init net i/f */
	case NETDISAB:			/* disable net i/f */
		/*
		 * Must be super-user!
		 */
		if (!suser()) {
			we->n_error = EPERM;
			break;
		}
		/*
		 * Copy and verify arguments.
		 */
		if (copyin((caddr_t)cmdp, &init, sizeof init)) {
			we->n_error = EFAULT;
			break;
		}
		/*
		 * Look for ifcb to init.
		 */
		for (lp = netcb.n_ifcb_hd; lp != NULL; lp = lp->if_next) {
			p = init.ni_name;
			q = lp->if_name;
			if (init.ni_unit == lp->if_unit) {
				len = 0;
				while (*p == *q && *q && len++ < NIMAX) {
					p++; q++;
				}
				if (len == NIMAX || *p == *q) {
				/*
				 * For init copy address and call dev init
				 * routine directly.  For disable, set
				 * flag for driver, and call netreset to
				 * clear queues and call dev init routine.
				 */
					if (cmd == NETINIT && !lp->if_avail) {
						lp->if_disab = FALSE;
						lp->if_addr = init.ni_addr;
						(*lp->if_init)(lp);
					} else if (cmd == NETDISAB) {
						lp->if_disab = TRUE;
						netreset(lp);
						/*
						 * Tell users on this i/f
						 */
						for (up=netcb.n_ucb_hd;
						     up != NULL;
						     up = up->uc_next)
							if (up->uc_srcif == lp)
								to_user(up,
									UABORT);
					}
					return 0;
				}
			}
		}
		we->n_error = EINVAL;
		break;
#endif
	case NETALLSTATS:
		{
		   struct getallstats allstats;

		   cnt = 0;
		   for (up = netcb.n_ucb_hd; up != NULL; up = up->uc_next)
		   {
		      struct ifucb *iup;

		      iup = &allstats.ucbs.uinfo[cnt];
		      memset ((char *)iup, 0, sizeof (struct ifucb));
		      if (up->uc_proc != 0)
		      {
			  iup->proc = (int)up->uc_proc;
			  iup->flags = up->uc_flags;
			  iup->state = up->uc_state;
			  iup->host = up->uc_host.s_addr;
			  iup->ssize = up->uc_ssize;
			  iup->rsize = up->uc_rsize;
			  iup->snd = up->uc_snd;
			  iup->rcv = up->uc_rcv;
			  if (up->uc_flags & UTCP) {
			     iup->fport = up->uc_tcb->t_fport;
			     iup->lport = up->uc_tcb->t_lport;
			     iup->tcb = (int)up->uc_tcb;
			     iup->tstate = up->uc_tcb->t_state;
			  } else {
			     iup->fport = up->uc_udp.u_fport;
			     iup->lport = up->uc_udp.u_lport;
			  }
			  cnt++;
		     }
		   }
		   allstats.ucbs.count = cnt;

		   cnt = 0;
		   for (gp = gateway; gp < gateNGATE; gp++) {
			struct ifroute *rp;

			rp = &allstats.routes.rinfo[cnt];
			rp->lnet = gp->g_lnet;
			rp->fnet = gp->g_fnet;
			rp->local = gp->g_local.s_addr;
			rp->flags = gp->g_flags;
			cnt++;
		   }
		   allstats.routes.count = cnt;

		   cnt = 0;
		   for (lp = netcb.n_ifcb_hd; lp != NULL; lp = lp->if_next) {
			struct ifstat lip;

			memcpy (lip.name, lp->if_name, 8);
			lip.avail = lp->if_avail;
			lip.error = lp->if_error;
			lip.needinit = lp->if_needinit;
			lip.active = lp->if_active;
			lip.flush = lp->if_flush;
			lip.blocked = lp->if_blocked;
			lip.disab = lp->if_disab;
			lip.mtu = lp->if_mtu;
			lip.addr = lp->if_addr.s_addr;
			lip.opkts = lp->if_opkts;
			lip.ipkts = lp->if_ipkts;
			lip.oerrs = lp->if_oerrs;
			lip.ierrs = lp->if_ierrs;
			memcpy ((char *)&allstats.ifstats.ifinfo[cnt],
				(char *)&lip, sizeof lip);
			cnt++;
		   }
		   allstats.ifstats.count = cnt;

		   allstats.ifmbufs.pages = netcb.n_pages;
		   allstats.ifmbufs.bufs = netcb.n_bufs;
		   allstats.ifmbufs.hiwat = netcb.n_hiwat;
		   allstats.ifmbufs.lowat = netcb.n_lowat;

		   allstats.nstats.m_drops = netstat.m_drops;
		   allstats.nstats.ip_badsum = netstat.ip_badsum;
		   allstats.nstats.t_badsum = netstat.t_badsum;
		   allstats.nstats.t_badsegs = netstat.t_badsegs;
		   allstats.nstats.t_unack = netstat.t_unack;
		   allstats.nstats.ic_drops = netstat.ic_drops;
		   allstats.nstats.ic_badsum = netstat.ic_badsum;
		   allstats.nstats.ic_quenches = netstat.ic_quenches;
		   allstats.nstats.ic_redirects = netstat.ic_redirects;
		   allstats.nstats.ic_echoes = netstat.ic_echoes;
		   allstats.nstats.ic_timex = netstat.ic_timex;

		   sysnet (SYSCOPYOUT, we->n_buffer, we->n_slot, &allstats,
			    sizeof allstats);
		}
		break;

	default:
		we->n_error = EINVAL;
	}
	return 0;
}

/*
 * Check for TCP connection is open or error.
 */
int
usr_reply()
{
	register struct netrequest *we;
	register struct ucb *up;
	register struct tcb *tp;
	int i;

	for (i = 0; i < CMAPSIZ; i++) if (requests[i].n_next) {
		we = &requests[i];
		up = we->n_ucb;
		tp = we->n_tcb;
		switch (we->n_request) {

		case SYSOPEN:
			if (up->uc_state == 0 && !tp->syn_acked)
          			return 1;

			if (up->uc_state != 0) {
				if (up->uc_state & UINTIMO)
					we->n_error = ENETTIM;
				else if (up->uc_state & URESET)
					we->n_error = ENETREF;
				else if (up->uc_state & UNRCH)
					we->n_error = ENETUNR;
				else if (up->uc_state & UDEAD)
					we->n_error = ENETDED;
				else
					we->n_error = ENETSTAT;
			} else if (!tp->syn_acked)
				we->n_error = EINTR;
			if (we->n_error == 0)
				we->n_retval = we->n_usrfd;
			break;

		case SYSREAD:
			if (tcp_read (we, up, 1))
				return 1;
			break;

		case SYSWRITE:
			if (tcp_write (we, up, 1))
				return 1;
			break;

		case SYSCLOSE:
#if 0
			if (tcp_close (we, up, 1))
				return 1;
#endif
			break;


		default: ;
		}

		we->n_next = 0; /* resusing field for request in-process */
 		if (we->n_error) {
#ifdef DEBUGREQ
	   		printf ("usr_reply: pid = %d, n_error = %d\n",
				we->n_pid, we->n_error);
#endif
		   	we->n_retval = -1;
		}
		sysnet (SYSREPLY, we->n_pid, we->n_proc,
			we->n_error, we->n_retval);
	}
	return 0;
}

int
usr_request (we)
struct netrequest *we;
{
	we->n_error = 0;
	we->n_retval = 0;
	we->n_next = 0;

	memcpy (&requests[we->n_slot], we, sizeof (struct netrequest));
	we = &requests[we->n_slot];

	switch (we->n_request) {
	case SYSOPEN:
		if (netopen (we)) {
			we->n_next = (struct netrequest *)1;
			return 1;
		}
		break;
	case SYSCLOSE:
		if (netclose (we)) {
			we->n_next = (struct netrequest *)1;
			return 1;
		}
		break;
	case SYSREAD:
		if (netread (we)) {
			we->n_next = (struct netrequest *)1;
			return 1;
		}
		break;
	case SYSWRITE:
		if (netwrite (we)) {
			we->n_next = (struct netrequest *)1;
			return 1;
		}
		break;
	case SYSIOCTL:
		if (netioctl (we)) {
			we->n_next = (struct netrequest *)1;
			return 1;
		}
		break;
	default: ;
	}

	if (we->n_error) {
#ifdef DEBUGREQ
	   printf ("usr_request: n_error = %d\n", we->n_error);
#endif
	   we->n_retval = -1;
	}
	sysnet (SYSREPLY, we->n_pid, we->n_proc, we->n_error, we->n_retval);
	return 0;
}
