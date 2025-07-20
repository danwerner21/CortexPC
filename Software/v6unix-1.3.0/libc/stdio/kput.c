kputn(n, b)
	unsigned n;
{
	register a;

	if(a = n/b)
		kputn(a, b);
	kputc((n%b) + '0');
}

void kputs (s)
	char *s;
{
	while(kputc(*s++));
}

