#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "param.h"
#include "sysnet.h"

#include "inet.h"
#include "net.h"
#include "mbuf.h"
#include "ifcb.h"
#include "tcp.h"
#include "ip.h"
#include "ucb.h"
#include "fsm.h"

extern struct net netcb;
extern struct host hosts[NHOST], *host, *hostNHOST;

extern struct work works[NWORK], *workfree, *workNHOST, *workNWORK;
extern int nwork;

extern struct gway gates[NGATE], *gateway, *gateNGATE;
extern int ngate;

extern struct mbuf  mbuffers[NMBUF+1];
extern struct mbuf* mbuf_free;

#ifdef BUFSTAT
struct buf_stat bufnull = { 0 };
#endif /* BUFSTAT */

extern int usr_request();
extern int usr_reply();

/*
 * Initialize memory
 */
static void
meminit()
{
	register struct mbuf* p;
	register int i;

	p = (struct mbuf*)((int)(mbuffers+1) & ~0x7f);
	mbuf_free = p;
	for(i = 0; i < NMBUF-1; i++) {
		memset (p, 0, sizeof (struct mbuf));
		p->m_next = p+1;
		p++;
	}

	memset (&hosts, 0, sizeof (hosts));
	memset (&works, 0, sizeof (works));

	host = &hosts[0];
	hostNHOST = &hosts[NHOST-1];

	workfree = &works[0];
	workNWORK = &works[NWORK-1];

	gateway = &gates[0];
	gateNGATE = &gates[0];
	ngate = 0;
}

/*
 * Net timer routine: scheduled every second, calls ip and tcp timeout routines
 * and does any net-wide statistics.
 */
static void
net_timer ()
{
#ifdef BUFSTAT
   register struct mbuf *m, *n;
   register struct ucb *up;
   register struct tcb *tp;
   register struct th *t;
   register struct ifcb *lp;
   register struct ipq *ip;
   register struct ip *i;
   register struct host *h;
   register struct work *w;
#endif

   ip_timeo ();
   tcp_timeo ();

#ifdef BUFSTAT
   bufstat = bufnull;
   for (up = netcb.n_ucb_hd; up != NULL; up = up->uc_next)
   {
      if (up->uc_flags & UTCP)
      {
         if ((tp = up->uc_tcb) == NULL)
            continue;
         bufstat.b_tcbs++;
         if (tp->t_state != ESTAB)
            bufstat.b_cons--;
         if ((t = tp->t_rcv_next) == NULL)
            continue;
         for (; t != (struct th *) tp; t = t->t_next)
         {
            m = dtom (t);
            while (m != NULL)
            {
               bufstat.b_tseq++;
               m = m->m_next;
            }
         }
         for (m = tp->t_rcv_unack; m != NULL; m = m->m_act)
         {
            n = m;
            do
            {
               bufstat.b_tuna++;
               n = n->m_next;
            }
            while (n != NULL);
         }
      }
      bufstat.b_cons++;
      bufstat.b_useq += up->uc_rsize + up->uc_ssize;
   }
   for (ip = netcb.n_ip_head; ip != NULL; ip = ip->iq_next)
      for (i = ip->iqx.ip_next; i != (struct ip *) ip; i = i->ip_next)
      {
         m = dtom (i);
         while (m != NULL)
         {
            bufstat.b_ipfr++;
            m = m->m_next;
         }
      }
   for (lp = netcb.n_ifcb_hd; lp != NULL; lp = lp->if_next)
   {
      for (m = lp->if_outq_hd; m != NULL; m = m->m_act)
      {
         n = m;
         do
         {
            bufstat.b_devq++;
            n = n->m_next;
         }
         while (n != NULL);
      }
      for (m = lp->if_outq_cur; m != NULL; m = m->m_next)
         bufstat.b_devq++;
      for (m = lp->if_inq_hd; m != NULL; m = m->m_act)
      {
         n = m;
         do
         {
            bufstat.b_devq++;
            n = n->m_next;
         }
         while (n != NULL);
      }
      for (m = lp->if_inq_msg; m != NULL; m = m->m_next)
         bufstat.b_devq++;
   }
   for (h = host; h < hostNHOST; h++)
      if (h->h_refct > 0)
         for (m = h->h_outq; m != NULL; m = m->m_act)
         {
            n = m;
            do
            {
               bufstat.b_rfnm++;
               n = n->m_next;
            }
            while (n != NULL);
         }
   for (w = netcb.n_work; w != NULL; w = w->w_next)
      if (w->w_type == INRECV || w->w_type == IUSEND)
         for (m = dtom (w->w_dat); m != NULL; m = m->m_next)
            bufstat.b_work++;
#endif /* BUFSTAT */
}

