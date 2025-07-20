#include "ftp.h"
#include "usr.h"
#include "ftptelnet.h"
#include <ctype.h>
#include <sys/types.h>
#include <sgtty.h>
#include "ftp_lib.h"
#include "inet.h"

/*
 * User FTP
 *
 * Modified by Dan Franklin Sept 8 1978 (BBN) to take
 * commands from "getline" routine. This enables it
 * to have its input redirected to a file of commands.
 * Also moved declarations of strings for command prompting
 * to ahead of the command table so that our new C compiler
 * won't complain.
 *
 * Modified by Dan Franklin (BBN) August 11 19598 to put
 * blank line after "From:" line in send-mail command.
 * The blank line indicates the end of the header as
 * per RFC733.
 *
 * 3/79, added cwd and quote commands.	delete inside rwait returns
 * to prompt instead of saying (incorrectly) "Host exited"  ado/bbn
 *
 * changed to use long hosts jsq BBN 3-25-79
 * use fmodes for creat mode, give error on NCP open fail jsq BBN 19July79
 * init and subshl changed to handle signals differently:  if parent
 * shell ignored signals, this process does too.  jsq BBN 5Aug79
 * add arguments to allow passing process id of other half so that one
 *	can kill the other when it is ready to die; also do all host
 *	name-number conversions in ftp.c so we don't have so much stuff
 *	in here.  jsq BBN 18Aug79
 *
 * made f_retr check if destination already existed, and not delete on error
 * if it does. This should eliminate one cause of the /dev/tty disappearances.
 * Also fixed signal handling in fork exec'ing shell. Dan Franklin (BBN) 12/21/79
 *
 * Added NLST command (like LIST but different at server end). Also changed
 * CMDENTS to use sizeof cmdtab.
 *
 * Added XSEN and XSEM commands, which are just like MAIL (to user FTP) but
 * send messages instead of/along with mail on servers which implement
 * them. Dan Franklin BBN Feb 8 '80
 *
 * Added SEND and GET synonyms for STOR and RETR respectively.  Added
 * LOGIN command with 1, 2 or 3 args, and new argument list types
 * ARG1AND2 and ARG1OR2OR3.  Buz Owen, BBN, March 4 '80.
 *
 * Fixed bug whereby LIST and NLST were synonyms of RETR.  Changed QUOTE
 * command to take two args.  Changed ARGSIZ to 80, and LINSIZ to 250.
 * Changed quote and cwd to use putarg, putstr, putcmd more conventionally.
 * Buz Owen, BBN, March 11, 80.
 *
 * ca. Mar 25, 1980 (dm at BBN):
 *   changed issep (c) to return TRUE if (c == '\0'); this is so movarg
 *     can act on things its already acted on, so it could be used in the
 *     multiple store stuff
 *   added the multiple store stuff to server and user processes, working
 *     on unix-unix tree transfers
 *   in chekds(), which opens the data connection, turned off user-interrupts
 *     while waiting for the connection to be openned.  at least with tenex,
 *     failure to open this connection after you've said you were going to can
 *     get the server confused, and it stops listening to you (probably
 *     waiting for the connection to open).
 *   changed f_stor() to act as a front end for a separate subroutine, store()
 *     which does the actual work;  store() is called by the multiple store
 *     stuff.
 *
 * jsq BBN 5April80 declared cmdtab as struct ftucmd cmdtab[] in initiali-
 *   zation, fixed definition of ftucmd.ft_info, declared getcmd properly in
 *   main before calling it, fixed getcmd to use ft_info correctly.
 *
 * dm (bbn) 15 apr 1980:
 *   installed mretrieve() command; gets an NLST from the other site, and then
 *     does a retrieve on all the names of files returned.
 *     has subcommands to translate tenex-like file-names into unixable file-
 *     names, and work other transformations
 *   changed error() to accept fprintf-like arguments, so i could improve
 *     error messages it returns
 *   added the statistics package
 *     note that statistics aren't gathered on recursive stores (done in
 *     a lower fork.
 *   moved all the MGET internals to the end of the file, so they would be
 *     easier to find
 * dm (bbn) 31 May 1980:
 *   put a 1-second sleep into the loop in sendabunch() tenex requires time
 *     to become convinced the last data-socket was closed, so, cycling too
 *     quickly through several files will result in a "cannot open data
 *     connection" message, and Unix will be sitting there trying to open
 *     the data socket, anyway...
 *   changed rwait() to know about the proposed 257 reply code which tells
 *     you what the directory was which was created.  Also changed ftptty
 *     to know about such things.
 *   put the interrupt stuff back into checkds()--its abscence caused too
 *     many problems, and i guess people can restart their tenex ftps if
 *     tenex gets confused...
 * jsq BBN 27May80 make prstat give baud rate as well as bytes/sec.
 * dm (bbn) 5 Sept. 80: make sure indat() & outdat() close the dataconnection
 *     when the file is through.
 * dm (bbn) 25 Nov. 80: make a version which works with both TCP & NCP
 *      changed setspec() & calling routines to handle the length of the
 *       tables automatically (i.e., recognize that a null entry implies
 *       the end of a table)
 *      changed f_mlfl() to use putcmd() & sndcmd() instead of writing
 *       directly to the network
 *      dsfds & dsfdr are structures now
 *
 * ado (bbn) feb - jun, 81.  finished tcp version, v7ized everything and 
 *	stdio'd everything possible,
 *	following agn's changes to c70 version, and further urging.
 *	combined ftp (the command), ftpmain, and ftptty into one program.
 *	stripped out data transfer routines in favor of ftp_lib routines which
 *	are common to both user and server ftp, supplied by agn.
 *	completely new version rwait to deal with systematic reply code scheme
 *	of new protocol.
 * ado (bbn) jul 81.  added cmdget and cmdsend commands, to retrieve into a
 *	a filter, and send output from a command  respectively.
 *  */

