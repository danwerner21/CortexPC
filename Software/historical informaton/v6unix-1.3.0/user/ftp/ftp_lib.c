/*  This package provides the data transmission routines used by both the
    user and server ftp for data connections.  It also provides the routines
    for dealing with performance statistics on the data transmissions.
    It is designed to meet the specifications of RFC 765 (aka IEN 149) - File
    Transfer Protocol by J. Postel, June, 1980.

    The current version supports the following combinations:
	 mode - only stream mode is supported
	 type - ASCII non-print, image, and local (8 bit byte size)
	 structure - file, record (if also in ASCII mode)
 
 The following external routines are defined within this package:
 inittime() - sets up an assortment of timers and counters
 dumptime(logfn) - calls logfn with a printable string of performance
	statistics
 rcvdata(LocalFile, logfn) - receive data from the network
 senddata(LocalFile, logfn) - send data to network
 
 The following externals are tested by these routines:
    Each of the following variables should really be an enumeration type, but
    too much of the rest of the code would have to be changed to support this
    bit of cleanliness for now.

 type -  TYPEA,TYPEAN - ASCII Non-print
	 TYPEAT - ASCII Telnet format effectors (not supported)
	 TYPEAC - ASCII Carriage Control characters (not supported)
	 TYPEEN - EBCDIC Non-print (not supported)
	 TYPEET - EBCDIC Telnet format effectors (not supported)
	 TYPEEC - EBCDIC Carriage control characters (not supported)
         TYPEI - Image mode
         TYPEL - Local mode (only 8 bit byte supported)
 stru -  STRUF - file structure
	 STRUR - record structure
	 STRUP - page structure (not supported)
 mode -  MODES - stream mode
	 MODEB - block mode (not supported)
	 MODEC - compressed mode (not supported)

 abrtxfer - this variable will cause a data transfer to be stopped if it
            becomes true (1) during the transfer.  Intended for use by signal
	    handlers.  It is set false (0) on entry to the data transfer
	    routines.

   Initial coding - Alan G. Nemeth, BBN, May 21, 1981
*/

#include "ftp_lib.h"
#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include "ftp.h"

#ifdef DEBUG
#include "hexdump.h"
#endif

extern struct net_stuff DataParams;

static time_t starttime;
static long   bytesmoved = 0;		/* # of bytes on net connection */
static time_t totaltime = 0;		/* elapsed time of transmission */

#define FTPESC 0377
#define FTPEOR 01
#define FTPEOF 02

#define MYBUFSIZ 256

/*
  inittime() - establishes a base by zeroing a count of total bytes
	       moved, and the totaltime spent moving them.
  timeon() - internal routine.  Starts the clock.
  timeoff() - internal routine.  Stops the clock and adds accumulated
	      time to totaltime.
  dumptime() - provides a formatted string describing the accumulated
	       performance statistics.
*/

inittime ()
{
    bytesmoved = 0;
    totaltime = 0;
}

static  timeon ()
{
    starttime = time (NULL);
}

static void
timeoff ()
{
    time_t endtime;

    endtime = time (NULL);
    totaltime += endtime - starttime;
}

char *
dumptime ()
{
    static char msgbuf[128];

    msgbuf[0] = '\0';
    if (bytesmoved > 0 && totaltime != 0)
	sprintf (msgbuf,
		"Transferred %ld bytes in %ld seconds ( %ld bytes/sec )\n",
		bytesmoved,
		(long)totaltime,
		bytesmoved / (long)totaltime);
    else
	if (bytesmoved > 0)
	    sprintf (msgbuf,
		    "Transferred %ld bytes in %ld seconds\n",
		    bytesmoved,
		    (long)totaltime);
    return (msgbuf);
}


/*
  rcvdata (file) is used to receive data from the
  network and write it into a file.  When entered, the network file and
  local file are already open and hooked up.  Upon completion, the files
  remain connected and the user of this routine should see to closing
  them.  

  rcvdata returns 0 if the transfer completes entirely successfully and
  -1 in case of any difficulty at all. 
*/

