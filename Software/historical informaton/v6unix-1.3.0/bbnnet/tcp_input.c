#define DEBUG
#include "param.h"
#include "sysnet.h"

#include "net.h"
#include "mbuf.h"
#include "ifcb.h"
#include "ucb.h"
#include "tcp.h"
#include "ip.h"
#include "fsm.h"
#include "fsmdef.h"

extern char *inet_ntoa();

extern int nosum;

/*
 * This is the scheduler for the tcp machine.  It is called
 * from the lower network levels, either directly from the
 * internet level, in case of input from the network; or
 * indirectly from netmain, in case of user or timer events
 * which awaken the main loop.
 */
void
tcp_input(mp, ip)
struct mbuf *mp;
struct ifcb *ip;
{
	register struct tcb *t;
	register struct th *tp;
	register struct work *p;
	register struct mbuf *m;
	register i, hlen, tlen;
	u_short lport, fport;
	struct ucb *up;
	struct work w;
	struct socket temp;
	struct ucb tempucb;

#ifdef DEBUGINPUT
	printf ("tcp_input: mp = 0x%x, ip = 0x%x\n", mp, ip);
#endif
	if (mp != NULL) {       /* called directly from ip with a datagram */

		/* set up needed info from ip header, note that beginning
		   of tcp header struct overlaps ip header.  ip options
		   have been removed by ip level option processing */

		tp = mtod(mp, struct th *);

		/* make sure header does not overflow mbuf */

		if ((hlen = tp->t_off << 2) > mp->m_len) {
			printf("tcp header overflow\n");
			netlog(mp);
			return;
		}

		tlen = ((struct ip *)tp)->ip_len;
		tp->t_len = short_to_net(tlen);
		tp->t_next = NULL;
		tp->t_prev = NULL;
		tp->t_x1 = 0;
		lport = short_from_net(tp->t_dst);
		fport = short_from_net(tp->t_src);
#ifdef DEBUGINPUT
		printf ("   pkt: lport = %d, laddr = %s\n",
			lport, inet_ntoa(tp->t_s.s_addr));
		printf ("   pkt: fport = %d, faddr = %s\n",
			fport, inet_ntoa(tp->t_d.s_addr));
#endif

		/* do checksum calculation, drop seg if bad */

		i = (u_short)tp->t_sum;
		tp->t_x0 = 0;
		tp->t_x00 = 0;
		tp->t_sum = 0;

		if (i != (u_short)cksum(mp, tlen + sizeof(struct ip))) {
#ifdef DEBUGINPUT
			printf ("tcp_input: cksum\n");
#endif
        		netstat.t_badsum++;
			if (nosum)
				goto ignore;
        		netlog(mp);

		} else {
ignore:
        		/* find a tcb for incoming message */

#ifdef DEBUGINPUT
			printf ("   check for exact match\n");
#endif
        		t = netcb.n_tcb_head;
        		while (t != NULL) {     /* look for exact match */

#ifdef DEBUGINPUT
			printf ("   tcb: lport = %d, laddr = %s\n",
				t->t_lport, inet_ntoa(t->t_ucb->uc_host.s_addr));
			printf ("   tcb: fport = %d, faddr = %s\n",
				t->t_fport, inet_ntoa(t->t_ucb->uc_local.s_addr));
#endif
        			if (t->t_lport == lport &&
        				t->t_fport == fport &&
        		                t->t_ucb->uc_host.s_addr ==
                                                          tp->t_s.s_addr &&
        		                t->t_ucb->uc_local.s_addr ==
                                                          tp->t_d.s_addr)
        				break;
        			t = t->t_tcb_next;
        		}

        		if (t == NULL) {        /* look for wild card match */

#ifdef DEBUGINPUT
				printf ("   check for wild card\n");
#endif
        		        t = netcb.n_tcb_head;
        			while (t != NULL) {

#ifdef DEBUGINPUT
			printf ("   tcb: lport = %d, laddr = %s\n",
				t->t_lport, inet_ntoa(t->t_ucb->uc_host.s_addr));
			printf ("   tcb: fport = %d, faddr = %s\n",
				t->t_fport, inet_ntoa(t->t_ucb->uc_local.s_addr));
#endif
        				if (t->t_lport == lport &&
        				    (t->t_fport == fport ||
        				     t->t_fport == 0) &&
        				     (t->t_ucb->uc_host.s_addr ==
							   tp->t_s.s_addr ||
        				      t->t_ucb->uc_host.s_addr == 0) &&
        				     (t->t_ucb->uc_local.s_addr ==
							   tp->t_d.s_addr ||
        				      t->t_ucb->uc_local.s_addr == 0))
        		 		        break;
                			t = t->t_tcb_next;
        			}
        		}

        		if (t != NULL) {        /* found a tcp for message */

				/* byte swap header */

				tp->t_len = tlen - hlen;
				tp->t_src = fport;
				tp->t_dst = lport;
				tp->t_seq = long_from_net(tp->t_seq);
				tp->t_ackno = long_from_net(tp->t_ackno);
				tp->t_win = short_from_net(tp->t_win);
				tp->t_urp = short_from_net(tp->t_urp);

				/* do TCP option processing */

				if (hlen > TCPSIZE)
					tcp_opt(t, tp, hlen);

			        /* check seg seq #, do RST processing */

				if ((i = netprepr(t, tp)) != 0) {

				/* seg failed preproc, drop it and
				   possibly enter new state */

					m_freem(mp);
					if (i != -1)
						t->t_state = i;
				} else {

			                /* put msg on unack queue if user
					   rcv queue is full and there is
					   not a reasonable number of free
					   buffers available. */

					up = t->t_ucb;
			                if ((int)up->uc_rcv - (int)up->uc_rsize <= 0
					     && tp->t_len != 0
					     && netcb.n_bufs < netcb.n_lowat) {

			                	mp->m_act = NULL;
			                	if ((m = t->t_rcv_unack) != NULL) {
			                		while (m->m_act != NULL)
			                			m = m->m_act;
			                		m->m_act = mp;
			                	} else
			                		t->t_rcv_unack = mp;

			                } else {

					/* set up work entry for seg, and call
					   the fsm to process it */

					        hlen += sizeof(struct ip);
					        mp->m_off += hlen;
					        mp->m_len -= hlen;
                        			w.w_tcb = t;
                        			w.w_dat = (char *)tp;
                        			action(&w, INRECV);
					}
				}

			} else {        /* nobody wants it */

				/* make sure we don't send a RST in
				   response to an RST */

#ifdef DEBUGINPUT
				printf ("   no match\n");
#endif
				if (tp->t_flags&T_RST) {
					m_freem(mp);
					return;
				}

        			/* free everything but the header */

        			m_freem(mp->m_next);
        			mp->m_next = NULL;
        			mp->m_len = sizeof(struct th);

        			/* form a reset from the packet and send */

        			temp = tp->t_d;
        			tp->t_d = tp->t_s;
        			tp->t_s = temp;
        			lport = tp->t_src;
        			tp->t_src = tp->t_dst;
        			tp->t_dst = lport;
        			if (tp->t_flags&T_ACK)
        				tp->t_seq = tp->t_ackno;
        			else {
        				tp->t_ackno = long_to_net(
						long_from_net(tp->t_seq)
						+ tlen - hlen
						+ (tp->t_flags&T_SYN ? 1 : 0));
        				tp->t_seq = 0;
        			}
				tp->t_flags = (tp->t_flags&T_ACK) ? T_RST
								  : T_RST+T_ACK;
        			tp->t_len = short_to_net(TCPSIZE);
        			tp->t_off = TCPSIZE/4;
				tp->t_sum = cksum(mp, sizeof(struct th));
				tempucb.uc_host = tp->t_d;
				tempucb.uc_route = NULL;
				tempucb.uc_local = tp->t_s;
				tempucb.uc_srcif = ip;
        			ip_send(&tempucb, mp, TCPROTO, TCPSIZE, 0,
					NULL, FALSE);
				if (tempucb.uc_route != NULL)
					h_free(tempucb.uc_route);
        			netstat.t_badsegs++;
			}
		}
	}

