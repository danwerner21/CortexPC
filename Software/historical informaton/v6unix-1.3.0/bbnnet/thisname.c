#include <stdio.h>
#include "inet.h"

#define THISHOST "/etc/thishost"
#define HOSTNAME "TI990"
#define NETNAMSIZ 32

/*
 * return name of local host
 */
char *
thisname()
{
	register int c;
	register char *cp;
	register FILE *fp;
	static char buf[NETNAMSIZ+1];

	fp = fopen(THISHOST, "r");
	if (fp == NULL)
		return HOSTNAME;
	cp = buf;
	while (cp < &buf[NETNAMSIZ] && (c = getc(fp)) != EOF && c != '\n')
		*cp++ = c;
	*cp = '\0';
	fclose(fp);
	return buf;
}
