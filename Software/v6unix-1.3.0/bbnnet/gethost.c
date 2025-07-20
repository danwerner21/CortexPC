#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "inet.h"

#define HOSTS "/etc/hosts"

unsigned long
gethost (name)
char *name;
{
   FILE *fd;
   char *bp, *ip, *hp, *ep;
   int found;
   int i;
   char buf[120];
   char lclname[64];
   unsigned long ipn;

   ipn = -1L;
   if (name[0] >= '0' && name[0] <= '9')
   {
      ipn = inet_addr (name);
   }
   else
   {
      found = 0;
      strcpy (lclname, name);
      for (bp = lclname; *bp; bp++)
      {
	 if (*bp >= 'A' && *bp <= 'Z')
	    *bp = *bp + 0x20;
      }
      if ((fd = fopen (HOSTS, "r")) != NULL)
      {
	 while (!found && fgets (buf, sizeof(buf), fd))
	 {
	    for (bp = buf; *bp; bp++)
	    {
	       if (*bp >= 'A' && *bp <= 'Z')
	       *bp = *bp + 0x20;
	    }
	    i = strlen(buf);
	    bp = buf;
	    while (*bp && (*bp == ' ' || *bp == '\t' || *bp == '\n')) bp++;
	    if (*bp == '#' || *bp == 0) continue;
	    ip = bp;
	    while (*bp && *bp != ' ' && *bp != '\t' && *bp != '\n') bp++;
	    *bp++ = 0;
	    while (!found && *bp)
	    {
	       while (*bp && (*bp == ' ' || *bp == '\t' || *bp == '\n')) bp++;
	       hp = bp;
	       while (*bp && *bp != ' ' && *bp != '\t' && *bp != '\n') bp++;
	       *bp++ = 0;
	       if (!strcmp (lclname, hp))
	       {
		  ipn = inet_addr (ip);
		  found = 1;
	       }
	    }
	 }
	 fclose (fd);
      }
   }
   return ipn;
}
