struct utmp {
	char	ut_name[8];		/* user id */
	char	ut_line[2];		/* tty */
	long	ut_time;		/* time on */
	int	ut_fill1;
};