struct net_stuff NetParams;
struct net_stuff DataParams;

char linbuf[LINSIZ];
char *linptr;		/* For Telnet output */

int mode;
int type;
int stru;
int abrtxfer;

char *errmsg();
char *dumptime();

#define ARGSIZ  80
#define TTBSIZ  160
#define MAXSTRING  10           /* Length of the longest command name */
#define IOBSIZ  120

struct spctab
    {
	char *typnm;
	char *typarg;
	int typnum;
    };
    
static struct spctab tycode[] =
    {
	{ "ASCII", "A", 0 },
	{ "TELNET", NULL, 1 },		/* non implimented values in */
	{ "PRINT", NULL, 2 },		/* anyway, so we can give */
	{ "EBCDIC", NULL, 3 },		/* slightly more intelligent */
	{ "IMAGE", "I", 6 },
	{ "LOCAL", "L8", 7 },
	{  NULL, NULL, -1 }
    };

static struct spctab mdcode[] =
    {
	{ "STREAM", "S",  0 },
	{ "BLOCK", NULL, -1 },			/* error messages. */
	{ "COMPRESSED", NULL, -1 },		/* not supported... */
	{  NULL, NULL, -1 },
    };

static struct spctab strucode[] =
    {
	{ "FILE", "F", 0 },
	{ "RECORD", "R", 1 },
	{ NULL, NULL, -1 },
    };
/*  */

static FILE *fdfil = NULL;

/* Uninitialized data */

static char ttbuf[TTBSIZ], *ttptr, ttcnt;             /* For tty input */
static char arg1[ARGSIZ], arg2[ARGSIZ], arg3[ARGSIZ]; /* For string arguments */
static char tmpbuf[120];

static int uicount = 0;               /* No. of user interrupts */
static int xreply;                    /* Global reply location */

static char mlbf[82], *mlptr;           /* For mail headers */
static char * HOST_ID, * them, * myname;
static unsigned long myip;

struct ftucmd				/* Structure of the command table */
    {
	char  *ft_cnm;
	int   ft_nargs;
	char  **ft_info;
	void   (*ft_fn)();
	char  *ft_desc;
    };

/* Definitions for ft_nargs field */
#define ARG0       0001
#define ARG1       0002
#define ARG2       0004
#define ARG0OR1    0010
#define ARG1OR2    0020
#define ARG1AND2   0040
#define ARG1OR2OR3 0100

/* Strings for command prompting */

static char locfil[] =  "localfile:  ";
static char rmtfil[] =  "remotefile: ";
static char persn[] = "person: ";

static char *afsnd[] = { "Input filter (command):", rmtfil };
static char *afget[] = { locfil, "Output filter (command): " };
static char badcmd[] = "bad filter/command: %s\n";

static char aborstr[] = { TNIAC, TNIP, TNIAC, TNDM, 'A', 'B', 'O', 'R', 0 };

static char *arsto[] = { locfil, rmtfil };

static char *arcwd[] = { "remote pathname: " };
static char *ardir[] = { "local pathname: "};
static char *ardel[] = { rmtfil };
static char *arlis[] = { "remote pathname: ", locfil };

