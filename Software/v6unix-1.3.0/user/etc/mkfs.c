#include <unistd.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/filsys.h>
#include <sys/ino.h>

static struct filsys filsys;
static time_t	utime;
static int	fin;
static int	fsi;
static int	fso;
static char	*charp;
static int	buf[256];
static char	string[50];
static char	*fsys;
static char	*proto;
static int	f_n	= 1;
static int	f_m	= 1;
static int	par_number;
static int	in_number;

getch()
{
	char c[2];
	if(charp)
		return(*charp++);
	read (fin, &c, 1);
	return(c[0]);
}

getstr()
{
	int i, c;

loop:
	switch(c=getch()) {

	case ' ':
	case '\t':
	case '\n':
		goto loop;

	case '\0':
		printf("EOF\n");
		exit();

	case ':':
		while(getch() != '\n');
		goto loop;

	}
	i = 0;

	do {
		string[i++] = c;
		c = getch();
	} while(c!=' '&&c!='\t'&&c!='\n'&&c!='\0');
	string[i] = '\0';
}

getnum()
{
	int n, i;

	getstr();
	n = 0;
	i = 0;
	for(i=0; string[i]!='\0'; i++) {
		if(string[i]<'0' || string[i]>'9') {
			printf("%s: bad number\n", string);
			exit();
		}
		n = n*10 + string[i] - '0';
	}
	return(n);
}

gmode(c, s, m0, m1, m2, m3)
char c, *s;
{
	int i;

	for(i=0; s[i]!='\0'; i++)
		if(c == s[i])
			return((&m0)[i]);
	if(c == '-')
		return(0);
	printf("%c/%s: bad mode\n", c, string);
	exit();
}

rdfs(bno, bf)
{
	int n;
	long bd;

	bd = bno;
	lseek (fsi, bd<<BSHIFT, 0);
	n = read(fsi, bf, 512);
#ifdef DEBUGDISK
	printf ("rdfs: bno = %d, cnt = %d\n", bno, n);
#endif
	if(n != 512) {
		printf("read error: %u\n", bno);
		exit();
	}
}

wtfs(bno, bf)
{
	int n;
	long bd;

	bd = bno;
	lseek (fso, bd<<BSHIFT, 0);
	n = write(fso, bf, 512);
#ifdef DEBUGDISK
	printf ("wtfs: bno = %u, cnt = %d\n", bno, n);
#endif
	if(n != 512) {
		printf("write error: %u\n", bno);
		exit();
	}
}

sizeit (bno, bf)
   unsigned int bno;
{
	int n;
	unsigned int lbno;
	long bd;

	lbno = bno;
	while (bno) {
		bd = bno;
#ifdef DEBUGSIZE
		printf ("bno-0 = %u\n", bno);
#endif
		lseek (fso, bd<<BSHIFT, 0);
		n = write(fso, bf, 512);
		if(n == 512) {
			break;
		}
		lbno = bno;
		bno >>= 1;
	}
#ifdef DEBUGSIZE
	printf ("bno = %u, lbno = %u\n", bno, lbno);
#endif
	while (bno < lbno) {
		bd = bno;
		lseek (fso, bd<<BSHIFT, 0);
		n = write(fso, bf, 512);
		if(n != 512) {
			bno--;
			break;
		}
		bno += 1024;
	}
#ifdef DEBUGSIZE
	printf ("bno-1 = %u\n", bno);
#endif
	while (bno < lbno) {
		bd = bno;
		lseek (fso, bd<<BSHIFT, 0);
		n = write(fso, bf, 512);
		if(n == 512) {
			break;
		}
		bno--;
	}
#ifdef DEBUGSIZE
	printf ("bno = %u\n", bno);
#endif
	return (bno);
}

alloc()
{
	int bno, i;

#ifdef DEBUG
	printf ("alloc: \n");
#endif
	filsys.s_nfree--;
	bno = filsys.s_free[filsys.s_nfree];
	filsys.s_free[filsys.s_nfree] = 0;
	if(bno == 0) {
		printf("out of free space\n");
		exit();
	}
	if(filsys.s_nfree <= 0) {
		rdfs(bno, buf);
		filsys.s_nfree = buf[0];
		for(i=0; i<100; i++)
			filsys.s_free[i] = buf[i+1];
	}
	return(bno);
}

ifree(bno)
{
	int i;

#ifdef DEBUGIFREE
	printf ("ifree: bno = %u\n", bno);
#endif
	if(filsys.s_nfree >= 100) {
		buf[0] = filsys.s_nfree;
		for(i=0; i<100; i++)
			buf[i+1] = filsys.s_free[i];
		wtfs(bno, buf);
		filsys.s_nfree = 0;
	}
	filsys.s_free[filsys.s_nfree] = bno;
	filsys.s_nfree++;
}

