/*
 * Print an unsigned integer in base b.
 */
static
printn(n, b)
	unsigned int n,b;
{
	register unsigned int a;
	char t[2];

	if(a = n/b)
		printn(a, b);
	t[0] = n%b + '0';
	write (1, t, 1);
}
/*
 * Print an unsigned integer in hex.
 */
static
printx(n)
	unsigned int n;
{
	register unsigned int a;
	char t[2];

	if(a = n >> 4)
		printx(a);
	n = n & 0xF;
	n += '0';
	if (n > '9')
	   n += 7;
	t[0] = n;
	write(1, t, 1);
}

/*
 * Scaled down version of C Library printk.
 * Only %s %l %d %x (==%l) %o are recognized.
 * Used to print diagnostic information
 */
printf(fmt,x1)
char fmt[];
{
	register char *s;
	register int *adx, c;
	char t[2];

	adx = &x1;
loop:
	while((c = *fmt++) != '%') {
		if (c == '\n') {
			write (1, "\r", 1);
		}
		if(c == '\0') {
			return;
		}
		t[0] = c;
		write(1, t, 1);
	}
	c = *fmt++;
	if(c == 'x')
		printx(*adx);
	else if(c == 'd' || c == 'l' || c == 'o')
		printn(*adx, c=='o'? 8: 10);
	else if(c == 's') {
		s = (char*) *adx;
		while(c = *s++) {
			t[0] = c;
			write(1, t, 1);
		}
	}
	adx++;
	goto loop;
}
