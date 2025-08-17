#include "stdio.h"
#include "string.h"

/* -------------------------- _ C M D E R R ------------------------- */
/*
 * _cmderr(func, errno, printargs) writes an error message
 * of the form
 *	progname: user_info errmsg.
 * on file descriptor 2. "errmsg" is obtained from the "errno"
 * argument; if "errno" is zero, it is null. "user_info" is printed by passing
 * the vector "printargs" on to printf. Whatever terminates the
 * user_info string (\r\n or \n) is moved to the end of the entire message.
 */
_cmderr(eno, printargs)
    int eno;
    char * printargs[];
{
    register char * term;
    char * format;
    register char * fmt_end;

    if (eno != 0)
    {
	format = printargs[0];
	fmt_end = &format[strlen(format)-1];
	if (fmt_end[0] == '\n' && fmt_end[-1] == '\r')
	{
	    fmt_end[-1] = '\0';
	    term = "\r\n";
	}
	else if (fmt_end[0]  == '\n')
	{
	    fmt_end[0] = '\0';
	    term = "\n";
	}
	else
	    term = "";
    }
    fprintf(stderr, "%r", printargs);
    if (eno != 0)
    {
	fprintf(stderr, " %s.%s", sys_errlist[eno], term);
	if (strchr(term, '\r'))
	{
	    fmt_end[-1] = '\r';
	    fmt_end[0] = '\n';
	}
	else if (strchr(term, '\n'))
	    fmt_end[0] = '\n';
    }
}

/* -------------------------- C M D E R R --------------------------- */
/*
 * Call _cmderr.
 */

/* VARARGS2 */
cmderr(eno, pfargs)
    int eno;
    char * pfargs;
{

    _cmderr(eno, &pfargs);
}

/* -------------------------- E C M D E R R ------------------------- */
/*
 * Like cmderr, but exit afterwards.
 */

/* VARARGS2 */
ecmderr(eno, pfargs)
    int eno;
    char *pfargs;
{

    _cmderr(eno, &pfargs);
    if (eno == 0)
	exit(-1);
    else
	exit(eno);
}

/* -------------------------- R C M D E R R ------------------------- */
/*
 * Like cmderr, but reset afterwards.
 */

/* VARARGS2 */
rcmderr(eno, pfargs)
    int eno;
    char * pfargs;
{

    _cmderr(eno, &pfargs);
    /*reset(1);	/* The argument is required by VAX version of reset (arrgh!) */
}

/* -------------------------- A C M D E R R ------------------------- */
/*
 * Like cmderr, but aborts afterwards.  
 */

/* VARARGS2 */
acmderr(eno, pfargs)
int   eno;
char *pfargs;
{

    _cmderr(eno, &pfargs);
    exit(1);
}


/* ------------------------ _ A C M D E R R ------------------------- */
/*
 * Like cmderr, but aborts afterwards.  This routine has a companion
 * function 'acmderr()' in a file of that name.  That function is 
 * is similar, but does a '_cleanup()' to flush buffers before the
 * abort.  It is not included here because some people are not using
 * stdio.
 */

/* VARARGS2 */
_acmderr(eno, pfargs)
int   eno;
char *pfargs;
{

    _cmderr(eno, &pfargs);
    fflush(stderr);                     /* Make sure the error gets printed */
    exit(1);
}