int
rcvdata (file)
FILE   *file;
{
    register int cnt;
    int escflg, retflg;
    char buf[MYBUFSIZ];
    register char *ptr;
    register char c;
    char msgbuf[128];

    errno = 0;
    escflg = retflg = 0;
    abrtxfer = 0;
    timeon ();
    switch (type)
    {
	case TYPEL: 
	case TYPEI: 
	    {
		if (mode != MODES || stru != STRUF)
		{
		    timeoff ();
		    logfn (
			 "554 Unimplemented mode/type/structure combination\n");
		    return (-1);
		}
		while ((cnt = net_read (&DataParams, buf, MYBUFSIZ)) > 0)
		{
		    bytesmoved += cnt;
		    if (fwrite (buf, 1, cnt, file) != cnt)
		    {
			timeoff ();
			sprintf (msgbuf, "552 Write failed: %s\n", errmsg (0));
			logfn (msgbuf);
			return (-1);
		    }
		    if (abrtxfer)
		    {
			timeoff ();
			return (-1);
		    }
		}
		timeoff ();
		if (cnt == 0)
		    return (0);
		else
		{
		    sprintf (msgbuf, "426 Data connection failure: %s\n",
			    errmsg (0));
		    logfn (msgbuf);
		    return (-1);
		}
	    }

	case TYPEA: 
	    {
		if ((mode != MODES) ||
			((stru != STRUF) && (stru != STRUR)))
		{
		    timeoff ();
		    logfn (
			 "554 Unimplemented mode/type/structure combination\n");
		    return (-1);
		}

		for (;;)
		{
		    if (abrtxfer)
		    {
			timeoff ();
			return (-1);
		    }
		    if (ferror (file))
		    {
			timeoff ();
			sprintf (msgbuf, "552 Write failed: %s\n",
				errmsg (0));
			logfn (msgbuf);
			return (-1);
		    }
		    cnt = net_read (&DataParams, buf, MYBUFSIZ);
		    if (cnt <= 0)
			break;
		    bytesmoved += cnt;
		    for (ptr = buf; ptr < buf + cnt; ptr++)
			switch (c = *ptr)
			{
			    case '\r': 
				if (escflg)
				{
				    putc (FTPESC, file);
				    escflg = 0;
				}
				if (retflg++)
				    putc (c, file);
				continue;

			    case '\n': 
				retflg = 0;
				putc (c, file);
				continue;

			    case '\0': 
				if (retflg)
				{
				    retflg = 0;
				    putc ('\r', file);
				}
				else
				    putc (c, file);
				continue;

			    case FTPESC: 
				if (retflg)
				{
				    putc ('\r', file);
				    retflg = 0;
				}
				if (stru != STRUR || escflg++)
				    putc (c, file);
				continue;

			    case FTPEOR: 
				if (escflg)
				{
				    putc ('\n', file);
				    escflg = 0;
				}
				else
				    putc (c, file);
				continue;

			    case FTPEOF: 
				if (escflg)
				{
				    timeoff ();
				    return (0);
				}
				else
				    putc (c, file);
				continue;

			    default: 
				if (retflg)
				{
				    putc ('\r', file);
				    retflg = 0;
				}
				if (escflg)
				{
				    putc (FTPESC, file);
				    escflg = 0;
				}
				putc (c, file);
				continue;
			}
		}
		timeoff ();
		if (cnt == 0)
		    return (0);
		else
		{
		    sprintf (msgbuf, "426 Data connection failure: %s\n",
			    errmsg (0));
		    logfn (msgbuf);
		    return (-1);
		}
	    }

	default: 
	    timeoff ();
	    logfn ("554 Unimplemented mode/type/structure combination\n");
	    return (-1);
    }
}

static int
dat_write(buff, count)
char *buff;
int count;
{
    int i;

    errno = 0;

#ifdef DEBUG
    printf ("dat_write: count = %d\n", count);
    hexdump (stdout, buff, 0, count);
#endif

    while ((i = net_write (&DataParams, buff, count)) < 0)
    {
	 printf ("   i = %d, errno = %d\n", i, errno);
	 if ((errno == ENETSTAT) &&
	     ((ioctl(DataParams.fds, NETGETS, &(DataParams.ns))),
	       (DataParams.ns.n_state & URXTIMO)))
	 {
	     errno = 0;
	     fprintf (stderr, "!");
	     fflush (stderr);
	 }
	 else break;
    }
    return i;
}

