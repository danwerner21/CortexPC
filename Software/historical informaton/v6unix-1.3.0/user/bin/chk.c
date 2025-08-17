#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int sum;
char buf[512];

void
check(filename)
	char *filename;
{
	int f,n,i;
	unsigned short sum;
	static int header_printed;

	f = open(filename, 0);
	if (f < 0) {
		printf("%s: not found\n", filename);
		return;
	}
	if (! header_printed) {
		printf("  check filename\n");
		header_printed = 1;
	}
	sum = 0;
	while(1) {
		if ((n = read(f, buf, 512))<0 ) {
			printf("%s: read error\n", filename);
			close(f);
			return;
		}
		if (!n) break;
		for(i=0; i<n; i++) {
			sum += buf[i];
		}
	}
	printf(" 0x%04x %s\n", sum, filename);
	close(f);
}

int
main(argc, argv)
	char **argv;
{
	register char *cp;
	int nfiles;

	sum = 0;
	nfiles = 0;
	while(--argc) {
		++argv;
		if (**argv != '-') {
			check (*argv);
			++nfiles;
			continue;
		}
		for (cp = *argv+1; *cp; cp++) {
			switch (*cp) {
			default:
				fprintf (stderr, "Usage: check file...\n");
				return (1);
			}
		}
	}
	if (nfiles == 0)
		check ("a.out");
	return 0;
}