static char *armod[] = { "transfer mode: "};
static char *arren[] = { "old remotefile: ", "new remotefile: " };

static char *arret[] = { rmtfil, locfil };

static char *arate[] = { "[reset] " };
static char *arper[] = { persn };
static char *arquo[] = { "command [argument]: " };
static char *arstr[] = { "file structure: " };
static char *artyp[] = { "data type: " };
static char *aruse[] = { "Username: " };
static char *armlf[] = { locfil, persn };

static void help(), f_acct(), f_appe(), byedie(), f_help(), f_cwd(), f_pwd();
static void f_dele(), f_list(), f_log(), f_mail(), f_mode(), f_pass();
static void f_quot(), f_rena(), f_stat(), f_stor(), f_stru(), f_type();
static void f_user(), f_retr(), f_rate(), f_err(), f_nlst(), f_mret();

static
struct ftucmd cmdtab[] =	/* Command Table -- must be alphabetical */
{
  {     "ACCOUNT",      ARG0OR1,        NULL,   f_acct,
          "specify account on foreign host"          },
  {     "APPEND",       ARG2,           arsto,  f_appe,
          "append local file to foreign file"        },
  {     "BYE ",         ARG0,           NULL,   byedie,
          "close the connection, and exit"           },
  {     "CD",           ARG1,           arcwd,  f_cwd,
          "change directory on foreign host"         },
  {     "DELETE",       ARG1,           ardel,  f_dele,
          "remove a file from the foreign host"      },
  {     "DIR",          ARG1OR2,        arlis,  f_list,
          "get a directory listing"		     },
  {     "GET ",         ARG2,           arret,  f_retr,
          "retrieve a file from the foreign host"    },
  {     "HELP",         ARG0OR1,        NULL,   help,
          "briefly describe commands"                },
  {     "LOGIN",        ARG1OR2OR3,     aruse,  f_log,
          "log onto a foreign host"                  },
  {     "LS",           ARG1OR2,        arlis,  f_list,
          "get a directory listing"		     },
  {     "MODE",         ARG1,           armod,  f_mode,
          "specify transfer mode (STREAM or BLOCK)"  },
  {     "NLIST",        ARG1OR2,        arlis,  f_nlst,
          "get a directory listing"		     },
  {     "PASSWORD",     ARG0OR1,        NULL,   f_pass,
          "tell the foreign host your password"      },
  {     "PUT" ,         ARG2,           arsto,  f_stor,
          "store a local file on the foreign host"   },
  {     "PWD" ,         ARG0OR1,        NULL,    f_pwd,
          "get current foreign host directory"       },
  {     "QUIT",         ARG0,           NULL,   byedie,
          "close the connection, and exit"           },
  {     "QUOTE",        ARG1OR2,        arquo,  f_quot,
          "send quoted string to ftp server"	     },
  {	"RATE",		ARG0OR1,	arate,	f_rate,
	  "print transfer rate"			     },
  {     "RENAME",       ARG2,           arren,  f_rena,
          "rename a file on the foreign host"        },
  {     "RETRIEVE",     ARG2,           arret,  f_retr,
          "retrieve a file from the foreign host"    },
  {     "RHELP",        ARG0OR1,        NULL,   f_help,
          "get remote server help"		     },
  {     "RSTATUS",      ARG0OR1,        NULL,   f_stat,
          "tell status of the ftp connection"        },
  {     "SEND",         ARG2,           arsto,  f_stor,
          "store a local file on the foreign host"   },
  {     "STRUCTURE",    ARG1,           arstr,  f_stru,
          "set file transfer structure"		     },
  {     "TYPE",         ARG1,           artyp,  f_type,
          "set file transfer type"                   },
  {     "USER",         ARG1,           aruse,  f_user,
          "tell the foreign host your name"          },
  {     NULL,           0,              NULL,   f_err,
          "command processor error"                  }
};

static
char *errtab[] = {
/* 0 */ 	"Unrecognized command\n",
/* 1 */ 	"Ambiguous command\n",
/* 2 */ 	"Command error\n",
/* 3 */         "Wrong number of arguments\n",
/* 4 */         "Command argument too long\n",
/* 5 */         "Bad specification\n",
/* 6 */         "Can't create file\n",
/* 7 */         "File not found\n"
};

static char ibuf[IOBSIZ], *iptr;
static int icnt = 0;
static int cmdflg = 0;
extern int synchno;


