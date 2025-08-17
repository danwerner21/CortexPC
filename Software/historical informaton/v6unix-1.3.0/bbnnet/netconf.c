#include <stdio.h>
#include <errno.h>

#include "inet.h"

#include "net.h"
#include "mbuf.h"
#include "ip.h"
#include "ifcb.h"

#define CONFIG_FILE "/etc/ifaces"

extern struct ifcb ifcb[];
extern struct net netcb;

extern char *scantok();

int lp_send(), lp_rcv(), lp_raw(), lp_init(), lp_out(), lp_poll();
int sl0_send(), sl0_rcv(), sl0_raw(), sl0_init(), sl0_out(), sl0_poll();

void
netconf()
{
   FILE *fd;
   char *cp;
   int i;
   struct ifcb *ip;
   char buffer[82];
   char token[20];

   memset (&netcb, 0, sizeof(netcb));

   if ((fd = fopen (CONFIG_FILE, "r")) == NULL)
   {
      printf ("Can't open %s: %s\n", CONFIG_FILE, sys_errlist[errno]);
      return;
   }

   for (i = 0; i < NIFCB; i++)
   {
      ip = &ifcb[i];
      memset (ip, 0, sizeof(struct ifcb));

   READ_AGAIN:
      if (fgets (buffer, sizeof (buffer), fd) == NULL)
      {
         if (ferror (fd))
	 {
	    printf ("Can't read %s: %s\n", CONFIG_FILE, sys_errlist[errno]);
	 }
	 return;
      }

      cp = scantok (buffer, token);
      if (token[0] == '#' || token[0] == 0)
         goto READ_AGAIN;

      if (!strcmp (token, "LOOP"))
      {
	 ip->if_send = &lp_send;
	 ip->if_rcv = &lp_rcv;
	 ip->if_raw = &lp_raw;
	 ip->if_init = &lp_init;
	 ip->if_out = &lp_out;
	 ip->if_poll = &lp_poll;
      }
      else if (!strcmp (token, "SLIP"))
      {
	 ip->if_send = &sl0_send;
	 ip->if_rcv = &sl0_rcv;
	 ip->if_raw = &sl0_raw;
	 ip->if_init = &sl0_init;
	 ip->if_out = &sl0_out;
	 ip->if_poll = &sl0_poll;
      }
      else
      {
         printf ("Unknown interface type: %s\n", token);
	 return;
      }

      cp = scantok (cp, token);
      strcpy (ip->if_name, token);
      cp = scantok (cp, token);
      ip->if_mtu = atoi (token);
      cp = scantok (cp, token);
      ip->if_addr.s_addr = inet_addr (token);
      if_attach (ip);
   }

   fclose (fd);
}