	/* now look through the work list for user or timer events */

	for (p = netcb.n_work; p != NULL; p = netcb.n_work) {

		if ((i = p->w_type) > 0 && i < INOP)
			action(p, i);

		p->w_type = 0;
		netcb.n_work = p->w_next;
		p->w_next = NULL;
	}
	nwakeup(workfree);       /* in case anyone waiting for work entries */
}

/*
 * Entry into TCP finite state machine
 */
void
action(wp, input)
register struct work *wp;
int input;
{
	register act, newstate;
	register struct tcb *tp;

#ifdef DEBUG
	printf ("action: wp = 0x%x, input = %d\n", wp, input);
#endif
	tp = wp->w_tcb;

	/* get index of action routine from transition table */

	act = fstab[tp->t_state][input];

	/* invalid state transition, just print a message and ignore */

	if (act == 0)  {
		printf("tcp: bad state: tcb=%X state=%d input=%d\n",
			tp, tp->t_state, input);
		if (wp->w_dat != NULL)
			m_freem(dtom(wp->w_dat));
		return;
	}

	tp->net_keep = FALSE;

	newstate = (*fsactab[act])(wp);         /* call action routine */

	/* print debugging info for this transition if debug flag on */

#ifdef TCPDEBUG
	if (tp->t_ucb->uc_flags & UDEBUG)
		tcp_debug(tp, wp, input, newstate);

	tcp_prt(tp, input, wp->w_stype, newstate);
#endif

	if (newstate != SAME)
		tp->t_state = newstate;
	if (wp->w_dat != NULL && !tp->net_keep && input == INRECV)
		m_freem(dtom(wp->w_dat));
}

