/*
 * Set date and time.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int	dmsize[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
long	timbuf;
char	*cbp;

/*
 * Accurate only for the past couple of centuries;
 * that will probably do.
 */
int dysize(y)
	int y;
{
	if (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0))
		return 366;
	return 365;
}

int
gpair()
{
	register int c, d;
	register char *cp;

	cp = cbp;
	if (*cp == 0)
		return -1;
	c = (*cp++ - '0') * 10;
	if (c<0 || c>100)
		return -1;
	if (*cp == 0)
		return -1;
	if ((d = *cp++ - '0') < 0 || d > 9)
		return -1;
	cbp = cp;
	return c + d;
}

void
gdadd(n)
{
	timbuf += n;
}

void
gmdadd(m, n)
{
	timbuf *= m;
	timbuf += n;
}

int
gtime()
{
	register int i;
	register int y, t;
	int d, h, m;
	long nt;

	t = gpair();
	if (t<1 || t>12)
		return 1;
	d = gpair();
	if (d<1 || d>31)
		return 1;
	h = gpair();
	if (h == 24) {
		h = 0;
		d++;
	}
	m = gpair();
	if (m<0 || m>59)
		return 1;
	y = gpair();
	if (y<0) {
		time(&nt);
		y = localtime(&nt)->tm_year;
	}
	if (*cbp == 'p')
		h += 12;
	if (h<0 || h>23)
		return 1;
	timbuf = 0;
	y += 2000;
	for (i=1970; i<y; i++)
		gdadd(dysize(i));
	if (dysize(i) == 366)
		dmsize[1] = 29;
	else
		dmsize[1] = 28;
	while (--t)
		gdadd(dmsize[t-1]);
	gdadd(d-1);
	gmdadd(24, h);
	gmdadd(60, m);
	gmdadd(60, 0);
	return 0;
}

void
getval (prompt, val, min, max)
	char *prompt; int *val;
{
	int l, lv, x;
	char temp[10];

	lv = -1;

	while (lv < 0) {
		l = strlen(prompt);
		write (1, prompt, l);
		l = read (0, temp, 6);
		temp[l] = 0;
		x = atoi(temp);
		if (x >= min && x <= max)
		   lv = x;
	}
	*val = lv;
}

int
main(argc, argv)
	int argc;
	char **argv;
{
	char *tzn;
	char *cbuf;
	int year, month, day, hour, minute;
	char temp[64];

	write (1, "Initialize Date/Time\n", 21);
AGAIN:
	getval ("   Year: ", &year, 2000, 2099);
	getval ("  Month: ", &month, 1, 12);
	getval ("    Day: ", &day, 1, 31);
	getval ("   Hour: ", &hour, 0, 23);
	getval (" Minute: ", &minute, 0, 59);
	sprintf (temp, "%02d%02d%02d%02d%02d",
	 	 month, day, hour, minute, year-2000);

	cbp = temp;
	if (gtime()) {
		write(1, "Bad conversion\n", 15);
		goto AGAIN;
	}
	/* convert to Greenwich time, on assumption of Standard time. */
	timbuf += timezone;
	/* Now fix up to local daylight time. */
	if (localtime(&timbuf)->tm_isdst)
		timbuf -= 1 * 60 * 60;
	if (stime(&timbuf) < 0) {
		write(1, "No permission\n", 14);
		return 1;
	}
	time(&timbuf);
	cbuf = ctime(&timbuf);
#ifdef TI_CTX
	write(1, cbuf, 20);
	tzn = tzname[localtime(&timbuf)->tm_isdst];
	if (tzn)
		write(1, tzn, 3);
	write(1, cbuf + 19, 6);
#else
	write(1, cbuf, 26);
#endif
	return 0;
}
