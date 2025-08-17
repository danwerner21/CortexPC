#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define	NAMSIZ	32

struct {
	char	file[NAMSIZ];
	char	spec[NAMSIZ];
} mtab;

int
main (int argc, char **argv)
{
	int mf;

	if (argc != 4) {
		fprintf (stderr, "usage: mkmtab mtab dev mnt\n");
		return 1;
	}
	memset (&mtab, 0, sizeof (mtab));
	strcpy (mtab.file, argv[3]);
	strcpy (mtab.spec, argv[2]);
	mf = creat (argv[1], 0644);
	write (mf, &mtab, sizeof(mtab));
	close (mf);
	return 0;
}
