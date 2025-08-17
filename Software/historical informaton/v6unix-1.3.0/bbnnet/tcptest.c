#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#ifdef USESOCKET
#include <memory.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/timeb.h>
#else
#include "net.h"
#include "con.h"
#include "inet.h"
#endif
#ifdef TIMERS
#include <sys/timeb.h>
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define mkanyhost(a) (a).s_addr = 0;
#define isbadhost(a) ((a).s_addr==(-1L) ? 1 : 0)
#define	isanyhost(a) ((a).s_addr==0)


#ifdef USESOCKET
struct sockaddr_in raddr;
struct sockaddr_in iraddr;
struct sockaddr_in waddr;
int sfr;
#else
struct con con;
struct netstate outstat;
#endif
#ifdef TIMERS
struct timeb stime, ttime;
float totals, itotals;
#else
long totals, itotals;
#endif
extern int errno;
#ifdef TIMERS
int timer;
#endif
int fr, fw, finis, loop, match, compare;
int await, conopen;
short wport, rport, temp, imp, eol;
char *dir;
char outbuf[1024];
char inbuf[1024];
long bytes, ibytes;

void
dorept(title, totals, bytes, brief)
char *title;
#ifdef TIMERS
float totals;
#else
long totals;
#endif
long bytes;
int brief;
{
	printf("%s: ", title);
#ifdef TIMERS
	if (!brief)
		printf("%ld bytes in %6.3f seconds", bytes, totals);
	if (totals == 0.0)
		putchar('\n');
	else {
		long thru;
		thru = (bytes/totals)*8;
		printf(" %ld bits/sec\n", thru);
	}
#else
	if (!brief)
		printf("%ld bytes\n", bytes);
#endif
}

void
testint(sig)
int sig;
{
	printf("Interrupt...\n");
	if (!conopen)
		exit(0);
	finis++;
}

void
testurg(sig)
int sig;
{
	signal(SIGTERM, testurg);
	printf("Urgent test...\n");
#ifndef USESOCKET
	if (fw)
	   ioctl(fw, NETSETU, 0);
#endif
}

void
urgent(sig)
int sig;
{
	signal(SIGURG, urgent);
	printf("Received urgent data\n.");
}

void
report(sig)
int sig;
{
	signal(SIGQUIT, report);
	dorept(dir, totals, bytes, FALSE);
}


#ifdef TIMERS
void
timerpt(sig)
int sig;
{
	static char title[24];

	signal(SIGALRM, timerpt);
	if (timer > 0)
		alarm(timer);
	sprintf(title, "int %s", dir);
	dorept(title, itotals, ibytes, TRUE);
	sprintf(title, "avg %s", dir);
	dorept(title, totals, bytes, TRUE);
	itotals = 0;
	ibytes = 0;
}
#endif

void
die(sig)
int sig;
{
	if (!conopen)
		exit(0);
	finis++;
}

