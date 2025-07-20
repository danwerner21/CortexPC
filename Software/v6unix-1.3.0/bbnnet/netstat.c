#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

static unsigned long
nettoip (ip)
unsigned long ip;
{
   if (ip != 0L)
   {
      while ((ip & 0xFF000000L) == 0L)
	 ip <<= 8;
   }
   return ip;
}

int
main (argc, argv)
     int argc;
     char *argv[];
{
   int fd, cnt;
   register int i, j;
   char all, opts, rstats, istats, pstats, mstats, cstats;
   struct getallstats allstats;

   all = opts = istats = pstats = mstats = rstats = cstats = 0;

   while (argc > 1 && argv[1][0] == '-')
   {
      switch (argv[1][1])
      {

      case 'a':
         cstats++;
         istats++;
         mstats++;
         pstats++;
         rstats++;
	 all++;
         break;

      case 'c':
         cstats++;
         break;

      case 'i':
         istats++;
         opts++;
         break;

      case 'm':
         mstats++;
         opts++;
         break;

      case 'r':
         rstats++;
         opts++;
         break;

      case 's':
         pstats++;
         opts++;
         break;

      default:
       bad:
         printf ("usage: %s [-acimrs] \n", argv[0]);
         return 1;
      }
      argv++;
      argc--;
   }

   if ((fd = open ("/dev/net", O_RDWR)) < 0) {
      perror ("Can't open /dev/net");
      return 1;
   }

   if (ioctl (fd, NETALLSTATS, &allstats, sizeof allstats) < 0) {
      perror ("Can't ioctl /dev/net");
      close (fd);
      return 1;
   }

   if (!opts || cstats)
   {
      printf (
"PID  MODE  STAT FOREIGN HOST    FPRT  LPRT  TCB   SB RB SA RA TS\n");

      for (cnt = 0; cnt < allstats.ucbs.count; cnt++)
      {
         struct ifucb *up;

	 up = &allstats.ucbs.uinfo[cnt];

	 printf ("%4d ", up->proc);

	 i = 5;
	 if (up->flags & UTCP)
	    putchar ('T');
	 else if (up->flags & UIP)
	    putchar ('I');
	 else if (up->flags & URAW)
	    putchar ('R');
	 else if (up->flags & UCTL)
	    putchar ('C');
	 else if (up->flags & UUDP)
	    putchar ('U');
	 else
	    putchar ('?');

	 if (up->flags & UEOL)
	 {
	    putchar ('P');
	    i--;
	 }
	 if (up->flags & UURG)
	 {
	    putchar ('U');
	    i--;
	 }
	 if (up->flags & RAWERR)
	 {
	    putchar ('E');
	    i--;
	 }
	 if (up->flags & RAWASIS)
	 {
	    putchar ('A');
	    i--;
	 }
	 else if (up->flags & RAWVER)
	 {
	    putchar ('V');
	    i--;
	 }
	 else if (up->flags & RAWCOMP)
	 {
	    putchar ('S');
	    i--;
	 }
	 if (up->flags & UDEBUG)
	 {
	    putchar ('D');
	    i--;
	 }
	 if (up->flags & ULISTEN)
	 {
	    putchar ('L');
	    i--;
	 }
	 while (i-- > 0)
	    putchar (' ');

	 printf ("%04o ", up->state);

	 printf ("%-15.15s ", inet_ntoa(up->host));

	 printf ("%5u %5u %04X %2d %2d %2d %2d %2d\n",
		 up->fport, up->lport, up->tcb, up->ssize,
		 up->rsize, up->snd, up->rcv, up->tstate);
      }
      if (all) putchar ('\n');
   }

   if (rstats)
   {
      printf ("LOCAL NET       FOREIGN NET     GATEWAY         FLAGS\n");
      for (cnt = 0; cnt < allstats.routes.count; cnt++)
      {
	 struct ifroute *rp;

	 rp = &allstats.routes.rinfo[cnt];
         printf ("%-15.15s ", inet_ntoa (nettoip (rp->lnet)));
         printf ("%-15.15s ", inet_ntoa (nettoip (rp->fnet)));
         printf ("%-15.15s ", inet_ntoa (rp->local));
         printf ("%d\n", rp->flags);
      }
      if (all) putchar ('\n');
   }

   if (istats)
   {

      printf ("NAME     FLAGS   MTU ADDR            OPKT OERR IPKT IERR\n");

      for (cnt = 0; cnt < allstats.ifstats.count; cnt++)
      {
	 struct ifstat *ip;

	 ip = &allstats.ifstats.ifinfo[cnt];
	 printf ("%-8.8s ", ip->name);
	 i = 7;
         if (ip->avail)
         {
	    putchar ('U');
	    i--;
         }
         if (ip->error)
         {
	    putchar ('E');
	    i--;
         }
         if (ip->needinit)
         {
	    putchar ('I');
	    i--;
         }
         if (ip->active)
         {
	    putchar ('A');
	    i--;
         }
         if (ip->flush)
         {
	    putchar ('F');
	    i--;
         }
         if (ip->blocked)
         {
	    putchar ('B');
	    i--;
         }
         while (i-- > 0)
	    putchar (' ');
	 printf ("%4d ", ip->mtu);
	 printf ("%-15.15s ", inet_ntoa (ip->addr));
	 printf ("%4d ", ip->opkts);
	 printf ("%4d ", ip->oerrs);
	 printf ("%4d ", ip->ipkts);
	 printf ("%4d\n", ip->ierrs);
      }
      if (all) putchar ('\n');
   }

   if (mstats)
   {
      printf ("pages = %d, bufs = %d, free = %d, hiwat = %d, lowat = %d\n",
              allstats.ifmbufs.pages,
              (allstats.ifmbufs.pages << 3),
              allstats.ifmbufs.bufs,
	      allstats.ifmbufs.hiwat,
	      allstats.ifmbufs.lowat);
      if (all) putchar ('\n');
   }

   if (pstats)
   {
      printf ("mem drops = %d\n", allstats.nstats.m_drops);
      printf (
      "ip badsums = %d, tcp badsums = %d, tcp rejects = %d, tcp unacked = %d\n",
          allstats.nstats.ip_badsum, allstats.nstats.t_badsum,
	  allstats.nstats.t_badsegs, allstats.nstats.t_unack);
      printf (
      "icmp drops = %d, icmp badsums = %d, src quenches = %d, redirects = %d\n",
          allstats.nstats.ic_drops, allstats.nstats.ic_badsum,
	  allstats.nstats.ic_quenches, allstats.nstats.ic_redirects);
      printf ("icmp echoes = %d, time exceeded = %d\n",
	  allstats.nstats.ic_echoes, allstats.nstats.ic_timex);
   }
}
