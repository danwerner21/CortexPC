#ifndef __IOCTL__
#define __IOCTL__

int ioctl();

/* net ioctl definitions */
#ifndef NETGETS
#define NETGETS 	1		/* get status */
#define NETSETD 	2		/* set debugging info */
#define NETSETU 	3		/* set urgent mode */
#define NETRSETU 	4		/* reset urgent mode */
#define NETSETE 	5		/* set EOL mode */
#define NETRSETE 	6		/* reset EOL mode */
#define NETCLOSE 	7		/* initiate tcp close */
#define NETABORT 	8		/* initiate tcp abort */
#define NETRESET	9		/* forced tcp connection reset */
#define NETDEBUG	10		/* toggle debugging flag */
#define NETGINIT	11		/* re-read gateway file */
#define NETTCPOPT	12		/* set tcp option string */
#define NETPRADD	13		/* add to raw proto list */
#define NETPRDEL	14		/* delete from raw proto list */
#define NETPRSTAT	15		/* return raw proto list */
#define NETROUTE	16		/* override IP routing info */
#define NETOWAIT	17		/* wait for tcp connection estab */
#define NETINIT		18		/* initialize net i/f */
#define NETDISAB	19		/* disable net i/f */
#endif

#define NETALLSTATS	20		/* Get all stats */

struct ifucb {
   unsigned long host;
   unsigned int proc;
   unsigned int flags;
   unsigned int state;
   unsigned int fport;
   unsigned int lport;
   unsigned int tcb;
   unsigned char ssize;
   unsigned char rsize;
   unsigned char snd;
   unsigned char rcv;
   unsigned char tstate;
};

struct getifucbs {
   int count;
   struct ifucb uinfo[20];
};

struct ifroute {
   unsigned long lnet;
   unsigned long fnet;
   unsigned long local;
   unsigned int flags;
};

struct getifroutes {
   int count;
   struct ifroute rinfo[5];
};

struct ifstat {
   unsigned long addr;
   int mtu;
   int opkts;			/* #packets sent */
   int ipkts;			/* #packets rcvd */
   int oerrs;			/* #output errors */
   int ierrs;			/* #input errors */
   char	name[8];
   unsigned char
		avail:1,		/* i/f available */
		error:1,		/* error on i/f */
		needinit:1,		/* i/f dev needs reset */
		active:1,		/* output in progress on i/f */
		flush:1,		/* flushing input buffers */
		blocked:1,		/* i/f temporarily blocked */
		disab:1;		/* disable i/f on init */
   unsigned char xx;
};

struct getifstats {
   int count;
   struct ifstat ifinfo[4];
};

struct getifmbufs {
   short pages;
   short bufs;
   short hiwat;
   short lowat;
};

struct getnstats {
   int m_drops;
   int ip_badsum;
   int t_badsum;
   int t_badsegs;
   int t_unack;
   int ic_drops;
   int ic_badsum;
   int ic_quenches;
   int ic_redirects;
   int ic_echoes;
   int ic_timex;
};

struct getallstats {
   struct getifucbs ucbs;
   struct getifroutes routes;
   struct getifstats ifstats;
   struct getifmbufs ifmbufs;
   struct getnstats nstats;
};

#ifndef CONACT
#define CONACT		0000001		/* active connection */
#define CONTCP		0000002		/* open a tcp connection */
#define CONIP		0000004		/* open a raw ip connection */
#define CONRAW		0000010		/* open a raw local net connection */
#define CONCTL		0000020		/* open a control port (no conn) */
#define CONUDP		0000040		/* open a udp connection */
#define CONDEBUG 	0000200		/* turn on debugging info */
#define CONRAWCOMP	0001000		/* system supplies raw leaders */
#define CONRAWVER	0002000		/* system supplies cksum only */
#define CONRAWASIS	0004000		/* user supplies raw leaders */
#define CONRAWERR	0010000		/* user wants raw ICMP error msgs */
#define CONCWAIT	0020000		/* block on TCP close */
#define CONOBLOK	0040000		/* don't block on TCP open */

				/* n_flags field definitions */
#define ULISTEN		CONACT		/* awaiting a connection */
#define UTCP		CONTCP		/* this is a TCP connection */
#define UIP		CONIP		/* this is a raw IP connection */
#define URAW		CONRAW		/* this is a raw 1822 connection */
#define UCTL		CONCTL		/* this is a control port only */
#define UUDP		CONUDP		/* this is a UDP connecetion */
#define UEOL		0000100		/* EOL sent */
#define UDEBUG		CONDEBUG	/* turn on debugging info recording */
#define UURG		0000400		/* urgent data sent */
#define RAWCOMP		CONRAWCOMP	/* system supplies raw headers */
#define RAWVER		CONRAWVER	/* system supplies raw ip cksum only */
#define RAWASIS		CONRAWASIS	/* user supplies raw headers */
#define RAWERR		CONRAWERR	/* give user ICMP error messages */
#define UCWAIT		CONCWAIT	/* wait for TCP close */
#define UNOBLOK		CONOBLOK	/* don't block on TCP open */
#define ULOCK		0100000		/* receive buffer locked */
#define RAWMASK		(RAWCOMP+RAWVER+RAWASIS)
#define CONMASK		(UTCP+UIP+URAW+UCTL+UUDP)
#endif

#endif