int
main(argc, argv)
int argc;
char *argv[];
{
	register char last;
	register int count, j, rdcnt;
	int bufsiz, pid, i;
	int fd;
	int t_write, t_read, printit;

	last = 0xff;
	count = -1;
	ibytes = 0L;
	bytes = 0L;
	bufsiz = 512;
	printit = 0;
	fd = -1;
	fw = fr = 0;

#ifdef USESOCKET
	memset ((char *)&raddr, 0, sizeof(raddr));
	raddr.sin_family = AF_INET;
	raddr.sin_addr.s_addr = htonl (INADDR_ANY);

	memset ((char *)&waddr, 0, sizeof(waddr));
	waddr.sin_family = AF_INET;
#else
	con.c_mode = CONTCP | CONACT;
	con.c_sbufs = 0;
	con.c_rbufs = 0;
	con.c_timeo = 0;
	con.c_fcon.s_addr = gethost ("127.0.0.1");
	mkanyhost(con.c_lcon);
	con.c_fport = 0;
	con.c_lport = 0;
#endif
#ifdef TIMERS
	itotals = 0.;
	totals = 0.;
#else
	totals = 0L;
	itotals = 0L;
#endif

	wport = 0x1234;
	rport = 0x5678;
	t_write = 1;
	t_read = 1;
	match = 0;
	finis = 0;
	loop = 0;
	eol = 0;
	compare = 0;
	conopen = 0;
	await = 0;

	while (argc > 1 && argv[1][0] == '-') {
		switch(argv[1][1]){

		case 'r':               /* read test */
			t_write--;
			t_read++;
			break;

		case 'w':               /* write test */
			t_read--;
			t_write++;
			break;

		case 'd':               /* debug connection */
#ifndef USESOCKET
			con.c_mode |= CONDEBUG;
#endif
			break;

		case 'a':               /* await mode */
#ifndef USESOCKET
			con.c_mode &= ~CONACT;
#endif
			await++;
			break;

		case 'h':               /* set foreign host/imp */
#ifdef USESOCKET
			waddr.sin_addr.s_addr = inet_addr(argv[2]);
#else
			con.c_fcon.s_addr = gethost(argv[2]);
#endif
			argv++; argc--;
			break;

		case 'g':
#ifdef USESOCKET
			raddr.sin_addr.s_addr = inet_addr(argv[2]);
#else
			con.c_lcon.s_addr = gethost(argv[2]);
#endif
			argv++; argc--;
			break;

		case 'p':               /* set foreign port */
			sscanf(argv[2], "%d", &wport);
#ifndef USESOCKET
			con.c_fport = wport;
#endif
			argv++; argc--;
			break;

		case 'q':               /* set local port */
			sscanf(argv[2], "%d", &rport);
#ifndef USESOCKET
			con.c_lport = rport;
#endif
			argv++; argc--;
			break;

		case 'x':               /* print after each read/write */
			printit++;
			break;

		case 'b':               /* read/write buffer size */
			sscanf(argv[2], "%d", &bufsiz);
			if (bufsiz > 1024)
				fprintf (stderr, "buffer size > 1024\n");
			argv++; argc--;
			break;

		case 'c':               /* send/rcv buffer allocation */
			sscanf(argv[2], "%d", &temp);
#ifndef USESOCKET
			con.c_sbufs = temp;
			con.c_rbufs = temp;
#endif
			argv++; argc--;
			break;

		case 't':               /* open timeout */
			sscanf(argv[2], "%d", &temp);
#ifndef USESOCKET
			con.c_timeo = temp;
#endif
			argv++; argc--;
			break;

		case 'l':               /* looping test */
			loop++;
			break;

		case 'm':               /* match read/write size mismatch */
			match++;
			break;

		case 'e':               /* send eols */
			eol++;
			break;

		case 'z':               /* comparison test */
			compare++;
			break;

#ifdef TIMERS
		case 'i':		/* report every n seconds */
			timer = 30;
			if (argc > 1) {
				timer = atoi(argv[2]);
				argv++; argc--;
			}
			break;
#endif

		case 'f':               /* output file */
			if (argc > 1) {
				if ((fd = creat(argv[2], 0666)) < 0)
					fprintf (stderr,
						"Cannot create: %s: %s\n",
						argv[2], sys_errlist[errno]);
				argv++; argc--;
			} else
				printf("No output file name given.\n");
			break;

		case '?':
			goto doc;

		default:
			printf("usage: %s [-rwadelmxz?] [-g <netaddr>] [-h <netaddr>] [-p <fport>] [-q <lport>]\n",
				argv[0]);
			printf("\t\t[-b <bufsiz>] [-c <conbufs>] [-t <timeout>] [<count>] [-i <intvl>]\n");
			return 1;
		}
		argv++; argc--;
	}

	if (argc > 1) {
		if (argv[1][0] == '?') {
doc:			printf("TCPTEST OPTIONS:\nopt\tdescription\t\tdefault\n");
			printf("a\tawait mode\t\toff\n");
			printf("b\tset buffer size\t\t512\n");
			printf("c\tset buffer alloc\t1\n");
			printf("d\tdebug mode\t\toff\n");
			printf("e\teol mode\t\toff\n");
			printf("f\tcreate output file\tnull\n");
			printf("g\tset local host\t\tlocal\n");
			printf("h\tset foreign host\tlocal\n");
			printf("i\treport every n secs\t30\n");
			printf("l\tloopback mode\t\toff\n");
			printf("m\trept r/w len mismatch\tno\n");
			printf("p\tset foreign port\t0\n");
			printf("q\tset local port\t\t0\n");
			printf("r\tread test\t\ton\n");
			printf("t\tset open timeout\t30\n");
			printf("w\twrite test\t\ton\n");
			printf("x\tprint after r/w\t\toff\n");
			printf("z\tcompare buffers\t\toff\n");
			return 1;
		}
		count = atoi(argv[1]);
	}

	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, testint);
	signal(SIGQUIT, report);
	signal(SIGURG, urgent);
	signal(SIGTERM, testurg);
	signal(SIGHUP, die);
#ifdef TIMERS
	signal(SIGALRM, timerpt);
