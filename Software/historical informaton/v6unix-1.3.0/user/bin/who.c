/*
 * who
 */

#include <stdlib.h>
#include <utmp.h>

char *ctime();

int	buf[256];

main(argc, argv)
	char **argv;
{
	char *s, *cbuf;
	int n, fi, i;
	int tty;
	struct utmp *p;

	s = "/etc/utmp";
	if(argc == 2)
		s = argv[1];
	fi = open(s, 0);
	if(fi < 0) {
		write("cannot open wtmp\n", 17);
		exit();
	}
	if (argc==3)
		tty = ttyn(0);

loop:
	n = read(fi, buf, 512);
	if(n == 0) {
		if (argc==3)
			write(1, "Nobody.\n", 8);
		exit();
	}

	for(p = (struct utmp *)&buf; (n -= 16)>=0; p++) {
		if (argc==3 && tty!=p->ut_line[1])
			continue;
		if(p->ut_name[0] == '\0' && argc==1)
			continue;
		for(i=0; i<8; i++) {
			if(p->ut_name[i] == '\0')
				p->ut_name[i] = ' ';
			putchar(p->ut_name[i]);
		}
		for(i=0; i<3; i++)
			putchar("tty"[i]);
		putchar(p->ut_line[1]);
		cbuf = ctime(&p->ut_time);
		for(i=3; i<16; i++)
			putchar(cbuf[i]);
		putchar('\n');
		if (argc==3) {
			exit();
		}
	}
	goto loop;
}

