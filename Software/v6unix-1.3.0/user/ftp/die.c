#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "ftp.h"

extern struct net_stuff NetParams;
extern struct net_stuff DataParams;

struct {int pid; FILE *file;} diepid = { 0, NULL };

diequit()
{
   die(SIGQUIT, "killed by quit signal\n");
}

dieother()
{
   die(SIGTERM, NULL); /* this is the ordinary state, so don't announce it*/
}

dieinit(other, errfd)
int other;
FILE *errfd;
{
    diepid.pid = other;      /* id of the process to kill */
    diepid.file = errfd;      /* fd to print messages on */
}

/* VARARGS */
die(status, s1, s2, s3, s4, s5)
int status;         
char *s1, *s2, *s3, *s4, *s5;
{
   if (s1 != NULL) fprintf(diepid.file, s1, s2, s3, s4, s5);
   if (NetParams.fds)
      close (NetParams.fds);
   if (DataParams.fds)
      close (DataParams.fds);
   exit(status);
}