/*
 * convert to upper case
 */

static void
getuc(s)
register char *s;
{
	register int c;

	while (c = *s)
		*s++ = islower(c)? toupper(c) : c;
}

static void
prompt()
{
	printf ("> ");
	fflush (stdout);
}

static int
issep (c)
  char (c);
{
	return (!c) || isspace (c);
}

static void
error (n)
  char *n;
{

	ttcnt = 0;
	ttptr = ttbuf;
	printf ("%s", n);
	fflush(stdout);
	if (fdfil != NULL && fdfil != stdout)
	{
	    fclose (fdfil);
	    fdfil = NULL;
	}
}

static int
movarg (s, buf, maxch)
  char *s, *buf;
  int maxch;
{
    register int i;
    register char *p, *q;
    char quote;

	p = s;  q = buf;   i = 0;
	if (*p == '"' || *p == '\'')
	{
	    quote = *p++;
	    i += 2;
	    maxch += 2;
	}
	else quote = '\0';
	if (quote) do if ((*q++ = *p++) == quote) goto out; while (i++ < maxch);
	else do if (issep (*q++ = *p++)) goto out; while (i++ < maxch);
	error (errtab[4]);
	return -1;
    out:
	*--q = '\0';
	ttcnt -= i;
	ttptr += i;
   return 0;
}

static char *
getarg()
{
	for (;;)
	{
	    if (ttcnt <= 0)
	    {
		ttbuf[0] = 0;
		if (fgets (ttbuf, TTBSIZ, stdin) == NULL)
		    ttcnt = 0;
		else
		    ttcnt = strlen (ttbuf);
		if (ttcnt <= 0)
		{
		    byedie();
		}
		ttptr = ttbuf;
	    }
	    while (ttcnt-- > 0)
		switch (*ttptr++)
		{
		    case '\n':
			return (0);
		    case ' ':
		    case '\t':
			continue;
		    default:
			++ttcnt;
			return (--ttptr);
		}
	}
}

static int
match (s1, s2)
  char *s1, *s2;
{
    register char *p1, *p2;

	p1 = s1;  p2 = s2;
	while (*p1 == *p2++ && *p1) p1++;
	return (*p1 ? 0 : 1);
}

static struct ftucmd *
getcmd()
{
    register struct ftucmd *sp;
    register char *p;

     while ((p = getarg()) == 0) prompt();
     if (movarg (p, arg1, MAXSTRING) < 0)
	return NULL;
     getuc (arg1);
     for (sp = cmdtab; (sp->ft_cnm); sp++)
     {
	 if (match (arg1, sp->ft_cnm)) goto win;
     }
     error (errtab[0]);
     return NULL;
 win:
     if ((sp+1)->ft_cnm && match (arg1, (sp+1)->ft_cnm))
     {
	error (errtab[1]);
	return NULL;
     }
     if ((p = getarg()) == 0)
	 switch (sp->ft_nargs)
	 {
	     case ARG0OR1:
		 arg1[0] = '\0';
	     case ARG0:
		 return (sp);
	     case ARG1:
	     case ARG2:
	     case ARG1AND2:
	     case ARG1OR2:
	     case ARG1OR2OR3:
		 printf ((sp->ft_info)[0]);
		 fflush (stdout);
		 if ((p = getarg()) == 0)
		 {
		    error (errtab[2]);
		    return NULL;
		 }
	 }
     else if (sp->ft_nargs == ARG0)
     {
	error (errtab[3]);
	return NULL;
     }
     if (movarg (p, arg1, ARGSIZ - 1) < 0)
	return NULL;

     if ((p = getarg()) == 0)
	 switch (sp->ft_nargs)
	 {
	     case ARG1AND2:
		 printf ((sp->ft_info)[1]);
		 fflush (stdout);
		 if (p = getarg()) break;
	     case ARG1OR2:                   /* arg 2 is optional */
	     case ARG1OR2OR3:
		 arg2[0] = '\0';
	     case ARG1:
	     case ARG0OR1:
		 return (sp);
	     case ARG2:
		 printf ((sp->ft_info)[1]);
		 fflush (stdout);
		 if ((p = getarg()) == 0)
		 {
		    error (errtab[2]);
		    return NULL;
		 }
	 }
     else if (sp->ft_nargs & (ARG1|ARG0OR1))
     {
	error (errtab[3]);
	return NULL;
     }
     if (movarg (p, arg2, ARGSIZ - 1) < 0)
	return NULL;
     if (p = getarg())
	 {
	     if (sp->ft_nargs == ARG1OR2OR3)
	     {
		     if (movarg (p, arg3, ARGSIZ - 1) < 0)
			return NULL;
	     }
	     else
	     {
		error (errtab[3]);
		return NULL;
	     }
	 }
     else arg3[0] = '\0';
     return (sp);
}