newblk(adbc, db, aibc, ib)
int *adbc, *db, *aibc, *ib;
{
	int bno, i;

#ifdef DEBUG
	printf ("newblk: \n");
#endif
	bno = alloc();
	wtfs(bno, db);
	for(i=0; i<256; i++)
		db[i] = 0;
	*adbc = 0;
	ib[*aibc] = bno;
	(*aibc)++;
	if(*aibc >= 256) {
		printf("indirect block full\n");
		exit();
	}
}

entry(ino, str, adbc, db, aibc, ib)
char *str;
int *adbc, *db, *aibc, *ib;
{
	char *s;
	int i;

#ifdef DEBUG
	printf ("entry: ino = %u, str = %s\n", ino, str);
#endif
	db[*adbc] = ino;
	(*adbc)++;
	s = (char *)&db[*adbc];
	for(i=0; i<14; i++) {
		*s++ = *str;
		if(*str != '\0')
			str++;
	}
	*adbc += 7;
	if(*adbc >= 256)
		newblk(adbc, db, aibc, ib);
}

bflist()
{
	char flg[100], adr[100];
	register i, j;
	char *low, *high;

#ifdef DEBUG
	printf ("bflist: f_n = %d\n", f_n);
#endif
	if(f_n > 100)
		f_n = 100;
	for(i=0; i<f_n; i++)
		flg[i] = 0;
	i = 0;
	for(j=0; j<f_n; j++) {
		while(flg[i])
			i = (i+1)%f_n;
		adr[j] = i;
		flg[i]++;
		i = (i+f_m)%f_n;
	}
#ifdef DEBUG
	for(j=0; j<f_n; j++) {
		printf ("j:%d, adr = %u, flg = %u\n", j, adr[j], flg[j]);
	}
#endif

	high = (char *)filsys.s_fsize-1;
	low = (char *)filsys.s_isize+2;
#ifdef DEBUG
	printf ("   low = %u, high = %u\n", (int)low, (int)high);
#endif
	ifree(0);
	for(i=(int)high; lrem(0,i+1,f_n); i--) {
#ifdef DEBUG
		printf ("   lrem = %d\n", lrem(0,i+1,f_n));
#endif
		if(i < low)
			break;
		ifree(i);
	}
#ifdef DEBUG
	printf ("   i1 = %u\n", i);
#endif
	for(; i >= low+f_n; i -= f_n)
		for(j=0; j<f_n; j++)
			ifree(i-adr[j]);
#ifdef DEBUG
	printf ("   i2 = %u\n", i);
#endif
	for(;i >= low; i--)
		ifree(i);
}

cfile(par)
struct inode *par;
{
	struct inode in;
	int db[256], ib[256];
	int dbc, ibc;
	static ino;
	int i, f, *p1, *p2;

	/*
	 * get mode, uid and gid
	 */

#ifdef DEBUG
	printf ("cfile: par = >%x\n", par);
#endif
	getstr();
	in.i_mode = IALLOC;
	in.i_mode |= gmode(string[0], "bcd", IFBLK, IFCHR, IFDIR);
	in.i_mode |= gmode(string[1], "u", ISUID);
	in.i_mode |= gmode(string[2], "g", ISGID);
	for(i=3; i<6; i++) {
		if(string[i]<'0' || string[i]>'7') {
			printf("%c/%s: bad digit\n", string[i], string);
			exit();
		}
		in.i_mode |= (string[i]-'0')<<(15-3*i);
	}
	in.i_uid = getnum();
	in.i_gid = getnum();

	/*
	 * general initialization prior to
	 * switching on format
	 */

