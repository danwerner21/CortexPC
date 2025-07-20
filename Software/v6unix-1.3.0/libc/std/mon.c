
monitor(lowpc, highpc, buf, bufsiz, cntsiz)
	char *lowpc, *highpc;
	int *buf;
	unsigned bufsiz, cntsiz;
{
	register unsigned o;
	static *sbuf, ssiz;

	if (lowpc == 0) {
		profil(0, 0, 0, 0);
		o = creat("mon.out", 0666);
		write(o, sbuf, ssiz<<1);
		close(o);
		return;
	}
	ssiz = bufsiz;
	buf[0] = (int)lowpc;
	buf[1] = (int)highpc;
	buf[2] = (int)cntsiz;
	sbuf = buf;
	buf += 3*(cntsiz+1);
	bufsiz -= 3*(cntsiz+1);
	if (bufsiz<=0)
		return;
	o = highpc - lowpc;
	bufsiz <<= 1;
	if(bufsiz < o)
		o = ludiv(bufsiz, 0, o);
	else
		o = 0xffff;
	profil(buf, bufsiz, lowpc, o);
}

