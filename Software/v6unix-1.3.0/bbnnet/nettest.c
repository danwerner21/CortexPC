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

static char str[] = "Hello world!\n";
static char imsg[2048];
static char omsg[2048];
static int sd;

static void 
sigcatcher (sig)
   int sig;
{
   if (sd)
      close (sd);
}

main (argc, argv)
     int argc;
     char **argv;
{
   unsigned long ip_addr;
   char *bp;
   int lcnt, i;
   int mlen, len;
   int tcptest;
   int printreply;
   int port;

   ip_addr = 0;
   sd = 0;
   lcnt = 1;
   tcptest = 0;
   printreply = 0;
   len = 0;
   mlen = 256;
   port = 4000;

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

            case 'n':
               i++;
               lcnt = atoi (argv[i]);
               break;

            case 'r':
               printreply = 1;
               break;

            case 't':
               tcptest = 1;
               break;

            default:
	    USAGE:
               printf ("usage: nettest [-options] host [port]\n");
               printf (" options:\n");
               printf ("    -l number    - Length of messages to send\n");
               printf ("    -n number    - Number of messages to send\n");
               printf ("    -r           - Print reply\n");
               printf ("    -t           - Send over TCP\n");
               return (1);
            }
      }
      else if (len == 0)
      {
         ip_addr = gethost (argv[i]);
	 len++;
      }
      else if (len == 1)
      {
         port = atoi (argv[i]);
	 len++;
      }
      else 
      {
         goto USAGE;
      }
   }

   printf (
    "Start: addr = %s, port = %d, tcp = %d, reply = %d, count = %d, len = %d\n",
       inet_ntoa(ip_addr), port, tcptest, printreply, lcnt, mlen);

   if (ip_addr == 0)
      goto USAGE;

   memset (&con, 0, sizeof (struct con));
   signal (SIGINT, sigcatcher);

   for (i = 0; i < mlen; i++)
      omsg[i] = i & 0xFF;
   strcpy (omsg, str);

   if (!tcptest)
   {

      printf ("Open UDP socket\n");
      con.c_mode = CONUDP | CONACT | CONRAWCOMP;

      con.c_fcon.s_addr = ip_addr;
      con.c_lcon.s_addr = 0;

      con.c_fport = port;
      con.c_lport = port;
      con.c_timeo = 60;

      sd = netopen (&con);
      if (errno)
         goto BADNESS;
      printf ("socket# %d\n", sd);

      for (i = 0; i < lcnt; i++)
      {
	 int mcnt;

         printf ("Write msg %d\n", i);
         if (write (sd, omsg, mlen) < 0)
            goto BADNESS;

	 printf ("Read msg %d\n", i);

	 while (mcnt < mlen) {
	    if ((len = read (sd, &imsg[mcnt], mlen - mcnt)) <= 0) {
	       printf ("NO REPLY\n");
	       goto BADNESS;
	    }
	    mcnt += len;
	 }

	 printf ("Received %d bytes\n", mcnt);

         if (printreply)
         {
	    printf ("Message: %s\n", imsg + sizeof (struct udp));
	 }
      }
   }

   else
   {
      printf ("Open TCP socket\n");

      con.c_mode = CONTCP | CONACT;
      con.c_fcon.s_addr = ip_addr;
      con.c_lcon.s_addr = 0;
      con.c_fport = port;
      con.c_lport = 5000;
      con.c_timeo = 60;

      if ((sd = netopen (&con)) < 0)
      {
         printf ("client open failed: error = %d\n", errno);
         exit (1);
      }
      printf ("client socket# %d\n", sd);

      for (i = 0; i < lcnt; i++)
      {
	 int mcnt;

         printf ("Write msg %d\n", i);
         if (write (sd, omsg, mlen) < 0)
	    goto BADNESS;

	 mcnt = 0;
	 printf ("Read msg %d\n", i);
	 while (mcnt < mlen) {
	    if ((len = read (sd, &imsg[mcnt], mlen - mcnt)) <= 0) {
	       printf ("NO REPLY\n");
	       goto BADNESS;
	    }
	    mcnt += len;
	 }
	 printf ("Received %d bytes\n", mcnt);

         if (printreply)
         {
	    printf ("Message: %s\n", imsg);
         }
      }
   }

   printf ("Close socket\n");
   close (sd);
   return 0;

 BADNESS:
   if (errno)
      printf ("ERROR = %d\n", errno);
   close (sd);
   return 1;
}
