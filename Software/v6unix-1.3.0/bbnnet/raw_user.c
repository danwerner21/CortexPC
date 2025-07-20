#include <unistd.h>
#include <errno.h>

#include "net.h"
#include "mbuf.h"
#include "ip.h"
#include "ifcb.h"
#include "ucb.h"
#include "tcp.h"
#include "fsm.h"

int
raw_request ()
{
   struct usrwork *wp;
   struct ucb *up;
   struct mbuf *top;
   int len;

   if ((wp = usr_deque ()) == NULL)
      return -1;

   up = wp->usr_ucb;
   top = wp->usr_top;
   len = wp->usr_len;
   usr_free (wp);

   switch (up->uc_flags & CONMASK)
   {

   case UUDP:
      udp_output (up, top, len);
      break;

   case UIP:
      ip_raw (up, top, len);
      break;

   case URAW:
      (*up->uc_srcif->if_raw) (up, top, len);
      break;

   default:
      m_freem (top);
      up->uc_error = ENETPARM;
      return -1;
   }
   return 0;
}