static char *
myputc (ptr, buf, chr)
char *ptr;
char *buf;
{
    *ptr++ = chr;
    if (ptr == &buf[MYBUFSIZ])
    {
	if (dat_write (buf, MYBUFSIZ) != MYBUFSIZ)
		return NULL;
	bytesmoved += MYBUFSIZ;
	ptr = buf;
    }
    return ptr;
}

#define CHECK(e) { if ((e) == NULL) goto finishup; }

/*
  senddata(file)  is the reverse of rcvdata - it copies
  data from the local file to the network.  Otherwise, it's calling
  conventions are identical with rcvdata.
*/
int
senddata (file)
register FILE  *file;
{
    register char *ptr;
    register int cnt;
    register int c;
    char buf[MYBUFSIZ];
    char msgbuf[128];

    errno = 0;
    abrtxfer = 0;
    timeon ();
    switch (type)
    {
    case TYPEL: 
    case TYPEI: 
	    {
		if (mode != MODES || stru != STRUF)
		{
		    timeoff ();
		    logfn (
			 "554 Unimplemented mode/type/structure combination\n");
		    return (-1);
		}
		while ((cnt = fread (buf, 1, MYBUFSIZ, file)) > 0)
		{
		    bytesmoved += cnt;
		    if (dat_write (buf, cnt) != cnt)
		    {
			timeoff ();
			sprintf (msgbuf,
				"426 Data connection failure: %s\n",
				errmsg (0));
			logfn (msgbuf);
			return (-1);
		    }
		    if (abrtxfer)
		    {
			timeoff ();
			return (-1);
		    }
		}
		timeoff ();
		if (cnt == 0)
		    return (0);
		else
		{
		    sprintf (msgbuf, "552 Read of file failed: %s\n",
			    errmsg (0));
		    logfn (msgbuf);
		    return (-1);
		}
	    }

     case TYPEA: 
	    {
		if ((mode != MODES) ||
			((stru != STRUF) && (stru != STRUR)))
		{
		    timeoff ();
		    logfn (
			 "554 Unimplemented mode/type/structure combination\n");
		    return (-1);
		}

	    ptr = buf;
	    cnt = 0;
	    for (;;)
	    {
		if (abrtxfer)
		{
		    timeoff ();
		    return (-1);
		}
		if (ferror (file))
		{
		    timeoff ();
		    sprintf (msgbuf, "552 Read failed: %s\n", errmsg (0));
		    logfn (msgbuf);
		    return (-1);
		}

		switch (c = getc (file))
		{
		    case '\r': 
			CHECK(ptr = myputc (ptr, buf, c));
			CHECK(ptr = myputc (ptr, buf, '\0'));
			continue;

		    case '\n': 
			if (stru == STRUR)
			{
			    CHECK(ptr = myputc (ptr, buf, FTPESC));
			    CHECK(ptr = myputc (ptr, buf, FTPEOR));
			}
			else
			    CHECK(ptr = myputc (ptr, buf, c));
			continue;

		    case (FTPESC & 0377): 
			if (stru == STRUR)
			    CHECK(ptr = myputc (ptr, buf, FTPESC));
			CHECK(ptr = myputc (ptr, buf, FTPESC));
			continue;

		    case EOF: 
			if (stru == STRUR)
			{
			    CHECK(ptr = myputc (ptr, buf, FTPESC));
			    CHECK(ptr = myputc (ptr, buf, FTPEOF));
			}
			if ((cnt = dat_write (buf, ptr - buf))
				!= (ptr - buf))
			    goto finishup;
			bytesmoved += ptr - buf;
			timeoff ();
			return (0);

		    default: 
			CHECK(ptr = myputc (ptr, buf, c));
			continue;
		}
	    }
       finishup: 
	    timeoff ();
	    if (cnt == 0)
		return (0);
	    else
	    {
		sprintf (msgbuf, "426 Data connection failure: %s\n",
			errmsg (0));
		logfn (msgbuf);
		return (-1);
	    }
       }

   default: 
       timeoff ();
       logfn ("554 Unimplemented mode/type/structure combination\n");
       return (-1);
    }
}

