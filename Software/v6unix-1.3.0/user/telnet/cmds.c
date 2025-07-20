/*
 * Command processor, command table, and a lot of commands
 * Sometimes a command only exists to exercise a particular
 * subroutine. To aid in distinguishing, all command names
 * begin with Z.
 */
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>

#include "net.h"
#include "tnio.h"
#include "telnet.h"
#include "globdefs.h"
#include "ttyctl.h"
#include "inet.h"
#include "con.h"

extern int telwtlog, telrdlog;
extern int verbose;
extern int debug;
extern int needprompt;
extern int udone;
extern int xdone;
extern int escape;
extern int fsocket;
extern NETCONN *NetConP;
extern TTYMODE *NetModeP;

extern NETCONN *telopen();
extern NETCONN *telinit();
extern TTYMODE *AllocMode();

extern char *atoiv();
extern int ArgInt(), ArgFlag(), ArgCount();
extern char * ArgStr();
extern int telfd();
extern int from_net();
extern char *optname();
extern int net2user();
extern myecf(), myelf(), myaof(), mybreakf(), mygaf(), mysynchf();
extern int optnum(), option();

struct ComEntry
{
    char *ComName;
    void (*ComFunc)();
    int ComInt;
    char *ComStr;
};

static void zhelp(), zverbose(), zsendctl(), zset(), zsendopt();
static void zsendsub(), zoption(), znetclose(), zexit();
void znetopen();

static struct ComEntry ComTab[] =
{
    "help",	    zhelp,	0,	NULL,
    "?",	    zhelp,	0,	NULL,
    "verbose",	    zverbose,	'v',	NULL,
    "brief",	    zverbose,	'b',	NULL,
    "ip",	    zsendctl,	IP,	NULL,
    "ao",	    zsendctl,	AO,	NULL,
    "break",	    zsendctl,	BREAK,	NULL,
    "ec",	    zsendctl,	EC,	NULL,
    "el",	    zsendctl,	EL,	NULL,
    "ayt",	    zsendctl,	AYT,	NULL,
    "synch",	    zsendctl,	SYNCH,	NULL,
    "ga",	    zsendctl,	GA,	NULL,
    "set",	    zset,	0,	NULL,
    "open",	    znetopen,	0,	NULL,
    "connect",	    znetopen,	0,	NULL,
#ifdef AWAIT
    "do",	    zsendopt,	TN_DO,	NULL,
    "dont",	    zsendopt,	TN_DONT,NULL,
    "will",	    zsendopt,	TN_WILL,NULL,
    "wont",	    zsendopt,	TN_WONT,NULL,
    "sub",	    zsendsub,	0,	NULL,
    "accept",	    zoption,	1,	NULL,
    "refuse",	    zoption,	0,	NULL,
#endif
    "close",	    znetclose,	0,	NULL, /* Clean close */
    "quit",	    zexit,	0,	NULL,
    0,		    0,		0,	NULL,
};

/* -------------------------- C O M T A B --------------------------- */

/*
 * Each ComEntry is called with four arguments. Normally, they are:
 *
 * A parameter: either an integer or a character string. The structure
 *	must have both because there is no way to statically initialize a
 *	union. It is assumed that the command takes a character string
 *	if ComStr is non-NULL; if it is NULL, the ComInt is supplied
 *	instead.
 *
 * The number of arguments that appeared on the command line -- as with
 *     UNIX main programs, this is at least 1;
 * A pointer to an array of pointers to arguments, as with UNIX main programs.
 *
 * Each function may also be called upon to explain what it does. In this call,
 *     the second argument will be 0 and the third argument will be the name
 *     of the entry.
 * The command table itself is at the end of the program to avoid the need
 *     to declare all the entries.
 */

static int
tolower (ch)
int ch;
{
   if (ch >= 'A' && ch <= 'Z')
      ch += 0x20;
   return ch;
}

