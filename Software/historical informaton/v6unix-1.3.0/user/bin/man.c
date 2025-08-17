#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

char	*page;
int	 section = 1;

void cleanup();
void nroff();

#define MANDIR	"/man"

int
mysystem(s)
	char *s;
{
	int status, pid, w;

	if ((pid = fork()) == 0) {
		execl("/bin/sh", "sh", "-c", s, 0);
		_exit(127);
	}
	while ((w = wait(&status)) != pid && w != -1)
		;
	if (w == -1)
		status = -1;
	return (status);
}

int
manual(sec, name)
	int   sec;
	char *name;
{
	char section = sec + '0';
	char work[100], work2[100], cmdbuf[150];
	struct stat stbuf, stbuf2;
	int last;

	strcpy(work, "manx/");
	strcat(work, name);
	strcat(work, ".x");
	last = strlen(work) - 1;

	work[3] = section;
	work[last] = section;
	work[last+1] = 0;
	if (stat(work, &stbuf) < 0) {
		printf("No entry for '%s' in section %c of the manual.\n",
			name, section);
		return;
	}

	strcpy(work2, "cat");
	strcpy(work2+3, work+3);
	if (stat(work2, &stbuf2) < 0 || stbuf2.st_mtime < stbuf.st_mtime) {
		printf("Reformatting page.  Wait...");
		fflush(stdout);
		unlink(work2);
		sprintf(cmdbuf, "nroff -ma %s | col > %s", work, work2);
		if (mysystem(cmdbuf)) {
			printf(" aborted (sorry)\n");
			cleanup();
			/*NOTREACHED*/
		}
	}
	strcpy(work, work2);
	nroff(work);
}

void
nroff(cp)
	char *cp;
{
	char cmd[BUFSIZ];

	if (cp[0] == 'c')
		sprintf(cmd, "more -u %s", cp);
	else
		sprintf(cmd, "nroff -ma %s | col", cp);
	mysystem(cmd);
}

void
cleanup()
{
	exit(0);
}

main(argc, argv)
	int argc;
	char *argv[];
{

	if (signal(SIGINT, SIG_IGN) == SIG_DFL) {
		signal(SIGINT, cleanup);
		signal(SIGQUIT, cleanup);
		signal(SIGTERM, cleanup);
	}
	if (chdir(MANDIR) < 0) {
		fprintf(stderr, "Can't chdir to %s.\n", MANDIR);
		exit(1);
	}
	if (argc == 1) {
		manual(1, "man");
		exit(0);
	}
	argc--, argv++;
	page = argv[0];
	if (argv[0][0] >= '0' && argv[0][0] <= '9' && argv[0][1] == 0) {
		section = argv[0][0] - '0';
		argc--, argv++;
		if (argc == 0) {
			printf("What page do you want from section %d?\n", section);
			exit(1);
		} else {
			page = argv[0];
		}
	}
	manual(section, page);
	exit(0);
}
