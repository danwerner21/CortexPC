#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include "inet.h"

#undef NULL

#include "net.h"
#include "con.h"
#include "udp.h"

static struct con con;
static char msg[2048];
static int sd;

static void 
sigcatcher (sig)
   int sig;
{
   if (sd)
      close (sd);
}

main (argc, argv)
     char **argv;
{
   char *bp;
   struct udp *up;
   struct socket ip;
   int i;
   int len;
   int mlen;
   int port;
   int tcptest;

   sd = 0;
   tcptest = 0;
   port = 4000;
   len = 0;
   mlen = 256;

   /*
    ** Scan off args 
    */

   for (i = 1; i < argc; i++)
   {
      bp = argv[i];

      if (*bp == '-')
      {
         for (bp++; *bp; bp++)
            switch (*bp)
            {
            case 'l':
               i++;
               mlen = atoi (argv[i]);
               break;

            case 't':
               tcptest = 1;
               break;

            default:
	       USAGE:
               printf ("usage: netserv [-options] port\n");
               printf (" options:\n");
               printf ("    -l number    - Length of messages to read\n");
               printf ("    -t           - TCP\n");
               return (1);
            }
      }
      else if (len == 0)
      {
         port = atoi(argv[i]);
	 len++;
      }
      else
         goto USAGE;
   }

   printf ("Start: port = %d, tcp = %d\n", port, tcptest);

   memset (&con, 0, sizeof (struct con));
   signal (SIGINT, sigcatcher);

   if (!tcptest)
   {
      printf ("Open UDP socket\n");
      con.c_mode = CONUDP | CONRAWVER;
      con.c_fport = 0;
      con.c_lport = port;
      con.c_fcon.s_addr = 0;
      con.c_lcon.s_addr = 0;
      con.c_sbufs = con.c_rbufs = 1;

      sd = netopen (&con);
      printf ("   sd = %d, error = %d\n", sd, errno);
      if (errno)
         goto BADNESS;

      while (1)
      {
         len = read (sd, msg, sizeof (msg));
	 printf ("   len = %d, error = %d\n", len, errno);
         if (len <= 0) {
	    break;
	 }
	 /* swap the addresses and ports */
	 up = (struct udp *)msg;
	 ip = up->u_s;
	 up->u_s = up->u_d;
	 up->u_d = ip;
	 i = up->u_src;
	 up->u_src = up->u_dst;
	 up->u_dst = i;
	 if (write (sd, msg, len) < 0)
	    break;
      }
   }

   else
   {
      printf ("Open TCP socket\n");

      con.c_mode = CONTCP;
      con.c_fport = 0;
      con.c_lport = port;
      con.c_fcon.s_addr = 0;
      con.c_lcon.s_addr = 0;
      con.c_sbufs = con.c_rbufs = 1;

      sd = netopen (&con);
      printf ("   sd = %d, error = %d\n", sd, errno);
      if (errno)
         goto BADNESS;

      while (1)
      {
	 int mcnt;

	 mcnt = 0;
	 while (mcnt < mlen) {
            if ((len = read (sd, &msg[mcnt], mlen - mcnt)) <= 0) {
	       mcnt = -1;
	       break;
	    }
	    mcnt += len;
	 }
	 if (mcnt < 0) {
	    break;
	 }
	 printf ("   len = %d, error = %d\n", mcnt, errno);
         if (write (sd, msg, mcnt) < 0)
	    break;
      }
   }

 BADNESS:
   if (errno)
      printf ("ERROR = %d\n", errno);
   close (sd);
   return 0;
}