/* -------------------------- C O M P A R --------------------------- */
/*
 * compar(s1, s2) compares s1 against s2. Returns -1 if they are equal.
 * Otherwise, if s1 is a prefix of s2, returns # chars in common.
 * Otherwise returns 0.
 */
int
compar(s1, s2)
char *s1;
char *s2;
{
    register char *sp1 ,*sp2;

    sp1 = s1;
    sp2 = s2;
    while (tolower(*sp1++) == *sp2++)
        if (*(sp1-1) == 0)
            return(-1);     /* Exact match */

    /* Strings differ. If first one ran out, return # chars matched. */
    if (*(sp1-1) == 0)
        return(sp1-s1-1);

    /* Not a match at all... */
    return(0);
}

/* -------------------------- G E T C O M --------------------------- */
/*
 * getcom(strp, comtab) takes a pointer to a command name and looks it up
 * in comtab, returning a pointer to its entry there.
 */
static struct ComEntry *
getcom(strp, comtabp)
char *strp;
struct ComEntry comtabp[];
{
    struct ComEntry *cp;            /* pointer to com table entry    */
    struct ComEntry *candidate;  /* ptr to candidate command      */
    register int nmatch;             /* num chrs matched on this try */
    register int bestmatch;         /* best match found yet */

    if (strp == 0)
        return(0);                /* can't match null string */

    candidate = 0;
    bestmatch = 0;
    for (cp = comtabp; cp->ComFunc; cp++)     /* linear srch */
    {
        if((nmatch = compar(strp, cp->ComName)) != 0)
            if (nmatch == -1)
                return(cp);     /* take exact match      */
            else if (nmatch > bestmatch)
            {
                candidate = cp;
                bestmatch = nmatch;
            }
            else
            {         /* had two candidates that matched same */
		fprintf(stderr, "Not unique\n");
                return(0);
            }
    }
    return(candidate);
}

/* -------------------------- C A L L C M D ------------------------- */
/*
 * Splits the line up into words separated by tabs or blanks. Looks up
 * the first word on the line in the command table (above), and calls the
 * indicated function as explained above. If the word is not found, it is
 * an error.
 */
callcmd(line)
    char *line;
{
    register char *linep, *eol;
    int linelen;
    char **argp;
    struct ComEntry *comp;
    int outside_arg;
    static char *arglst[100];

    linelen = strlen(line);
    eol = &line[linelen-1]; /* Point to end */

    /* Clear MSB of each byte, in case binary mode is on. */

    for (linep = line; linep <= eol; linep++)
	*linep &= 0177;

    /* Make arg array point to beginning of each argument. */
    /* Null-terminate it. */

    argp = arglst;
    outside_arg = 1;
    for (linep = line; linep <= eol; linep++)
	switch(*linep)
	{
	    case ' ':
	    case '\t':
		outside_arg++;
		*linep = 0;
		break;
	    default:
		if(outside_arg)
		{
		    *argp++ = linep;
		    outside_arg = 0;
		}
		break;
	}

    /* Execute it */

    if (argp >= &arglst[sizeof(arglst) - 1])
	fprintf(stderr, "con: too many args. Max is %d.\n",
			sizeof(arglst) / sizeof(arglst[0]));
    else if (argp > arglst) /* At least a command name */
    {
	*argp = 0; /* To indicate last arg */
	if ((comp = getcom(arglst[0], ComTab)))
	{
	    arglst[0] = comp->ComName;	/* Give full name */
	    if (comp->ComStr != NULL)
		(*(comp->ComFunc))(comp->ComStr, argp - arglst, arglst);
	    else
		(*(comp->ComFunc))(comp->ComInt, argp - arglst, arglst);
	}
	else
	{
	    fprintf (stderr, "Unrecognized command: %s\n", arglst[0]);
	    return;
	}
    }
}

