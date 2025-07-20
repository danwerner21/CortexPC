#include <signal.h>
void
die (sig)
{
   printf ("I'm dying, sig = %d\n", sig);
   exit (sig);
}

main()
{
   int i;
   signal (SIGINT, die);
   signal (SIGKILL, die);
   while (1) {
      i=i+1;
   }
}
