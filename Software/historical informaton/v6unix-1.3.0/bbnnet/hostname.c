#include <stdio.h>
#include "inet.h"

#include "net.h"
#include "inet.h"

#define HOSTS "/etc/hosts"

static char name[32];

char *
hostname (addr)
unsigned long addr;
{
   FILE *fd;
   char *bp, *ip, *hp;
   int found;
   char buf[120];

   found = 0;
   if ((fd = fopen (HOSTS, "r")) != NULL)
   {
      while (!found && fgets (buf, sizeof(buf), fd))
      {
	 bp = buf;
	 while (*bp && *bp == ' ') bp++;
	 if (*bp == '#' || *bp == '\n') continue;
	 ip = bp;
	 while (*bp && *bp != ' ' && *bp != '\t') bp++;
	 *bp++ = 0;
	 if (addr == inet_addr(ip))
	 {
	    while (*bp && (*bp == ' ' || *bp == '\t')) bp++;
	    hp = bp;
	    while (*bp && (*bp != ' ' || *bp == '\t')) bp++;
	    *bp++ = 0;
	    strcpy (name, hp);
	    found = 1;
	 }
      }
      fclose (fd);
   }
   if (!found)
      strcpy (name, inet_ntoa(addr));
   return name;
}
