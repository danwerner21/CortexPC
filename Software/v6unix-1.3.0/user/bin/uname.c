#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>

static void
usage ()
{
   fprintf (stderr, "usage: uname [-a | -m | -p | -r | -s | -v] \n");
   exit (1);
}

int
main (argc, argv)
   char **argv;
{
   char *ap;
   int i;
   int sflg, vflg, rflg, pflg, mflg;
   struct utsname uts;

   sflg = vflg = rflg = pflg = mflg = 0;
   if (argc == 1)
      sflg = 1;
   else for (i = 1; i < argc; i++)
   {
      ap = argv[i];
      if (*ap == '-')
      {
	 for (ap++; *ap; ap++)
	    switch (*ap)
	    {
	    case 'a':
	       sflg = vflg = rflg = pflg = mflg = 1;
	       break;
	    case 'm':
	       mflg = 1;
	       break;
	    case 'p':
	       pflg = 1;
	       break;
	    case 'r':
	       rflg = 1;
	       break;
	    case 's':
	       sflg = 1;
	       break;
	    case 'v':
	       vflg = 1;
	       break;
	    default:
	       usage();
	    }
      }
      else
	 usage();
   }
   uname (&uts);

   if (sflg)
      printf ("%s ", uts.sysname);

   if (vflg)
      printf ("%s ", uts.version);

   if (rflg)
      printf ("%s ", uts.release);

   if (mflg)
   {
#ifdef TI990
      printf ("TI-990 ");
#endif
#ifdef TI990_10A
      printf ("TI-990/10A ");
#endif
#ifdef TI990_12
      printf ("TI-990/12 ");
#endif
#ifdef TI_CTX
      printf ("TI-CORTEX ");
#endif
   }

   if (pflg)
      printf ("%s ", uts.machine);
   printf ("\n");
}
