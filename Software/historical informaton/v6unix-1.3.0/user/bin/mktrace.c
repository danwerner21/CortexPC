#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char names[655*8][9];

main(argc, argv)
char **argv;
{
	FILE *map, *trace;
	char *line = NULL, *p;
	long  linecap = 0, linelen, adr;
	int i;

	map = fopen("unix.map","r");
	while ((linelen = getline(&line, &linecap, map)) > 0) {
		adr = strtoul(line, NULL, 16);
		for(i=0; i<8; i++) {
			char c = line[i+7];
			if (c=='\n') break;
			names[adr][i] = c;
		}
	}
	fclose(map);

	map = fopen("app.map","r");
	while ((linelen = getline(&line, &linecap, map)) > 0) {
		adr = strtoul(line, NULL, 16);
		for(i=0; i<8; i++) {
			char c = line[i+7];
			if (c=='\n') break;
			names[adr+2*65536][i] = c;
		}
	}
	fclose(map);

	trace = fopen("tmp","r");
	while ((linelen = getline(&line, &linecap, trace)) > 0) {
		for( p = line; *p!='\n' && *p!=':'; p++);
		adr = strtoul(p+10, NULL, 16);
		find_label(adr);
		fprintf(stdout, "%s", line);		
	}
	fclose(trace);
}

find_label(addr)
{
	int i;

	for(i=0; i<addr; i++) {
		if (!names[addr-i][0]) continue;
		printf("%8s+%4d: ", names[addr-i], i);
		break;
	}
}
