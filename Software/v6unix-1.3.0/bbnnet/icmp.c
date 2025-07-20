#include "param.h"
#include "net.h"
#include "mbuf.h"
#include "ifcb.h"
#include "ip.h"
#include "tcp.h"
#include "raw.h"
#include "icmp.h"
#include "ucb.h"

extern int nosum;

#define NICTYPE	17

/* ICMP message formats */
#define ICBAD	0	/* unimplemented */
#define ICERR	1	/* error format (use header) */
#define ICDAT	2	/* data format (use id) */
#define ICINT	3	/* data format (handle internally) */

char icaction[NICTYPE] = { ICDAT, ICBAD, ICBAD, ICERR, ICERR, ICERR, ICBAD,
			   ICBAD, ICINT, ICBAD, ICBAD, ICERR, ICERR, ICINT,
			   ICDAT, ICINT, ICDAT };

/*
 * Process ICMP messages.  Called directly from ip_input processor.
 */
void
icmp(mp, ifp)
register struct mbuf *mp;
struct ifcb *ifp;
{
	register struct ip *ip;
	register struct icmp *icp;
	register struct ucb *up;
	u_short i;
	int op, ilen;
	struct socket temp;
	struct ucb *icfind();

	ip = mtod(mp, struct ip *);
	mp->m_off += sizeof(struct ip);
	mp->m_len -= sizeof(struct ip);
	icp = (struct icmp *)((int)ip + sizeof(struct ip));

	ilen = ip->ip_len;

	i = (u_short)icp->ic_sum;
	icp->ic_sum = 0;
	if (i != (u_short)cksum(mp, ilen)) {
		printf ("icmp: cksum\n");
		netstat.ic_badsum++;
		if (nosum)
			goto ignore;
		netlog(mp);
	} else {
ignore:
		/* decode message format, and associate msg with connection */

		if (icp->ic_type < NICTYPE)
			op = icaction[icp->ic_type];
		else
			op = ICBAD;

		switch (op) {

		case ICERR:	/* error format -- get conn from orig hdr */

			up = icfind(&icp->ic_data.hdr);
			break;

		case ICDAT:	/* data format -- get conn from id (obsolete) */
		case ICINT:	/* internal format -- don't need conn */

			break;

		default:	/* drop bad messages */
			netstat.ic_drops++;
			goto badret;
		}

		/* Now do any processing.  Internal messages get handled
		   here.  Others may be counted, or cause some other action */

		switch (icp->ic_type) {

		case ICMP_UNRCH:	/* destination unreachable */

			if (up != NULL) {
				up->uc_xstat = icp->ic_code;
				to_user(up, (icp->ic_code == IC_U_HOST)
							? UDEAD : UNRCH);
			}
			break;

		case ICMP_SRCQ:		/* source quench */

			netstat.ic_quenches++;
			break;

		case ICMP_REDIR:	/* redirect */

			if (up != NULL) {

				/* free the old route and insert the new */

				if (up->uc_route != NULL)
					h_free(up->uc_route);
				up->uc_route = h_make(&icp->ic_misc.ic_gaddr, FALSE);
				netstat.ic_redirects++;
			}
			break;

		case ICMP_ECO:		/* echo */

			/* reply to echo: swap src and dest addrs, change
			   type to zero, recompute checksum */

			icp->ic_type = ICMP_ECR;
			temp = ip->ip_src;
			ip->ip_src = ip->ip_dst;
			ip->ip_dst = temp;
			icp->ic_sum = cksum(mp, ilen);
			mp->m_off -= sizeof(struct ip);
			mp->m_len += sizeof(struct ip);
			icsend(mp, ifp, ip, ilen);
			netstat.ic_echoes++;
			return;

		case ICMP_TIMX:		/* time exceeded */
			netstat.ic_timex++;
			break;

		case ICMP_TIS:		/* timestamp */

			/* reply by changing type, swapping addrs, and
			   adding r/t timestamps.  Timestamps are not UT
			   so high order bit is set. */

			if (icp->ic_code == 0) {
				icp->ic_type = ICMP_TIR;
				icp->ic_data.ic_time.trecv = (long)time(NULL) | 1;
				icp->ic_data.ic_time.ttrans = (long)time(NULL)| 1;
				temp = ip->ip_src;
				ip->ip_src = ip->ip_dst;
				ip->ip_dst = temp;
				icp->ic_sum = cksum(mp, ilen);
				mp->m_off -= sizeof(struct ip);
				mp->m_len += sizeof(struct ip);
				icsend(mp, ifp, ip, ilen);
			}
			return;

		case ICMP_INO:		/* info request */

			/* reply by changing type, swapping addrs, and
			   filling in net part of addr */

			if (icp->ic_code == 0) {
				icp->ic_type = ICMP_INR;
				temp = ip->ip_src;
				ip->ip_src.s_addr =  ip->ip_dst.s_addr |
						netoint(iptonet(ifp->if_addr));
				ip->ip_dst.s_addr = temp.s_addr |
						netoint(iptonet(ifp->if_addr));
				icp->ic_sum = cksum(mp, ilen);
				mp->m_off -= sizeof(struct ip);
				mp->m_len += sizeof(struct ip);
				icsend(mp, ifp, ip, ilen);
			}
			return;
		}

		/* Dispatch messages to raw connections, if wanted */

		if (up != NULL && up->uc_flags&(UIP+URAW) &&
		    op == ICERR && up->uc_flags&RAWERR) {
			mp->m_off -= sizeof(struct ip);
			mp->m_len += sizeof(struct ip);
			raw_queue(mp, up);
		 } else
badret:
			netlog(mp);
	}
}

