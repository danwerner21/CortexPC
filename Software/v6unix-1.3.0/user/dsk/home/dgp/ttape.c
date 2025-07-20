#include <stdio.h>

static char buf[10240];

int
main (argc, argv)
char **argv;
{
   int rdwr;
   int fd;
   int mode;
   int len;

   rdwr = 1;
   mode = 0;
   if (argc != 2) 
   {
   USAGE:
      fprintf (stderr, "usage: ttape r|w\n");
      return 1;
   }
   if (argv[1][0] == 'r')
   {
      mode = 0;
      rdwr = 1;
   }
   else if (argv[1][0] == 'w')
   {
      mode = 1;
      rdwr = 0;
      for (len = 0; len < 512; len++)
         buf[len] = len & 0xFF;
   }
   else goto USAGE;

   if ((fd = open ("/dev/mt0", mode)) < 0)
   {
      perror ("Can't open /dev/mt0");
      return 1;
   }

   if (rdwr)
   {
      if ((len = read (fd, buf, 10240)) < 0)
      {
	 perror ("Can't read /dev/mt0");
	 return 1;
      }
   }
   else
   {
      if ((len = write (fd, buf, 512)) < 0)
      {
	 perror ("Can't write /dev/mt0");
	 return 1;
      }
   }
   printf ("len = %d\n", len);

   if (rdwr)
   {
      if ((len = read (fd, buf, 10240)) < 0)
      {
	 perror ("Can't read /dev/mt0");
	 return 1;
      }
   }
   else
   {
      if ((len = write (fd, buf, 512)) < 0)
      {
	 perror ("Can't write /dev/mt0");
	 return 1;
      }
   }
   printf ("len = %d\n", len);

   close(fd);
   return 0;
}