static int
netgetch()
{
	if(--icnt < 0)
	{
retry:
		if ((icnt = net_read(&NetParams, ibuf, IOBSIZ)) < 0) {
		    if (errno == EINTR)
			goto retry;
		    else if (errno == ENETSTAT) {
			get_stuff(&NetParams);
			if (NetParams.ns.n_state & URXTIMO) {
				printf("Host not responding\n");
				goto retry;
			} else if (NetParams.ns.n_state & UURGENT)
				goto retry;
		    }
		}
		if(icnt-- < 0)
		{
		   sprintf (tmpbuf, "Net input closed: %s\n", errmsg(0));
		   die (4, tmpbuf);
		}
		iptr = ibuf;
	}
	return(*iptr++ & 0377);
}

static void
tpopt(c)
  unsigned char c;
{
    unsigned char bf[3];

	bf[0] = TNIAC;
	bf[1] = (c == TNDO ? TNWONT : TNDONT);
	bf[2] = netgetch();
	if (net_write(&NetParams, bf, 3) < 0)
	{
	   sprintf(tmpbuf, "%s: can't write options to net: %s\n", errmsg(0));
	   die (3, tmpbuf);
	}

}

static void
linein()                                /* Get a line into linbuf */
{
    register int c, ovflg;
    register char *linptr;
    int retflg;

	linptr = linbuf;
	retflg = ovflg = 0;

	for (;;)
	{
	    if(linptr >= (linbuf + LINSIZ - 2)) ovflg++;
	    c = netgetch();
	    if(cmdflg)
	    {
		cmdflg = 0;

		/* "Interesting" Commands */
		if(c == TNDM)
		{
		     extern tsturg();
		     synchno = tsturg(&NetParams);
		    linptr = linbuf;
		    ovflg = 0;
		    continue;
		}

		/* Other Telnet Commands */
		if(synchno == 0)
		{
		    switch(c)
		    {
			case TNEC:
			    if(linptr>linbuf)
			    {
				linptr--;
				ovflg = 0;
			    }
			    continue;
			case TNEL:
			    linptr = linbuf;
			    ovflg = 0;
			    continue;
			case TNDO:
			case TNWILL:
			    tpopt(c);
			    continue;
			case TNIAC:
			    if(ovflg == 0) *linptr++ = c;
		    }
		}
		continue;  /* Command ignored if synchno or not implemented */
	    }

	    if(c == TNIAC) cmdflg++;
	    else if(synchno == 0) switch(c)
	    {
		case '\r':
		    retflg++;
		    continue;
		case '\n':
		    *linptr++ = c;
		    *linptr++ = '\0';
		    return;
		case '\0':
		    if(retflg)
		    {
			if(ovflg == 0) *linptr++ = '\r';
			retflg = 0;
		    }
		    continue;
		default:
		    if(ovflg == 0) *linptr++ = c;
	    }
	}
	fprintf (stderr, "linbuf = %s\n", linbuf);
}

static int
iconv(s)
  char *s;
{
    register int k, i, c;

	k = 0;
	for (i = 0; i < 3; i++)
	{
	    c = s[i];
	    if(c < '0' || c > '9') return(0);
	    k = k*10 + c - '0';
	}
	if(s[3] == '-') return(0);      /* this is a continued comment */
	return(k);
}

static int
rget()
{
   errno = 0;
   linein();
   xreply = iconv(linbuf);
   fputs(linbuf, stderr);
   return xreply;
}

static int
rwait (n)
int n;
{
    register int rnum;

    while ((rnum = rget ()) < 200) {
	if (n < 0) break;		/* special hack to return low reply */
    }

    if (rnum == 421) byedie();

    if (rnum < 200 && n < 0) return 0;	/* hack for now. */

    switch (rnum / 100)
    {
	case 1:
	case 2:
	    return 1;

	case 3:
	case 4:
	case 5:
	    return 0;

	default:
	    exit (1);
    }
}