/*
 * Process GGP messages.
 */
void
ggp(mp, ifp)
register struct mbuf *mp;
struct ifcb *ifp;
{
	register struct ip *ip;
	register struct ggp *gp;
	register struct ucb *up;
	u_short i;
	int ilen;
	struct socket temp;
	struct ucb *icfind();

	ip = mtod(mp, struct ip *);
	gp = (struct ggp *)((int)ip + sizeof(struct ip));

	ilen = ip->ip_len;

	switch (gp->gg_type) {

	case GGP_UNRCH:		/* destination unreachable */

		if ((up = icfind(&gp->gg_data.gg_hdr)) != NULL) {
			up->uc_xstat = gp->gg_code;
			to_user(up, (gp->gg_code == GGP_U_HOST)
						? UDEAD : UNRCH);
		}
		break;

	case GGP_SRCQ:		/* source quench */

		up = icfind(&gp->gg_data.gg_hdr);
		netstat.ic_quenches++;
		break;

	case GGP_REDIR:		/* redirect */

		if ((up = icfind(&gp->gg_data.gg_redir.rhdr)) != NULL) {

		/* free the old route and insert the new */

			if (up->uc_route != NULL)
				h_free(up->uc_route);
			up->uc_route = h_make(&gp->gg_data.gg_redir.raddr, FALSE);
			netstat.ic_redirects++;
		}
		break;

	case GGP_ECHO:		/* echo */

		/* reply to echo: swap src and dest addrs, change
		   type to zero, recompute checksum */

		gp->gg_type = GGP_ECHR;
		temp = ip->ip_src;
		ip->ip_src = ip->ip_dst;
		ip->ip_dst = temp;
		icsend(mp, ifp, ip, ilen);
		netstat.ic_echoes++;
		return;

	default:
		netstat.ic_drops++;
	}

	/* Dispatch messages to raw connections, if wanted */

	if (up != NULL && up->uc_flags&(UIP+URAW) &&
	    i == ICERR && up->uc_flags&RAWERR)
		raw_queue(mp, up);
	else
		netlog(mp);
}

/*
 * Given pertinent fields from the original datagram header portion of
 * a GGP or ICMP packet, find the corresponding connection that originated
 * it (if any).
 */
struct ucb *
icfind(hdr)
register struct th *hdr;
{
	register struct ucb *up;
	register struct tcb *tp;
	u_short src, dst;

	src = short_from_net(hdr->t_src);
	dst = short_from_net(hdr->t_dst);

	/* match ip/tcp header fields with open connection */

	for (up=netcb.n_ucb_hd; up != NULL; up = up->uc_next) {

		if (up->uc_flags & UTCP) {	/* tcp connection */

			if ((tp = up->uc_tcb) == NULL)
				continue;

			if (tp->t_lport == src &&
			    tp->t_fport == dst &&
			    up->uc_host.s_addr == hdr->t_d.s_addr &&
			    up->uc_local.s_addr == hdr->t_s.s_addr &&
			    hdr->t_pr == TCPROTO)
				return(up);

		} else if (up->uc_flags & UIP) {	/* ip connection */

			if ((up->uc_host.s_addr == hdr->t_d.s_addr ||
			     up->uc_host.s_addr == NULL) &&
			    (up->uc_local.s_addr == hdr->t_s.s_addr ||
			     up->uc_local.s_addr == NULL) &&
			    up->uc_proto != NULL &&
			    up->uc_proto->pr_num == hdr->t_pr)
				return(up);

		} else if (up->uc_flags & URAW) {	/* raw connection */

			if ((up->uc_host.s_addr == hdr->t_d.s_addr ||
			     up->uc_host.s_addr == NULL) &&
			    (up->uc_local.s_addr == hdr->t_s.s_addr ||
			     up->uc_local.s_addr == NULL) &&
			    up->uc_proto != NULL &&
			    up->uc_proto->pr_num == up->uc_srcif->if_link)
				return(up);
		}
	}

	netstat.ic_drops++;
	return(NULL);	/* not found */
}

/*
 * Turn around an ICMP packet.
 */
void
icsend(mp, ifp, ip, ilen)
register struct mbuf *mp;
register struct ifcb *ifp;
register struct ip *ip;
register ilen;
{
	register hlen;
	struct socket *temp;
	struct socket *ip_route();

	/* fix ip header and recompute ip checksum */

	hlen = ip->ip_hl << 2;
	ilen += hlen;
	ip->ip_len = short_to_net(ilen);
	ip->ip_off = 0;
	ip->ip_id = ip->ip_id;
	ip->ip_tos = 0;
	ip->ip_sum = 0;
	ip->ip_sum = cksum(mp, hlen);

	/* pass packet to local net driver */

	if ((temp = ip_route(&ip->ip_src, &ip->ip_dst)) != NULL)
		(*ifp->if_send)(mp, ifp, temp, ilen, ifp->if_link, FALSE);
	else
		m_freem(mp);
}
