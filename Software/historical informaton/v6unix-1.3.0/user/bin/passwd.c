/*
 * enter a password in the password file
 * this program should be suid with owner
 * with an owner with write permission on /etc/passwd
 */
#include <stdio.h>

char	*tfile = "/tmp/ptmp";
char	*pfile = "/etc/passwd";
FILE	*tf;
FILE	*pf;
char	buf[128];

char* crypt();

main(argc, argv)
	char *argv[];
{
	register int u, c;
	register char *p;

	if(argc != 3) {
		write(2, "Usage: passwd user password\n", 28);
		goto bex;
	}
	signal(1, 1);
	signal(2, 1);
	signal(3, 1);

	if(stat(tfile, buf) >= 0) {
		write(2, "Temporary file busy -- try again\n", 33);
		goto bex;
	}
	tf = fopen(tfile, "w");
	if(!tf) {
		write(2, "Cannot create temporary file\n", 29);
		goto bex;
	}
	pf = fopen(pfile, "r");
	if(!pf) {
		write(2, "Cannot open /etc/passwd\n", 24);
		goto out;
	}
	goto l1;

/*
 * skip to beginning of next line
 */

skip:
	while(c != '\n') {
		if(c < 0)
			goto ill;
		c = getc(pf);
		putc(c, tf);
	}

/*
 * compare user names
 */

l1:
	c = getc(pf);
	if(c < 0) {
		write(2, "User name not found in password file\n", 37);
		goto out;
	}
	putc(c, tf);
	p = argv[1];
	while(c != ':') {
		if(*p++ != c)
			goto skip;
		c = getc(pf);
		putc(c, tf);
	}
	if(*p)
		goto skip;
/*
 * skip old password
 */
	do {
		c = getc(pf);
		if(c < 0)
			goto ill;
	} while(c != ':');

/*
 * copy in new password
 */
	p = argv[2];
	for(c=0; c<9; c++)
		if(*p++ == 0)
			break;
	*--p = 0;
	if(p != argv[2])
		p = crypt(argv[2]);
	while(*p)
		putc(*p++, tf);
	putc(':', tf);

/*
 * validate uid
 */

	u = 0;
	do {
		c = getc(pf);
		putc(c, tf);
		if(c >= '0' && c <= '9')
			u = u*10 + c-'0';
		if(c < 0)
			goto ill;
	} while(c != ':');
	c = getuid() & 0377;
	if(c != 0 && c != u) {
		write(2, "Permission denied\n", 18);
		goto out;
	}

/*
 * copy out and back
 */

	for(;;) {
		c = getc(pf);
		if(c < 0) {
			fclose(pf);
			fclose(tf);
			tf = fopen(tfile, "r");
			if(!tf) {
				write(2, "Urk\n", 4);
				goto out;
			}
			pf = fopen(pfile, "w");
			if(!pf) {
				write(2, "Cannot create /etc/passwd\n", 26);
				goto out;
			}
			while((c = getc(tf)) >= 0)
				putc(c, pf);
			unlink(tfile);
			exit(0);
		}
		putc(c, tf);
	}

ill:
	write(2, "Password file illformed\n", 24);

out:
	unlink(tfile);

bex:
	exit(1);
}