/* -------------------------- A R G P A R S E ----------------------- */
/*
 * Parse open command arguments. Put result in ncpopen structure.
 */
static int
argparse(argc, argv, op)
    int argc;
    char * argv[];
    register struct con * op;
{
    int host_arg_index;
    int i;
    char *logfile;

    if (argc == 1)
    {
	fprintf (stderr,
		 "Usage: %s [-lp #] [-fp #] [-t #] [-server] [host]\n",
		 argv[0]);
        return -1;
    }


    /* Defaults */
    op->c_mode = CONTCP | CONACT | ((debug)?CONDEBUG:0);
    op->c_sbufs = op->c_rbufs = 2;

    if (ArgFlag("-server", argv))
    {
	op->c_mode &= ~CONACT;
	op->c_lport = SERVER_SOCKET;
	op->c_rbufs = 1;
    } else op->c_sbufs = 1;

    op->c_lport = ArgInt("-lp", 0, argv);
    op->c_fport = ArgInt("-fp", fsocket, argv);
    op->c_timeo = ArgInt("-t", 30, argv);
    logfile = ArgStr("-log", NULL, argv);
    if (logfile)
    {
	telrdlog = telwtlog = open(logfile, O_WRONLY);
	if (telrdlog == -1)
	{
	    fprintf (stderr, "Log file create failed: %s\n", logfile);
	    return -1;
	}
    }
/*  if (ArgFlag("-direct", argv))       /* what is all this stuff anyway???
	op->c_type |= DIRECT;
    if (ArgFlag("-init", argv))
	op->c_type |= INIT;
    if (ArgFlag("-specific", argv))
	op->c_type |= SPECIFIC;
    if (ArgFlag("-duplex", argv))
	op->c_type |= DUPLEX;*/
/*    if (ArgFlag("-relative", argv)) /* Useless right now */
/*	op->c_type |= RELATIVE;	*/

    argc = ArgCount(argv);
    host_arg_index = 0;
    for (i = 1; i < argc; i++) {
	if (argv[i][0] == '-')
	{
	    fprintf (stderr, "%s: Unknown option\n", argv[i]);
	    return -1;
	}
	else if (host_arg_index == 0)
	{
	    host_arg_index = i;
	}
	else
	{
	    fprintf (stderr, "%s: Superfluous argument\n", argv[i]);
	    return -1;
	}
    }

    if (host_arg_index != 0)
    {
	op->c_fcon.s_addr = gethost(argv[host_arg_index]);
	mkanyhost(op->c_lcon);
	if (isbadhost(op->c_fcon))
	{
	    fprintf (stderr, "%s: Unknown host\n",
		     argv[host_arg_index]);
	    return -1;
	}
    }
    if (isanyhost(op->c_fcon))
    {
	fprintf (stderr, "No host specified.\n");
	return -1;
    }
    return 0;
}

/* -------------------------- Z N E T O P E N ----------------------- */
/*
 * Open a TELNET connection. Set NetConP to the connection pointer if
 * successful.
 */
