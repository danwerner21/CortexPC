#include <unistd.h>
#include <stdio.h>

static int
number (s)
char *s;
{
	int n, c;

	n = 0;
	while (c = *s++) {
		if (c<'0' || c>'9')
			return(-1);
		n = n*10 + c-'0';
	}
	return(n);
}

main (argc, argv)
int argc;
char **argv;
{
	int m, a, b;

	if (argc != 5) {
		goto usage;
	}
	if (*argv[2] == 'b')
		m = 060666; 
	else if (*argv[2] == 'c')
		m = 020666;
	else
		goto usage;
	a = number (argv[3]);
	if (a < 0)
		goto usage;
	b = number (argv[4]);
	if (b < 0)
		goto usage;
	if (mknod (argv[1], m, (b<<8)|a) < 0)
		perror ("mknod");
	exit (0);

usage:
	printf ("usage: mknod name b/c major minor\n");
	exit (1);
}
