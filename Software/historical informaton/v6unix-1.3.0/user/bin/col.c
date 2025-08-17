#include <stdio.h>

main()
{
	register int c, state;

	state = 0;
	while((c = getchar())>=0) {
		if( c=='\n' ) {
			if( state>=2 ) continue;
			state++;
			putchar(c);
			continue;
		}
		state = 0;
		putchar(c);
	}
	fflush(stdout);
}