void
znetopen(arg, argc, argv)
    int arg;
    int argc;
    char * argv[];
{
    NETCONN * connp = NULL;
    TTYMODE *modec = NULL;
    TTYMODE *modep = NULL;
    struct tchars *tp;
    int fd;
    struct con tcpopen;

    if (argc == 0)
    {
	fprintf(stderr, "Open a connection to the specified host.\n");
	return;
    }

    if (NetConP != NULL)
    {
	fprintf (stderr, "Connection already open.\n");
	return;
    }

    if (ArgFlag("-debug", argv))
	debug = 1;
    argc = ArgCount(argv);
    if (argparse(argc, argv, &tcpopen) < 0)
       return;

    fprintf(stderr, "Trying...");
    fflush (stderr);
    if ((fd = netopen (&tcpopen)) < 0)
    {
         fprintf (stderr, "Cannot connect to foreign host: %s\n",
	 	sys_errlist[errno]);
	 fflush (stderr);
	 return;
    }
    ioctl(fd, NETSETE, NULL); /* urgent assumed off, eols on. */

    fprintf(stderr, "Open\n");
    connp = telinit(fd);
    if (connp == NULL)
    {
	fprintf (stderr, "Cannot get connection block: %s\n", 
	 	sys_errlist[errno]);
	return;
    }
    modec = AllocMode();
    if (modec == NULL)
    {
	fprintf (stderr, "Cannot get mode block: %s\n", 
	 	sys_errlist[errno]);
	return;
    }
    NetConP = connp;
    NetModeP = modec;
    ChgMode(modec);
    /*SetFlags(modec, CRMOD, 0);*/
    modep = AllocMode();
    /*tp = &(modep->tm_tchars);*/
    /*tp->t_brkc = '\r';*/
    /*modep->tm_local = modec->tm_local&(~LCTLECH);*/
    ChgMode(modep);
    sendopt (connp, DONT, OPT_ECHO);
    if (debug)
    {
	telfunc(connp, EC, myecf);
	telfunc(connp, EL, myelf);
	telfunc(connp, AO, myaof);
	telfunc(connp, BREAK, mybreakf);
	telfunc(connp, GA, mygaf);
	telfunc(connp, SYNCH, mysynchf);
	verbose = 1;
    }
    /*IoEnter(telfd(connp), from_net, 0, connp);*/
    from_net (1, connp);
    /*IoxEnter(net2user, connp);*/
    needprompt = 1;
}

/* -------------------------- Z V E R B O S E ------------------------- */
/*
 * Turns the verbose flag on or off.
 */
static void
zverbose(val, argc, argv)
    char val;
    int argc;
    char *argv[];
{
    if (argc == 0)
    {
        if (val == 'v')
	    fprintf(stderr, "Announce all subsequent option negotiation\n");
        else
	    fprintf(stderr,
	       "Announce important option negotiations (e.g. 'Remote echo')\n");
        return;
    }
    else if (argc != 1)
    {
	fprintf(stderr, "I take no arguments\n");
        return;
    }
    switch(val)
    {
        case 'v':
            verbose = 1;
	    fprintf(stderr, "Verbose mode\n");
            break;
        case 'b':
            verbose = 0;
	    fprintf(stderr, "Brief mode\r\n");
            break;
        default:
	    fprintf(stderr, "vswitch(%c)?\n", val);
            break;
    }
    return;
}

/* -------------------------- Z S E N D C T L ----------------------- */

static void
zsendctl(arg, argc, argv)
    int arg;
    int argc;
    char *argv[];
{
    if (argc > 1)
    {
	fprintf(stderr, "I take no arguments\n");
	return;
    }

    if (argc == 0)
	switch(arg)
	{
	    case AO:
		fprintf(stderr,
			"Abort output (send an Abort-Output and Synch)\n");
		return;
	    case BREAK:
		fprintf(stderr, "Break; that is, send a Break and Synch\n");
		return;
	    case IP:
		fprintf(stderr, "Interrupt process (send IP and Synch)\n");
		return;
	    case AYT:
		fprintf(stderr,
			"Ask foreign host if it is still alive (send AYT)\n");
		return;
	    case EC:
		fprintf(stderr, "Erase last character (send EC)\n");
		return;
	    case EL:
		fprintf(stderr, "Erase to beginning of line (send EL)\n");
		return;
	    case SYNCH:
		fprintf(stderr, "Send a Synch\n");
		return;
	    case GA:
		fprintf(stderr, "Send a Go-Ahead\n");
		return;
	}

    if (NetConP == 0)
    {
	fprintf(stderr, "No active connection\n");
	return;
    }
    sendctl(NetConP, arg);
}

/* -------------------------- Z H E L P ----------------------------- */

