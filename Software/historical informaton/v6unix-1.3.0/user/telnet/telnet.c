/*
 * Simple-minded user telnet - 2 process version
 */
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "net.h"
#include "con.h"
#include "globdefs.h"
#include "tnio.h"
#include "telnet.h"
#include "ttyctl.h"

#ifdef DEBUG
#include "hexdump.h"
#endif

#define BUFSIZE 256

NETCONN *NetConP;	/* Current connection */
TTYMODE *NetModeP;	/* TTY mode block for connection */

int escape;		/* Escape char */
int needprompt;		/* Initially, prompt for user input */
int inputfd;
int fsocket;		/* Default foreign socket for open commands */
int debug;		/* Debugging the protocol -- print a lot */
int verbose;		/* Makes TELNET library print info */
int udone;		/* Set when user done */
int ndone;		/* Set when network connection done */
int xdone;		/* Set to abort without waiting for net */
int usercr;		/* Follow CR with LF on input */

char prompt[32];	/* Prompt sequence printed when escape typed */

struct termstat {
   char row;
   char col;
   int len;
};

/* -------------------------- C H E C K _ D O N E ------------------- */
/*
 * Examine the environment (all the done flags, etc.) and if done,
 * kill other process (if it exists) and exit.
 */
int
check_done()
{

    if (xdone ||    /* Aborting exit */
	((udone || ndone) &&	/* Otherwise make sure net quiescent */
	 (NetConP == NULL || telempty(NetConP) == 0)))
    {
	if (NetConP != NULL)
	{
	   if (udone)
	   {
	       telfinish(NetConP);
	   }
	   else
	   {
	       telclose(NetConP);
	   }
	}
	ChgMode(OrigMode());
	exit(0);
    }
    return(1);
}

/* -------------------------- F R O M _ U S E R --------------------------- */
/*
 * Copy from terminal and process what you get, sending it to command
 * processor or network as appropriate.
 */
from_user(fd)
    int fd;
{
    int nread;
    char buf[BUFSIZE];

    for(;;)
    {
	if (needprompt)
	{
	    write (2, prompt, strlen(prompt));
	    needprompt = 0;
	}
	check_done();
	if ((nread = read (fd, buf, sizeof(buf))) < 0) {
	    if (errno != EINTR)
	    	fprintf (stderr,
			 "Error occurred while reading standard input: %s\n",
	 		 sys_errlist[errno]);
	     continue;
	}
	process(buf, nread, NetConP);
	check_done();
    }
}

/* -------------------------- F R O M _ N E T ----------------------- */
/*
 * Copy from net to terminal.
 */
void
from_net(fd, connp)
    int fd;
    NETCONN * connp;
{
    int nread;
    int work;
    int gotpass;
    int savflags;
    struct netstate nstat;
    struct sgttyb ttyb;
    char nbuf[BUFSIZE];

    gtty (0, &ttyb);
    savflags = ttyb.sg_flags;

    gotpass = 0;
    for (;;)
    {
	work = 0;
	check_done();
	if (ioctl (connp->NetFD, NETGETS, &nstat) < 0)
	{
	   ndone++;
	}

	if (nstat.n_rsize)
	{
	   nread = telread(connp, nbuf, sizeof(nbuf));
	   if (nread == TELEOF || nread == TELERR)
	   {
	      if (nread == TELERR)
		 fprintf (stderr, "Network read error: %s\n", 
	 		  sys_errlist[errno]);
	      ndone++;
	   }
	   else if (nread > 0)
	   {
	      work = 1;

#ifdef DEBUG
	      fprintf (stderr, "netbuf: nread = %d\n", nread);
	      hexdump (stderr, nbuf, 0, nread);
#endif

	      if (!strncmp (nbuf, "Password", 8))
	      {
		 ttyb.sg_flags &= ~ECHO;
		 stty (0, &ttyb);
		 gotpass = 1;
	      }
	      write (fd, nbuf, nread);
	   }
	}
	check_done();

	if (needprompt)
	{
	    write (2, prompt, strlen(prompt));
	    needprompt = 0;
	}
	check_done();

	gtty (0, &ttyb);
	if (ttyb.sg_flags & DAVAIL)
	{
	   work = 1;
	   nread = read (0, nbuf, sizeof (nbuf));
	   if (gotpass)
	   {
	      ttyb.sg_flags = savflags;
	      stty (0, &ttyb);
	      gotpass = 0;
	   }
	   process(nbuf, nread, connp);
	}
	if (!work) sleep (1);
    }
}

/* ------------------------------------------------------------------ */

static void 
sigcatcher (sig)
int sig;
{
   NETCONN *NP;

   for (NP = NetConP; NP; NP->NextConP)
      close (NP->NetFD);
   exit (0);
}

main(argc, argv)
    int argc;
    char *argv[];
{
   escape = '^';
   needprompt = 0;
   usercr = 1;    /* Follow CR with LF on input */
   strcpy (prompt, "* ");

   option(0, OPT_ECHO);    /* Refuse remote echo */
   option(0, OPT_SUPPRESS_GA);/* And suppress-go-ahead */
   fsocket = SERVER_SOCKET;

   signal(SIGQUIT, sigcatcher);
   if (argc > 1)
	znetopen(1, argc, argv);

   needprompt = 1;
   from_user (0);
}
