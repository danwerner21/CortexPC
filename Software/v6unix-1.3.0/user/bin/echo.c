main(argc, argv)
int argc;
char *argv[];
{
	int i;
	char c, *s;

	argc--;
	for(i=1; i<=argc; i++) {
		s = argv[i];
		while (c = *s++) write(1, &c, 1);
		c = (i==argc) ? '\n': ' ';
		write(1, &c, 1);
	}
}