static void
zhelp(arg, argc, argv)
    int arg;
    int argc;
    char *argv[];
{
    int i;

    if (argc == 1)     /* Give each command and have it show-and-tell */
    {
	fprintf(stderr, "TELNET Commands\n");
	fprintf(stderr,
"Only enough of each command name to uniquely identify it need be typed.\n\n");
	for (i = 0; ComTab[i].ComFunc; i++)
	{
	    fprintf(stderr, "%-11.11s", ComTab[i].ComName);
	    if (ComTab[i].ComStr != NULL)
		(*(ComTab[i].ComFunc))(ComTab[i].ComStr, 0, &ComTab[i].ComName);
	    else
		(*(ComTab[i].ComFunc))(ComTab[i].ComInt, 0, &ComTab[i].ComName);
	}
    }
    else if (argc == 0)
	fprintf(stderr, "Briefly explain each command.\n");
    else
    {
	fprintf(stderr, "Sorry, individual explanations not implemented yet\n");
	fprintf(stderr,
	        "Type 'help' for a list of all commands and their purposes\n");
    }
    return;
}

/* -------------------------- Z N E T C L O S E --------------------- */
/*
 * Close a network connection.
 */
static void
znetclose(arg, argc, argv)
    int arg;
    int argc;
    char *argv[];
{

    if (argc == 0)
	fprintf(stderr, "Close the current network connection\n");
    else if (argc != 1)
	fprintf(stderr, "%s takes no arguments\n", argv[0]);
    /* Connection we're supposed to close doesn't exist */
    else if (NetConP == 0)
	return;
    else
	udone++;
}

#ifdef AWAIT
/* -------------------------- Z S E N D O P T ----------------------- */
/*
 * Start the negotiation process for the requested option.
 */
static void
zsendopt(arg, argc, argv)
    int arg;
    int argc;
    char *argv[];
{
    int optno;

    if (argc == 0)
    {
	fprintf(stderr, "Try to negotiate the specified option.\n");
	return;
    }
#if 0
    if (setexit())
	return;
#endif

    if (argc != 2)
    {
	int i, nopts;

	fprintf(stderr, "Usage: %s option\n", argv[0]);
	fprintf(stderr, "Available options are:");
	nopts = 0;
	for (i = 0; i < 255; i++)   /* All possible option numbers */
	{
	    char * name = optname(i);

	    if (name != NULL)
	    {
		if (nopts % 3 == 0)
		    fprintf(stderr, "\n\t  ");
		fprintf(stderr, "%20s", name);
		nopts++;
	    }
	}
	fprintf(stderr, "\n");
	return;
    }

    if (NetConP == NULL)
    {
	fprintf(stderr, "No active connection\n");
	return;
    }

    optno = optnum(argv[1]);
    if (optno == -1)
    {
	fprintf (stderr, "%s: Ambiguous\n", argv[1]);
	return;
    }
    if (optno == -2)
    {
	fprintf (stderr, "%s: Not recognized.\n", argv[1]);
	return;
    }

    if (sendopt(NetConP, arg, optno))
	fprintf (stderr, "%s: Could not request option\n", argv[1]);
    return;
}

/* -------------------------- Z O P T I O N ------------------------- */
/*
 * Accept or refuse the given option.
 */
static void
zoption(arg, argc, argv)
    int arg;	/* 0 refuse, nonzero accept */
    int argc;
    char * argv[];
{
    int optno;

    if (argc == 0)
    {
	if (arg)
	    fprintf(stderr, "Accept the specified option.\n");
	else
	    fprintf(stderr, "Refuse the specified option.\n");
	return;
    }
#if 0
    if (setexit())
	return;
#endif

    if (argc == 2)
    {
	optno = optnum(argv[1]);
	if (optno == -2)
	{
	    fprintf (stderr, "%s: Option not recognized\n", argv[1]);
	    return;
	}
	if (optno == -1)
	{
	    fprintf (stderr, "%s: Option ambiguous\n", argv[1]);
	    return;
	}
	if (option(arg, optno))
	{
	    fprintf (stderr, "Cannot enter option.\n");
	    return;
	}
    }
    else
	fprintf (stderr, "Usage: %s option\n", argv[0]);

    return;
}
#endif

