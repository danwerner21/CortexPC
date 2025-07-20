
char	shell[]	= "/bin/sh";
char	minus[]	= "-";
char	runc[]	= "/etc/rc";
char	init[]	= "/etc/init";
char	ctty[]	= "/dev/tty0";

main()
{
	register i;
	register struct tab *p, *q;
	int reset();

	/*
	 * run boot script
	 */
	i = fork();
	if(i == 0) {
		open("/", 0);
		open(ctty, 1);
		dup(0);
		execl(shell, shell, runc, 0);
		exit();
	}
	while(wait() != i);

	/*
	 * main loop for hangup signal
	 * start shell & restart forever
	 */
	setexit();
	signal(1, reset);

	for(;;) {
		i = fork();
		if(i == 0) {
			open(ctty, 2);
			dup(0);
			dup(0);
			execl("/bin/login", 0);
			exit();
		}
		while(wait() != i);
	}
}
