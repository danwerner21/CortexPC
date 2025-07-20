#include <stdio.h>
#include <sys/stat.h>

#define	NMOUNT	16
#define	NAMSIZ	32

extern int errno;
extern char *sys_errlist[];

struct mtab {
	char	file[NAMSIZ];
	char	spec[NAMSIZ];
} mtab[NMOUNT];

int
main(argc, argv)
	char **argv;
{
	register int ro;
	register struct mtab *mp;
	register char *np;
	int n, mf;
	struct stat stbuf;

	mf = open("/etc/mtab", 0);
	read(mf, mtab, sizeof(mtab));
	close(mf);

	if (argc==1) {
		for (mp = mtab; mp < &mtab[NMOUNT]; mp++)
			if (mp->file[0])
				printf("/dev/%s on %s\n", mp->spec, mp->file);
		return 0;
	}

	if(argc < 3) {
		fprintf(stderr, "usage: %s device dir [ro]\n", argv[0]);
		return 1;
	}

	ro = 0;
	if(argc > 3)
		ro++;

	if (stat(argv[1], &stbuf) < 0) {
		fprintf(stderr, "Device %s: %s\n",
		   	   argv[1], sys_errlist[errno]);
		return 1;
	}
	if (!S_ISBLK(stbuf.st_mode)) {
		fprintf(stderr, "Device %s: not a block device\n",
		   	   argv[1]);
		return 1;
	}

	if (stat(argv[2], &stbuf) < 0) {
		fprintf(stderr, "Mount point %s: %s\n",
		   	   argv[2], sys_errlist[errno]);
		return 1;
	}
	if (!S_ISDIR(stbuf.st_mode)) {
		fprintf(stderr, "Mount point %s: not a directory\n",
		   	   argv[2]);
		return 1;
	}

	if (mount(argv[1], argv[2], ro) < 0) {
		fprintf(stderr, "Cannot mount %s: %s\n",
		   	   argv[1], sys_errlist[errno]);
		return 1;
	}
	np = argv[1];
	while(*np++)
		;
	np--;
	while(*--np == '/')
		*np = '\0';
	while(np > argv[1] && *--np != '/')
		;
	if(*np == '/')
		np++;
	argv[1] = np;
	for (mp = mtab; mp < &mtab[NMOUNT]; mp++) {
		if (mp->file[0] == 0) {
			for (np = mp->spec; np < &mp->spec[NAMSIZ-1];)
				if ((*np++ = *argv[1]++) == 0)
					argv[1]--;
			for (np = mp->file; np < &mp->file[NAMSIZ-1];)
				if ((*np++ = *argv[2]++) == 0)
					argv[2]--;
			mp = &mtab[NMOUNT];
			while ((--mp)->file[0] == 0);
			mf = creat("/etc/mtab", 0644);
			write(mf, mtab, (mp-mtab+1)*2*NAMSIZ);
			close(mf);
			return 0;
		}
	}
}