/* -------------------------- Z S E N D S U B ----------------------- */
/*
 * Send a subnegotiation.
 */
static void
zsendsub(arg, argc, argv)
    int arg;
    int argc;
    char * argv[];
{
    int i;
    char buffer[64];
    int optno;

    if (argc == 0)
    {
	fprintf(stderr, "Send the specified subnegotiation sequence.\n");
	return;
    }
    if (argc == 1)
    {
	fprintf(stderr, "Usage: %s optnum [ byte ] ...\n", argv[0]);
	return;
    }
#if 0
    if (setexit())
	return;
#endif
    optno = optnum(argv[1]);
    if (optno == -1)
    {
	fprintf (stderr, "%s: Ambiguous\n", argv[1]);
	return;
    }
    if (optno == -2)
    {
	fprintf (stderr, "%s: Not recognized\n", argv[1]);
	return;
    }
    for (i = 2; i < argc; i++)
    {
	int value;

	if (*atoiv(argv[i], &value) != '\0')
	{
	    fprintf (stderr, "%s: Not numeric\n", argv[i]);
	    return;
	}
	buffer[i-2] = value;
    }
    sendsub(NetConP, optno, buffer, i-2);
}

/* -------------------------- Z E X I T ----------------------------- */
/*
 * Exit from telnet without waiting for connection to stabilize.
 */
static void
zexit(arg, argc, argv)
    int arg;
    int argc;
    char * argv[];
{

    if (argc == 0)
    {
	fprintf(stderr, "Exit from TELNET immediately.\n");
	return;
    }
    xdone = 1;
    check_done();
    exit (0);
}

/* -------------------------- Z S E T ------------------------------- */
/*
 * Sets the variable to the character described.
 */
static void
zset(arg, argc, argv)
    int arg;
    int argc;
    char *argv[];
{
    register char * ap;
    register int * charp;

    if (argc == 0)
    {
	fprintf(stderr, "Set special characters.\n");
	return;
    }
    if (argc != 3)
    {
	fprintf(stderr, "Usage: %s <function> { <char> | off }\n", argv[0]);
	fprintf(stderr,
		" where <function> is currently only 'escape' ('esc')\n");
	return;
    }
    ap = argv[1];
    if ((strcmp(ap, "escape") == 0) || (strcmp(ap, "esc") == 0))
	charp = &escape;
    else
    {
	fprintf(stderr, "%s: No such function\n", ap);
	return;
    }
    ap = argv[2];
    if (ap[1] == '\0')
	*charp = ap[0] & 0377;
    else if (strcmp(ap, "off") == 0)
	*charp = -1;
    else
    {
	fprintf(stderr,
		"Sorry, only a literal character or 'off' may be specified.\n");
	return;
    }
}

/* -------------------------- A N N O U N C E ----------------------- */
/*
 * Announce being called with given argument.
 */
announce(arg)
    char * arg;
{
    fprintf(stderr, "%s", arg);
}

/* -------------------------- M Y E C F ----------------------------- */
myecf() { announce("<EC>"); }
/* -------------------------- M Y E L F ----------------------------- */
myelf() { announce("<EL>"); }
/* -------------------------- M Y A O F ----------------------------- */
myaof() { announce("<AO>"); }
/* -------------------------- M Y I P F ----------------------------- */
mygaf() { announce("<GA>"); }
/* -------------------------- M Y B R E A K F ----------------------- */
mybreakf() { announce("<BREAK>"); }
/* -------------------------- M Y S Y N C H ------------------------- */
mysynchf() { announce("<SYNCH>"); }

/* ------------------------------------------------------------------ */
