#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

extern void hexdump();

int
main (argc, argv)
   int argc;
   char **argv;
{
   int fd, i;
   struct getifstats buffer;

   if ((fd = open ("/dev/net", O_RDWR)) < 0) {
      perror ("Can't open /dev/net");
      return 1;
   }

   if (ioctl (fd, NETIFSTAT, &buffer, sizeof buffer) < 0) {
      perror ("Can't ioctl /dev/net");
      close (fd);
      return 1;
   }

   printf ("NAME     FLAGS   MTU ADDR            OPKT OERR IPKT IERR\n");
   for (i = 0; i < buffer.if_count; i++) {
      int j;
      printf ("%-8.8s ", buffer.ifstats[i].if_name);
      j = 7;
      if (buffer.ifstats[i].if_avail)
      {
	 putchar ('U');
	 j--;
      }
      if (buffer.ifstats[i].if_error)
      {
	 putchar ('E');
	 j--;
      }
      if (buffer.ifstats[i].if_needinit)
      {
	 putchar ('I');
	 j--;
      }
      if (buffer.ifstats[i].if_active)
      {
	 putchar ('A');
	 j--;
      }
      if (buffer.ifstats[i].if_flush)
      {
	 putchar ('F');
	 j--;
      }
      if (buffer.ifstats[i].if_blocked)
      {
	 putchar ('B');
	 j--;
      }
      while (j-- > 0)
	 putchar (' ');
      printf ("%4d ", buffer.ifstats[i].if_mtu);
      printf ("%-15.15s ", inet_ntoa(buffer.ifstats[i].if_addr));
      printf ("%4d ", buffer.ifstats[i].if_opkts);
      printf ("%4d ", buffer.ifstats[i].if_oerrs);
      printf ("%4d ", buffer.ifstats[i].if_ipkts);
      printf ("%4d\n", buffer.ifstats[i].if_ierrs);
   }
   close (fd);
   return 0;
}