static void
abterr()
{
	register char *p, *q;

	uicount = 0;
	q = linbuf;
	p = aborstr;            /* <IAC> <IP> <IAC> <DM> ABOR */
   	while (*p)
		*q++ = *p++;
	linptr = q;
	urgon (&NetParams);       /* send INS to match DM */
	sndcmd();
	urgoff (&NetParams);       /* send INS to match DM */
	if (fdfil != NULL && fdfil != stdout)
	{
	    fclose (fdfil);
	    fdfil = NULL;
	}
	while (rwait (8) && xreply!= 225 && xreply!=226);
}

static void
putcmd (s)                       /* put in 4 chars of cmd */
  char *s;
{
    register char *p, *q;
    register int i;

     p = linbuf;
     q = s;
     for (i=0; *q && i<4; i++) *p++ = *q++;
     linptr = p;
}

void
logfn(s)			/* interfaces to ftp_lib, skips over reply */
char * s;
{
    printf (s+4);
}

static void
f_err()
{
    puts ("ftpmain: command parser ERROR; dispatch on 0 command pointer");
    puts ("please report this to BUG-FTP @ BBN-Unix");
}

static void
help()
{
 register struct ftucmd *sp;
 register i,j;
 char flag;

 flag = i = j = 0;
 if (*arg1==0)
 {
	printf ("\nCommands known to this process:\n\n");
	for (sp = cmdtab; (sp->ft_cnm); sp++)
	{
	       printf ("%10s", (sp->ft_cnm));
	       if (!(++j%6))printf ("\n");
	}
	printf ("\nany unambiguous substring will invoke the given command\n");
	printf ("for a brief description of a given command type \"help <command>\"\n");
	printf ("for a brief description of all commands type \"help all\", or \"help * \"\n");
	printf ("\"help server\" will ask the server process for help\n");
 }
 else {
	getuc (arg1);
	if (match (arg1, "SERVER"))
	{
		putcmd ("HELP");
		sndcmd();
		rwait (12);
		return;
	}
	else if (match (arg1, "ALL")||match (arg1, "*")) flag++;

	for (sp = cmdtab; (sp->ft_cnm); sp++)
	{
		if (flag||match (arg1, sp->ft_cnm))
		{
			printf ("%s\t %s\n", sp->ft_cnm, sp->ft_desc);
			i++;
		}
	}
	if (!i) printf ("command \"%s\" unrecognised\n", arg1);
 }
}

static void
putarg (s)                       /* put in argument with no space */
  char *s;
{
    register char *p, *q;
    p = linptr;     q = s;
    while (*q) *p++ = *q++;
    linptr = p;
}


static void
putstr (s)                       /* put in space, then arg */
  char *s;
{
    *linptr++ = ' ';
    putarg (s);
}

static void
setspec (what, spec, tabl)
char *what; int *spec;
struct spctab tabl[];
{
    register struct spctab *p;

	getuc (arg1);
	for (p=tabl; p->typnm; p++)
	{
	    if (match (arg1,p->typnm)) break;
	}

	if (!p->typarg)
	{
	   if (p->typnm)
	      sprintf (tmpbuf, "%s %s not implimented.\n", what, arg1);
	   else
	      sprintf (tmpbuf, "Bad %s: \"%s\"\n", what, arg1);
	   error (tmpbuf);
	}
	else
	{
	    putstr (p->typarg);
	    sndcmd();
	    if (rwait (2) == 0) return;
	    *spec = p->typnum;
	}
}

static void
f_mode()
{
	setspec ("Mode", &mode, mdcode);
}

static void
f_type()
{
	setspec ("Type", &type, tycode);
}

static void
f_stru()
{
	setspec ("Structure", &stru, strucode);
}

static void
f_pwd()
{
	putstr (arg1);
	sndcmd();
	rwait (2);
}

static void
f_help()
{
	putcmd ("HELP");
	putstr (arg1);
	sndcmd();
	rwait (5);
}

static void
f_stat()
{
	putcmd ("STAT");
	putstr (arg1);
	sndcmd();
	rwait (5);
}

static void
f_rena()
{
	putcmd ("RNFR");
	putstr (arg1);
	sndcmd();
	if (rwait (2), xreply != 350) return;
	putcmd ("RNTO");
	putstr (arg2);
	sndcmd();
	rwait (6);
}

static void
f_cwd ()
{
	putcmd ("CWD");
	putstr (arg1);
	sndcmd();
	rwait (3); 
}

static void
f_rate()
{
    printf("%s", dumptime());
    if (!strcmp("reset", arg1))
	inittime();
}

