#include <stdio.h>
#include <string.h>

#define	NMOUNT	16
#define	NAMSIZ	32

extern int errno;
extern char *sys_errlist[];

struct mtab {
	char	file[NAMSIZ];
	char	spec[NAMSIZ];
} mtab[NMOUNT];

char *mtabfile = "/etc/mtab";

int
main(argc, argv)
	char **argv;
{
	register struct mtab *mp;
	register char *p1;
	int mf;

	if(argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		return 1;
	}

	if ((mf = open(mtabfile, 0)) < 0) {
		fprintf(stderr, "Cannot open %s: %s\n",
			mtabfile, sys_errlist[errno]);
		return 1;
	}
	read(mf, mtab, sizeof(mtab));
	close (mf);

	sync();
	if (umount(argv[1]) < 0) {
		fprintf(stderr, "Cannot unmount %s: %s\n",
 	   		argv[1], sys_errlist[errno]);
		return 1;
	}

	p1 = argv[1];
	while(*p1++) ;
	p1--;
	while(*--p1 == '/') *p1 = '\0';
	while(p1 > argv[1] && *--p1 != '/') ;
	if(*p1 == '/') p1++;

	for (mp = mtab; mp < &mtab[NMOUNT]; mp++) {
		if (!strcmp (p1, mp->spec)) {
			for (p1 = mp->file; p1 < &mp->file[NAMSIZ*2];)
					*p1++ = 0;
			mp = &mtab[NMOUNT-1];
			for (; (mp >= mtab) && mp->file[0] == 0; mp--);
			mf = creat(mtabfile, 0644);
			write(mf, mtab, (mp-mtab+1)*2*NAMSIZ);
			close (mf);
			return 0;
		}
	}
	fprintf(stderr, "Cannot unmount %s: Not in mount table\n", argv[1]);
	return 1;
}
