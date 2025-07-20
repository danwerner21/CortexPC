#include <stdio.h>

getit()
{
	return getchar();
}

putit(c)
{
	putchar(c);
}

main()
{
	register int c, state;

	state = 0;
	while((c = getit())>=0) {
		if( c=='\n' ) {
			if( state>=2 ) continue;
			state++;
			putit(c);
			continue;
		}
		state = 0;
		putit(c);
	}
	fflush(stdout);
}