	ino++;
	in_number = ino;
	if(ldiv(0, ino, 16) > filsys.s_isize) {
		printf("too many inodes\n");
		exit();
	}
	in.i_nlink = 1;
	in.i_size0 = 0;
	in.i_size1 = 0;
	for(i=0; i<8; i++)
		in.i_addr[i] = 0;
	for(i=0; i<256; i++) {
		db[i] = 0;
		ib[i] = 0;
	}
	if(par == 0) {
		par = &in;
		in.i_nlink--;
	}
	dbc = 0;
	ibc = 0;
	switch(in.i_mode&IFMT) {

	case 0:
		/*
		 * regular file
		 * contents is a file name
		 */

		getstr();
		f = open(string, 0);
		if(f < 0) {
			printf("%s: cannot open\n", string);
			break;
		}
		while((i=read(f, db, 512)) > 0) {
			in.i_size1 += i;
			newblk(&dbc, db, &ibc, ib);
		}
		close(f);
		break;

	case IFBLK:
	case IFCHR:
		/*
		 * special file
		 * content is maj/min types
		 */

		in.i_addr[0] = getnum()<<8;
		in.i_addr[0] |= getnum();
		break;

	case IFDIR:
		/*
		 * directory
		 * put in extra links
		 * call recursively until
		 * name of "$" found
		 */

		par->i_nlink++;
		entry(par_number, "..", &dbc, db, &ibc, ib);
		in.i_nlink++;
		entry(in_number, ".", &dbc, db, &ibc, ib);
		in.i_size1 = (char *)32;
		for(;;) {
			getstr();
			if(string[0]=='$' && string[1]=='\0')
				break;
			entry(ino+1, string, &dbc, db, &ibc, ib);
			in.i_size1 += 16;
			cfile(&in);
		}
		break;
	}
	if(dbc != 0)
		newblk(&dbc, db, &ibc, ib);
	if(ibc > 8) {
		in.i_mode |= ILARG;
		dbc = alloc();
		wtfs(dbc, ib);
		in.i_addr[0] = dbc;
	} else
	for(i=0; i<ibc; i++)
		in.i_addr[i] = ib[i];
	in.i_atime = in.i_mtime = utime;
	i = in_number + 31;
	dbc = ldiv(0, i, 16);
	p1 = &buf[lrem(0, i, 16)*16];
	p2 = &in.i_mode;
	rdfs(dbc, buf);
	for(i=0; i<16; i++)
		*p1++ = *p2++;
	wtfs(dbc, buf);
}

main(argc, argv)
char **argv;
{
	int f, n;

	/*
	 * open relevent files
	 */

	f_n = 24;
	f_m = 3;
	time(&utime);
	if(argc > 3) {
	usage:
		printf("usage: mkfs fs [proto|size]\n");
		exit(1);
	}
	if (argc == 1)
		goto usage;
	fsys = argv[1];
	fso = creat(fsys, 0666);
	if(fso < 0) {
		printf("%s: cannot create\n", fsys);
		exit();
	}
	fsi = open(fsys, 0);
	if(fsi < 0) {
		printf("%s: cannot open\n", fsys);
		exit();
	}
	if (argc == 2) {
		fin = -1;
		n = 65535;
		n = sizeit (n, buf);
		goto f0;
	}
	proto = argv[2];
	fin = open(proto, 0);
	if(fin < 0) {
		n = 0;
		for(f=0; proto[f]; f++) {
			if(proto[f]<'0' || proto[f]>'9') {
				printf("%s: cannot open\n", proto);
				exit();
			}
			n = n*10 + proto[f]-'0';
		}
	f0:
		filsys.s_fsize = n;
		filsys.s_isize = ldiv(0, n, 43+ldiv(0, n, 1000));
#ifdef DEBUG
		printf("isize = %u\n", filsys.s_isize);
		if(f_n != 1)
			printf("free list %u/%u\n", f_m, f_n);
#endif
		charp = "d--777 0 0 $ ";
		goto f3;
	}

	/*
	 * get name of boot load program
	 * and read onto block 0
	 */

	getstr();
	if (string[0]) {
		f = open(string, 0);
		if(f < 0) {
			printf("%s: cannot open init\n", string);
			goto f2;
		}
		read(f, buf, 020);
		if(buf[0] != 0407) {
			printf("%s: bad format\n", string);
			goto f1;
		}
		n = buf[1]+buf[2];
		if(n > 512) {
			printf("%s: too big\n", string);
			goto f1;
		}
		read(f, buf, n);
		wtfs(0, buf);

	f1:
		close(f);
	}

	/*
	 * get total disk size
	 * and inode block size
	 */

f2:
	filsys.s_fsize = getnum();
	filsys.s_isize = getnum();

f3:
	if(filsys.s_isize > filsys.s_fsize ||
	   filsys.s_fsize-filsys.s_isize-2 < filsys.s_isize) {
		printf("%u/%u: bad ratio\n", filsys.s_fsize, filsys.s_isize);
		exit();
	}
	bflist();

	/*
	 * initialize files
	 */

	for(n=0; n<256; n++)
		buf[n] = 0;
	for(n=0; n!=filsys.s_isize; n++)
		wtfs(n+2, buf);
	cfile(0);

	/*
	 * write out super block
	 */

	for(n=0; n<256; n++)
		buf[n] = 0;
	filsys.s_time = utime;
	wtfs(1, &filsys);
}