/*
 * Generic local net reset:  clear input and output queues and call driver
 * init routine.
 */
void
netreset (ip)
     register struct ifcb *ip;
{
   register struct mbuf *m, *n;

   ip->if_avail = FALSE;

   /*
    * clear all i/f queues
    */
   for (m = ip->if_inq_hd; m != NULL; m = n)
   {
      n = m->m_act;
      m_freem (m);
   }
   ip->if_inq_hd = NULL;
   ip->if_inq_tl = NULL;

   for (m = ip->if_outq_hd; m != NULL; m = n)
   {
      n = m->m_act;
      m_freem (m);
   }
   ip->if_outq_hd = NULL;
   ip->if_outq_tl = NULL;

   if (ip->if_inq_msg != NULL)
   {
      m_freem (ip->if_inq_msg);
      ip->if_inq_msg = NULL;
      ip->if_inq_cur = NULL;
   }

   if (ip->if_outq_cur != NULL)
   {
      m_freem (ip->if_outq_cur);
      ip->if_outq_cur = NULL;
   }

   h_reset (ip->if_addr);
   ip->if_resets++;

   /*
    * now call net driver specific init routine to reinit
    */
   (*ip->if_init) (ip);
}

char *
scantok (cp, t)
     char *cp;
     char *t;
{
   *t = 0;
   while (*cp && (*cp == ' ' || *cp == '\t'))
      cp++;
   while (*cp && *cp != ' ' && *cp != '\n' && *cp != '\t')
      *t++ = *cp++;
   *t = 0;
   return (cp);
}

/*
 * Read the gateway file into the gateway table.  The file
 * contains gateway structure entries produced from the ASCII gateway
 * table.  Called from netmain or netioctl, returns number of entries
 * successfully read or -1 if error
 */
static int
gatinit ()
{
   char *cp;
   register int i;
   FILE *fd;
   register struct ifcb *lp;
   register struct gway *gp;
   struct socket ipa;
   struct gway gatent;
   char *gatefile = "/etc/gateways";
   char buff[80], token[20];

   /* set up to read the gateway file */

   if ((fd = fopen (gatefile, "r")) == NULL)
   {
      printf ("Can't open gateway file: %s: %s\n",
              gatefile, sys_errlist[errno]);
      return (-1);
   }

   /* read entries from the file into the table */

   for (gp = gateway, i = NGATE; i > 0; i--)
   {
    READ_AGAIN:
      if ((fgets (buff, sizeof (buff), fd)) == NULL)
      {
	 if (ferror (fd))
	 {
	    printf ("Can't read gateway file: %s: %s\n",
		    gatefile, sys_errlist[errno]);
	 }
         goto out;
      }

      /* Scan off gateway components */

      cp = scantok (buff, token);
      if (token[0] == '#' || token[0] == 0)
         goto READ_AGAIN;

      memset (&gatent, 0, sizeof (gatent));

      ipa.s_addr = inet_addr (token);
      gatent.g_lnet = iptonet (ipa);

      cp = scantok (cp, token);
      ipa.s_addr = inet_addr (token);
      gatent.g_fnet = iptonet (ipa);

      cp = scantok (cp, token);
      gatent.g_local.s_addr = inet_addr (token);

      cp = scantok (cp, token);
      gatent.g_flags = atoi (token);

#ifdef DEBUGGATE
      printf ("gatent: lnet = %s", inet_ntoa (gatent.g_lnet));
      printf (", fnet = %s", inet_ntoa (gatent.g_fnet));
      printf (", gate = %s, flags = %ld\n",
              inet_ntoa (gatent.g_local.s_addr), gatent.g_flags);
#endif

      /* look for i/f corresponding to gateway local net addr */

      for (lp = netcb.n_ifcb_hd; lp != NULL; lp = lp->if_next)
      {
#ifdef DEBUGGATE
         printf ("   lp_addr = %lX, g_addr = %lX\n",
                 lp->if_addr.s_addr, gatent.g_local.s_addr);
#endif
         if ((u_long) lp->if_addr.s_addr == (u_long) gatent.g_local.s_addr)
         {
            gatent.g_ifcb = lp;
	    ngate++;
	    *gp = gatent;
#ifdef DEBUGGATE
	    printf ("gp: lnet = %s", inet_ntoa (gp->g_lnet));
	    printf (", fnet = %s", inet_ntoa (gp->g_fnet));
	    printf (", gate = %s, flags = %ld\n",
		    inet_ntoa (gp->g_local.s_addr), gp->g_flags);
#endif
	    gp++;
	    break;
         }
      }
      if (lp == NULL)
      {
	 printf ("Invalid gateway addr %s, entry %d ignored\n",
		 inet_ntoa (gatent.g_local.s_addr), NGATE - i + 1);
      }
   }
   if (i == 0)
      printf ("Gateway table full\n");
 out:
   gateNGATE = gp;
#ifdef DEBUGGATE
   printf ("gp = 0x%x, gateway = 0x%x, gateNGATE = 0x%x\n",
	   gp, gateway, gateNGATE);
#endif
   fclose (fd);
   return (NGATE - i + 1);
}

