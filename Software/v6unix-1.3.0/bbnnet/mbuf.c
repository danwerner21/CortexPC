
#include "param.h"
#include "net.h"
#include "mbuf.h"
#include "ip.h"
#include "ifcb.h"
#include "ucb.h"

/* global variables */

extern struct net_stat netstat;

extern struct mbuf  mbuffers[NMBUF+1];
extern struct mbuf* mbuf_free;

struct mbuf *
m_get(type)
	int type;
{
	register struct mbuf* p;

	p = mbuf_free;
	if (p) {
		mbuf_free = p->m_next;
		p->m_next = NULL;
		p->m_act  = NULL;
		p->m_off  = MHEAD;
		p->m_len  = 0;
	}
	else if (!type)
		netstat.m_drops++;
	return p;
}

/* returns ->next mbuf in chain */
struct mbuf *
m_free(m)
	register struct mbuf *m;
{
	register struct mbuf* p;

	if (m==NULL) return NULL;
	p = m->m_next;
	m->m_next = mbuf_free;
	mbuf_free = m;
	return p;
}

/* returns # mbufs freed */
int
m_freem(m)
	register struct mbuf *m;
{
	register int i = 0;

	while (m) {
		m = m_free(m);
		i++;
	}
	return(i);
}


/*
 * Adjust length of mbuf chain
 */
struct mbuf *
m_adj(mp, len)
	struct mbuf *mp;
	register len;
{
	register struct mbuf *m, *n;

	if ((m = mp) == NULL)
		return;

	if (len >= 0) {
		/* adjust from top of msg chain */
		while (m != NULL && len > 0) {
			if (m->m_len <= len) {          /* free this mbuf */
				len -= m->m_len;
				m->m_len = 0;
				m = m->m_next;
			} else {                        /* adjust mbuf */
				m->m_len -= len;
				m->m_off += len;
				break;
			}
		}

	} else {
		/* adjust from bottom of msg chain */
		len = -len;
		while (len > 0 && m->m_len != 0) {

			/* find end of chain */

			while (m != NULL && m->m_len != 0) {
				n = m;
				m = m->m_next;
			}
			if (n->m_len <= len) {          /* last mbuf */
				len -= n->m_len;
				n->m_len = 0;
				m = mp;
			} else {                        /* adjust length */
				n->m_len -= len;
				break;
			}
		}
	}
}

/*
 * Copy mbuf chain.
 */
struct mbuf *
m_copy(mp)
	struct mbuf *mp;
{
	register struct mbuf *m, *n, *nn;
	struct mbuf *top;

	/* set up dummy to point to top of copy */
	top = NULL;
	nn = (struct mbuf *)&top;

	/* copy each mbuf to new, if no mbufs, free and return */
	for (m = mp; m != NULL; m = m->m_next) {
		if ((n = m_get(1)) == NULL) {
			m_freem(top);
			return(NULL);
		}
		memcpy(n, m, MSIZE);
		nn->m_next = n;
		n->m_next = NULL;
		n->m_act = NULL;
		nn = n;
	}
	return(top);
}

void
m_expand ()
{
   return;
}

void
m_relse ()
{
   return;
}