static void
f_quot()
{
	putcmd (arg1);
	if (arg2[0]) putstr (arg2);
	sndcmd();
	rwait (2);
}

static void
f_dele()
{
	putstr (arg1);
	sndcmd();
	rwait (7);
}


static void
f_user()
{
	putstr (arg1);
	sndcmd();
	rwait (1);
}

static void
f_log()
{
	putcmd ("USER");
	f_user();
	if (xreply == 331)
	    {
		putcmd ("PASS");
		strcpy (arg1,arg2);
		f_pass();
	    }
	if (xreply == 332)
	    {
		strcpy (arg1, arg3);
		f_acct();
	    }
}

static int
secret (s)
  char *s;
{
    struct sgttyb tmp;
    register int sav;
    register char *p;

	if (arg1[0] == '\0')
	{
	    ttcnt = 0;
	    gtty (1,&tmp);
	    sav = tmp.sg_flags;
	    tmp.sg_flags &= ~ECHO;
	    stty (1,&tmp);                /* Set noecho for password */
	    printf ("%s",s);
	    fflush(stdout);
	    p = getarg();
	    tmp.sg_flags = sav;
	    stty (1,&tmp);
	    printf ("\n");           /* Echo the absorbed newline */
	    if (p)
	    {
		if (movarg (p, arg1, ARGSIZ - 1) < 0)
	   		return -1;
		getarg();               /* Skip the newline */
	    }
	}
	putstr (arg1);
	sndcmd();
	rwait (1);
	return 0;
}

static void
f_pass()
{
	secret ("Password: ");
}

static void
f_acct()
{
	putcmd ("ACCT");
	secret ("Account #: ");
}

static int
putpasv(mode)
{
   char *bp, *np;
   int i, num;
   unsigned short port;

   putcmd ("PASV");
   sndcmd();
   if (rwait (2) == 0)
      return 0;

   bp = linbuf+4;
   while (*bp && *bp != '(') bp++;
   bp++;

   np = (char *)&DataParams.ns.n_fcon;
   num = 0;
   for (i = 0; i < 4; i++)
   {
      for (; *bp && isdigit (*bp); bp++)
         num = (num * 10) + (*bp - '0');
      bp++;
      *np++ = num & 0xFF;
      num = 0;
   }

   np = (char *)&DataParams.ns.n_fport;
   num = 0;
   for (i = 0; i < 2; i++)
   {
      while (*bp && isdigit (*bp))
         num = (num * 10) + (*bp++ - '0');
      bp++;
      *np++ = num & 0xFF;
      num = 0;
   }
   return 1;
}

static int
fetch (cmd, src, dest)
char *cmd, *src, *dest;
{
	int flag = 0;
	int tfd;

	/* First find out if the destination file already existed */
	if (dest[0])
	{
	   if ((tfd = open (dest, 1)) == -1)
	       flag = 1;	/* Delete on error */
	   else
	   {
	       close (tfd);
	       flag = 0;	/* Don't delete on error, it already existed */
	   }

	   if ((fdfil = fopen (dest, "w")) == NULL)
	   {
	      sprintf (tmpbuf, "Can't create %s: %s\n", dest, errmsg (0));
	      error (tmpbuf);
	      return -1;
	   }
	}
	else
	{
	   fdfil = stdout;
	}

	if (putpasv() == 0)
	    goto closeall;

	errno = 0;

	putcmd (cmd);
	putstr (src);
	sndcmd();

	if (!chekds (0))
	{
	    rwait (-1);
	    rcvdata (fdfil);
	    net_close (&DataParams);
	    if (fdfil != stdout)
	       fclose (fdfil);
	    fdfil = NULL;
	    return rwait (4);
	}

closeall:
	if (fdfil != stdout)
	{
	   fclose (fdfil);
	   if (flag)
	       unlink (dest);
	}
	fdfil = NULL;
	net_close (&DataParams);
	if (errno == EINTR) abterr();
	return 0;
}

static void
f_nlst()
{
	fetch ("NLST", arg1, arg2);
}


static void
f_list()
{
	fetch ("LIST", arg1, arg2);
}

static void
f_retr()
{
	fetch ("RETR", arg1, arg2);
}

static int
sendopen (fname)
  char *fname;
{
  if ((fdfil = fopen (fname, "r")) == NULL)
  {
     sprintf (tmpbuf, "can't open %s: %s\n", fname, errmsg (0));
     error (tmpbuf);
     return -1;
  }

  return 0;
}

