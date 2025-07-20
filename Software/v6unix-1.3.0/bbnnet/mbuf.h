#define MSIZE 128                       /* size of an mbuf */
#define MHEAD (2+2*sizeof(struct mbuf *)) /* mbuf hdr length (not incl data) */
#define MLEN  (MSIZE-MHEAD)             /* mbuf max data length */
#define NMBUF 80			/* total number of mbufs */

struct mbuf {                   /* message buffer */
	struct mbuf *m_next;		/* -> next msg buffer in chain */
	struct mbuf *m_act;		/* auxiliary mbuf pointer */
	u_char m_off;			/* offset in msg buf to curr. header */
	u_char m_len;			/* curr. data length in msg buf */
	u_char m_dat[MLEN];		/* data storage */
};

/* addr to head of mbuf */
#define dtom(x)  ((struct mbuf *)((int)x & ~0x7f))      
/* mbuf head to typed data */
#define mtod(x,t) ((t)((int)(x) + (x)->m_off))

#ifdef KERNEL

struct mbuf *m_get();
struct mbuf *m_free();
int m_freem();
struct mbuf *m_adj();
struct mbuf *m_copy();
struct host *h_find();
struct host *h_make();

#endif /* KERNEL */