/*
 * Allocate a work list entry
 */
void
w_alloc(type, stype, tp, m, we)
register type;
int stype;
struct tcb *tp;
char *m;
struct netrequest *we;
{
	register struct work *w, *wp;

	/* look for a free work entry */

again:
	for (w = workfree; w < workNWORK; w++)
		if (w->w_type == 0) {

			/* found a free entry */

			w->w_type = type;
			w->w_stype = stype;
			w->w_tcb = tp;
			w->w_dat = m;
			if (we) {
				w->w_uproc = we->n_pid;
#ifdef DEBUG
				printf ("w_alloc: n_pid = 0x%x\n", w->w_uproc);
#endif
			} else {
				w->w_uproc = 0;
			}
			/* chain work request at end of list */

         		w->w_next = NULL;

        		if ((wp = netcb.n_work) != NULL) {
        			while (wp->w_next != NULL)
        				wp = wp->w_next;
        			wp->w_next = w;
        		} else
        			netcb.n_work = w;

			/* wakeup net input process */

			wakeup(&netmain);
			return;
		}

	/* couldn't find a free entry: wait for one if request is from user */

	if (type != INRECV && type != ISTIMER && type != INCLEAR) {
#ifdef DEBUG
		printf ("w_alloc: woorkfree sleep\n");
#endif
		nsleep(workfree, PZERO);
		goto again;

	/* if timer request, just reschedule a second later */

	} else if (type == ISTIMER) {
		tp->t_timers[stype]++;

	/* otherwise, just drop the request, since it can only be
	   a net clear or an unacked queue recycle (regular net input
	   is done with a private work entry). */

	} else
		if (m != NULL)
			m_freem(dtom(m));
}

#ifndef TCPDEBUG

tcp_debug()
{
	;
}

#else

/*
 * Write a record in the tcp debugging log
 */
tcp_debug(tp, wp, input, newstate)
register struct tcb *tp;
register struct work *wp;
register input, newstate;
{
#if 0
	struct t_debug debuf;
	register struct th *n;
	register struct inode *ip;
	register long siz;

	/* look for debugging file inode */

	if ((ip=netcb.n_debug)==NULL)
		return;
	plock(ip);

	/* set up the debugging record */

	debuf.t_tod = (long)time;
	if ((debuf.t_tcb = tp) != NULL)
		debuf.t_old = tp->t_state;
	else
		debuf.t_old = 0;
	debuf.t_inp = input;
	debuf.t_tim = wp->w_stype;
	debuf.t_new = newstate;

	if (input == INRECV) {
		n = (struct th *)wp->w_dat;
		debuf.t_sno = n->t_seq;
		debuf.t_ano = n->t_ackno;
		debuf.t_wno = n->t_win;
		if (debuf.t_new == 0xff)
	        	debuf.t_lno = ((struct ip *)n)->ip_len;
		else
			debuf.t_lno = n->t_len;
		debuf.t_flg = *(char *)((int)&n->t_win - 1);
	} 

	/* write out the record */

	siz = ip->i_size;
	u.u_offset = siz;
	u.u_base = (char*)&debuf;
	u.u_count = sizeof(debuf);
	u.u_segflg = 1;
	u.u_error = 0;
	writei(ip);
	if(u.u_error)
		ip->i_size = siz;
	prele(ip);
#endif
}


/*
 * Print debugging info on the console
 */
tcp_prt(tp, input, timer, newstate)
register struct tcb *tp;
register input, timer, newstate;
{
	register oldstate;

	oldstate = tp->t_state;

	printf("TCP(%x) %s X %s", tp, tcpstates[oldstate], tcpinputs[input]);

	if (input == ISTIMER)
		printf("(%s)", tcptimers[timer]);

	printf(" --> %s", tcpstates[ (newstate > 0) ? newstate : oldstate ]);

	if (newstate < 0)
		printf(" (FAILED)\n");
	else
		putchar('\n');
}

#endif