static void
snddat (c, s)
  char *c, *s;
{
    int child;

    if (putpasv() == 0)
	return;

    errno = 0;
    putcmd (c);
    putstr (s);
    sndcmd ();

    if (chekds (1))
    {
	 fclose (fdfil);
	 fdfil = NULL;
	 net_close (&DataParams);
	 if (errno == EINTR) 
	     abterr ();
    }
    else 
    {
       rwait (-1);
       senddata (fdfil);
       net_close(&DataParams);
       rwait (4);
    }
    if (fdfil != NULL)
    {
       fclose (fdfil);
       fdfil = NULL;
    }
}

static void
f_stor()
{
    if (sendopen (arg1, 0) < 0)
      return;
    snddat ("STOR", arg2);
}


static void
f_appe()
{
	if (sendopen (arg1, 0) < 0)
	   return;
	snddat ("APPE", arg2);
}

static void
byedie()
{
     putcmd (QUIT);
     sndcmd();
     rwait (1);
     printf("%s", dumptime());
     die (0, NULL);
}

/* User Telnet and FTP - modified for Illinois NCP from old Rand user telnet.
 *
 * Note: BBN-UNIX only uses it to invoke User FTP. User Telnet is now a
 * completely different program. Dan Franklin (BBN)
 *
 * Changed to use long host numbers jsq BBN 3-27-79.
 *
 * Leave signals alone: child will set according to what parent had set
 *  jsq BBN 5Aug79
 *
 * Pass arguments to both sides of ftp:  pid of other side, name of other
 *  host, name of this host.  All host name-number conversions are now in this
 *  process, and children can now mop up on each other when either one dies.
 *
 * Changed to flush the telnet code, as we don't use it any more dm 3-19-80
 *
 * jsq BBN 5April80 reformatted using indent, put proper names for files
 * to exec back.
 *
 * changed to work with NCP or TCP dm 11-19-80
 */

static void 
sigcatcher (sig)
int sig;
{
   if (NetParams.fds)
      close (NetParams.fds);
   if (DataParams.fds)
      close (DataParams.fds);
   exit(0);
}

int
main (argc, argv)
int   argc;
char *argv[];
{
  register struct ftucmd *sp;
  int badhost;
  portsock socket;
  char  *host;
  netaddr hnum;
  register int count;

  mode = MODES;
  type = TYPEA;
  stru = STRUF;

  signal (SIGQUIT, sigcatcher);
  signal (SIGINT, sigcatcher);
  socket = (portsock)0;
  if (argc > 1)
  {
	host = argv[1];
	hnum.s_addr = gethost(host);
	badhost = isbadhost(hnum);
	if (badhost)
		printf ("Unknown host name: \"%s\"\n", host);
	if (argc > 2) socket = ATOSOCK (argv[2]);
  }

  while (badhost)
  {
	printf ("Host: ");
	fflush (stdout);
	if (fgets (tmpbuf, sizeof tmpbuf, stdin) == NULL) exit (1);
	count = strlen (tmpbuf);
	if (tmpbuf[count-1] == '\n') tmpbuf[count-1] = 0;
	while (count-- >= 0)
	    if (tmpbuf[count] == '?')
		{
			printf("use host(1) or prhost(1) to find host names\n");
		    break;
		}
	if ((count < 0) && (tmpbuf[0]))
	{
		hnum .s_addr= gethost(tmpbuf);
		badhost = isbadhost(hnum);
		if (badhost)
			printf ("Unknown host name: \"%s\"\n", tmpbuf);
		else host = tmpbuf;
	}
  }

  printf ("Trying %s (%s)\n", host, inet_ntoa(hnum.s_addr));
  if (net_open(&NetParams, hnum, (socket?socket:(portsock)FTPSOCK)) < 0)
  {
     printf ("Cannot connect: %s\n", errmsg (0));
     exit (1);
  }

  printf ("Connections established.\n\n");
  myname = (char *)thisname ();
  myip = gethost (myname);
  them = (char *)hostname (hnum.s_addr);
  strcpy ((HOST_ID = (char *)malloc(strlen(them)+1)), them);
  getuc (HOST_ID);

  NetInit (&DataParams);
  get_stuff (&NetParams);
  inittime();
  ttcnt = 0;
  ttptr = ttbuf;

  rwait (1);			/* wait for 200 hello */

  for (;;)
  {
     if (uicount) abterr();
     prompt();
     if ((sp = getcmd()) != NULL) {
	putcmd (sp->ft_cnm);
	(*sp->ft_fn)();
     }
  }
}