/*
 * This is the mainline of the network input process.  First, we
 * fork off as a separate process within the kernel, initialize the
 * net buffer manager, local net interfaces, and timers.  Then, we
 * loop forever, sleeping on net input.  Once awakened by a local net
 * driver, we dispatch to each local interface in turn, to process the
 * input message(s).  After handling the interfaces, we adjust the net
 * buffer allocation, call on the tcp processor to handle any outstanding
 * timer or user events, and...
 */
int
main ()
{
   register struct ifcb *lp;
   struct netrequest request;

   meminit ();                  /* Init buf mgmt system */
   netconf ();                  /* Configure the ifcb(s) */
   gatinit ();                  /* Read gateway file and init table */
#if 0
   usr_init ();                 /* Configure user */
#endif

   /* initialize all local net i/fs */

   for (lp = netcb.n_ifcb_hd; lp != NULL; lp = lp->if_next)
   {
      if ((*lp->if_init) (lp) < 0)
      {
         printf ("if_init: error = %d\n", errno);
         exit (1);
      }
   }

   /* start timers */

   net_timer ();
   sysnet (SYSINIT, NULL, NULL);

   printf ("Network process running.\n");

   for (;;)
   {                            /* body of input process */

      /*
       * reset local i/f if necessary, otherwise just handle net and user input
       */
      for (lp = netcb.n_ifcb_hd; lp != NULL; lp = lp->if_next)
      {
       process:
         if (lp->if_error && !lp->if_needinit)
         {
            lp->if_needinit = TRUE;
            netreset (lp);
         }
         else if (lp->if_avail && lp->if_inq_hd != NULL)
            (*lp->if_rcv) (lp);
      }

      /* Call TCP processor to handle any user or timer events. */

   checkwork:
      if (netcb.n_work != NULL)
	 tcp_input (NULL, NULL);

#ifdef SUPPORTRAW
      /* Check for user raw/udp requests */
      raw_request ();
#endif

      /* Expand or shrink the buffer freelist as needed */
      if (netcb.n_bufs < netcb.n_lowat)
	 m_expand ();
      else if (netcb.n_bufs > netcb.n_hiwat)
	 m_relse ();

      /* See if we have work from the drivers */

      for (lp = netcb.n_ifcb_hd; lp != NULL; lp = lp->if_next)
      {
         if ((*lp->if_poll) (lp) == 0) {
            if (lp->if_inq_hd != NULL)
               goto process;
	 }
      }

      /* Check timer events */

      net_timer();

      /* See if we have any work to/from users */

      usr_reply ();
      if (sysnet (SYSREQUEST, &request, sizeof request) != NULL) {
	     usr_request (&request);
	     goto checkwork;
      }

      ksleep (&netmain, PUSER);
   }
}
