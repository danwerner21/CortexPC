#include <errno.h>
#include "ftp.h"
#include "usr.h"

extern struct net_stuff DataParams, NetParams;
extern char *linptr;
extern char linbuf[LINSIZ];

sndcmd()
{
   register char *p;
   char tmpbuf[120];

   p = linptr;
   *p++ = '\r'; *p++ = '\n'; *p++ = '\0';

   if (net_write(&NetParams, linbuf, strlen(linbuf)) < 0)
   {
      sprintf (tmpbuf, "Can't write command '%s' to net: %s\n",
	       linbuf, errmsg(0));
      die (24, tmpbuf);
   }
}

chekds(gender)
{
   register int fd, *fp;
   register struct con *sp;

   fp = &(DataParams.fds);
   sp = &(DataParams.np);

   if(gender)
   {
      sp->c_sbufs  = 3;
      sp->c_rbufs = 0;
   }
   else
   {
      sp->c_rbufs  = 3;
      sp->c_sbufs = 0;
   }

   sp->c_mode = CONTCP | CONACT;
   sp->c_lport = 0; /*NetParams.ns.n_lport;*/
   sp->c_timeo = 5; /* FTPTIMO;*/
   sp->c_fcon = DataParams.ns.n_fcon;
   sp->c_fport = DataParams.ns.n_fport;
   mkanyhost(sp->c_lcon);
   if (isbadhost(sp->c_fcon))
      exit(1);
   *fp = fd = netopen(sp);
   if (fd < 0)
   {
      if (errno != ENETTIM)
	 printf("Can't open data socket: %s\n", errmsg(0));
      return(1);
   }

   return(0);
}
