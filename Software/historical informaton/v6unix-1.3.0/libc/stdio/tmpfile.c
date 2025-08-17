/************************************************************************
* tmpfile - Open/create a temporary file.
************************************************************************/

#include <stdio.h>
#include <stdlib.h>

FILE *
tmpfile ()
{
   FILE *fd;
   char path[32];

   strcpy (path, "/tmp/tmXXXXXX");
   if (mktemp (path) == NULL)
      return (NULL);
   fd = fopen (path, "w");
   return (fd);
}