#endif

	for (i=0, j=0; i < bufsiz; i++)
		outbuf[i] = j++;

	if (t_read && !t_write)
		goto readit;

	if (t_read && t_write)
		switch (pid = fork()) {

		case -1:
			fprintf (stderr, "cannot fork: %s\n",
				sys_errlist[errno]);

		default:
			break;

readit:		case 0:
			dir = "read";
#ifdef USESOCKET
			raddr.sin_port = htons (rport);
        		if ((sfr = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        			fprintf (stderr, "read socket error: %s\n",
					 sys_errlist[errno]);
			}
			if (bind (sfr, (struct sockaddr *)&raddr, sizeof(raddr)) < 0) {
        			fprintf (stderr, "read bind error: %s\n",
					 sys_errlist[errno]);
			}
			if (listen (sfr, 1)) {
        			fprintf (stderr, "read listen error: %s\n",
					 sys_errlist[errno]);
			}
			i = sizeof (struct sockaddr_in);
			if ((fr = accept (sfr, (struct sockaddr *)&iraddr, (void *)&i)) < 0) {
        			fprintf (stderr, "read accept error: %s\n",
					 sys_errlist[errno]);
			}
#else
			if (loop) {
               		       con.c_lport = rport;
               		       con.c_fport = wport;
				con.c_mode &= ~CONACT;
			}
			if (await) {
				mkanyhost(con.c_fcon);
				mkanyhost(con.c_lcon);
				con.c_fport = 0;
			}
        		if ((fr = netopen(&con)) < 0) {
        			fprintf (stderr, "read open error: %s\n",
					 sys_errlist[errno]);
			}
#endif
			conopen++;
#ifdef TIMERS
			if (timer > 0)
				alarm(timer);
#endif

			if (count >= 0)
        			rdcnt = count * bufsiz;
			else
				rdcnt = 1;

			while (rdcnt > 0 && !finis) {

				if (fd >= 0)
        				for (i=0; i < bufsiz; i++)
        					inbuf[i] = 0;

#ifdef TIMERS
				ftime(&stime);
#endif
				if ((i = read(fr, inbuf, bufsiz)) < 0) {
#ifndef USESOCKET
					if (errno == 35) {
        					ioctl(fr, NETGETS, &outstat);
						printf("read status: %o\n",
							outstat.n_state);
					} else
#endif
					if (errno != EINTR) {
						fprintf (stderr,
							"read error: %s\n",
							sys_errlist[errno]);
        					finis++;
					}

				} else if (i > 0) {
#ifdef TIMERS
        				ftime(&ttime);
#endif
					if (i != bufsiz && match)
					        printf("read %d: length mismatch -- asked %d got %d\n",
						 count, bufsiz, i);
					else
					        if (printit)
						        printf("read %d: %d\n",
							  count,i);

					if (compare)
						for (j=0; j < i; j++)
							if (inbuf[j] != ++last) {
								printf("read %d: comparison test failed at position %d char %x last %x\n",
								  count, j, inbuf[j], last);
								last = inbuf[j];
								break;
							}

					if (fd >= 0)
						if (write(fd, inbuf, i) < 0)
							fprintf (stderr,
								"Error writing output file: %s\n",
								sys_errlist[errno]);
#ifdef TIMERS
					totals += (float)(ttime.time - stime.time) +
						  (float)(ttime.millitm - stime.millitm) / 1000.;
					itotals += (float)(ttime.time - stime.time) +
						  (float)(ttime.millitm - stime.millitm) / 1000.;
#endif
					ibytes += i;
        				bytes += i;
					if (count-- >= 0)
        					rdcnt -= i;
				} else {
					printf("read: got EOF\n");
					finis++;
				}

			}
			close(fr);
			dorept(dir, totals, bytes, FALSE);
		        exit(0);
		}

	if (t_write) {

		dir = "write";
#ifdef USESOCKET
        	if ((fw = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        		fprintf (stderr, "write socket error: %s\n",
				 sys_errlist[errno]);
		}
		waddr.sin_port = htons (rport);
		if (bind (sfr, (struct sockaddr *)&raddr, sizeof(raddr)) < 0) {
        		fprintf (stderr, "write bind error: %s\n",
				 sys_errlist[errno]);
		}
		waddr.sin_port = htons (wport);
		if (connect (fw, (struct sockaddr *)&waddr, sizeof (waddr)) < 0) {
        		fprintf (stderr, "write connect error: %s\n",
				 sys_errlist[errno]);
		}
#else
		if (loop) {
        		con.c_lport = wport;
        		con.c_fport = rport;
			sleep(1);
		}
		if ((fw = netopen(&con)) < 0) {
			fprintf (stderr, "write open error: %s\n",
				sys_errlist[errno]);
		}
#endif
		conopen++;
#ifdef TIMERS
		if (timer > 0)
			alarm(timer);
#endif

#ifndef USESOCKET
		if (eol)
			ioctl(fw, NETSETE, 0);
#endif

		while (!finis && count != 0) {

#ifdef TIMERS
			ftime(&stime);
#endif
			if ((i = write(fw, outbuf, bufsiz)) < 0) {
#ifndef USESOCKET
				if (errno == 35) {
					ioctl(fw, NETGETS, &outstat);
					printf("write status: %o\n",
						outstat.n_state);
					continue;
				} else
#endif
				if (errno != EINTR) {
					fprintf (stderr, "write error: %s\n",
						sys_errlist[errno]);
					finis++;
				}

			} else {
#ifdef TIMERS
        			ftime(&ttime);
#endif
				if (i != bufsiz && match)
				        printf("write %d: length mismatch -- tried %d wrote %d\n",
     					     count, bufsiz, i);
        			else
        				if (printit)
        					printf("write %d: %d\n", count, i);
#ifdef TIMERS
				totals += (float)(ttime.time - stime.time) +
					  (float)(ttime.millitm - stime.millitm) / 1000.;
				itotals += (float)(ttime.time - stime.time) +
					  (float)(ttime.millitm - stime.millitm) / 1000.;
#endif
        			ibytes += i;
        			bytes += i;
			}

			if (count > 0 && --count <= 0)
				finis++;

		}
		close(fw);
		printf("write done.\n");

		if (t_read)
			wait(0);
		dorept(dir, totals, bytes, FALSE);
	}
	return 0;
}
