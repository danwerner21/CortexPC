#include "net.h"
#include "mbuf.h"
#include "ip.h"
#include "ifcb.h"

/* Queue datagram on input queue */
int
lp_send (m, ip, addr, len, link)
     register struct mbuf *m;
     register struct ifcb *ip;
     struct socket *addr;
     register len;
     int link;
{
#ifdef DEBUG
   register struct mbuf *n;
   unsigned char *p;
   int i;

   printf ("lp_send: entered\n");
   for (n = m; n; n = n->m_next)
   {
      printf ("mbuf >%x: m_next = >%x, m_act = >%x, m_len = %d, m_off = %d\n",
              n, n->m_next, n->m_act, n->m_len, n->m_off);
   }
#endif
   m->m_act = NULL;
   if (ip->if_inq_hd != NULL)
      ip->if_inq_tl->m_act = m;
   else
      ip->if_inq_hd = m;
   ip->if_inq_tl = m;
   ip->if_opkts++;

   return (TRUE);
}

int
lp_rcv (ip)
     register struct ifcb *ip;
{
   register struct mbuf *m;

#ifdef DEBUG
   register struct mbuf *n;
   unsigned char *p;
   int i;

   printf ("lp_rcv: entered\n");
#endif

   /* process all messages on input queue */

   while ((m = ip->if_inq_hd) != NULL)
   {
#ifdef DEBUG
      for (n = m; n; n = n->m_next)
      {
         printf
            ("mbuf >%x: m_next = >%x, m_act = >%x, m_len = %d, m_off = %d\n",
             n, n->m_next, n->m_act, n->m_len, n->m_off);
      }
#endif
      ip->if_inq_hd = m->m_act;
      m->m_act = NULL;
      ip->if_ipkts++;
      ip_input (m, ip);
   }
   return 0;
}

int
lp_raw ()
{
   ;
}

int
lp_init (ip)
     register struct ifcb *ip;
{
   ip->if_avail = 1;
   return 0;
}

int
lp_out ()
{
   ;
}

int
lp_poll (ip)
     register struct ifcb *ip;
{
#ifdef DEBUGP
   printf ("lp_poll: entered: if_inq_hd = >%x\n", ip->if_inq_hd);
#endif
   if (ip->if_inq_hd)
      return (1);
   return (0);
}
