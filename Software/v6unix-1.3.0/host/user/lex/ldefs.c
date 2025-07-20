# include <stdio.h>
# define PP 1
# ifdef UNIX

# define CWIDTH 7
# define CMASK 0177
# define ASCII 1
# endif

# ifdef GCOS
# define CWIDTH 9
# define CMASK 0777
# define ASCII 1
# endif

# ifdef IBM
# define CWIDTH 8
# define CMASK 0377
# define EBCDIC 1
# endif

# ifdef ASCII
# define NCH 128
# endif

# ifdef EBCDIC
# define NCH 256
# endif


# define DEFSIZE 40
# define STARTCHAR 100
# define STARTSIZE 256
# define CCLSIZE 1000

# ifdef SMALL
# define TOKENSIZE 512
# define DEFCHAR 512
# define TREESIZE 600
# define NSTATES 300
# define MAXPOS 1500
# define NTRANS 1000
# define NOUTPUT 1000
# endif
# ifndef SMALL
# define TOKENSIZE 1000
# define DEFCHAR 1000
# define TREESIZE 1000
# define NSTATES 500
# define MAXPOS 2500
# define NTRANS 2000
# define NOUTPUT 3000
# endif

# define NACTIONS 100
# define ALITTLEEXTRA 30

# define RCCL NCH+90
# define RNCCL NCH+91
# define RSTR NCH+92
# define RSCON NCH+93
# define RNEWE NCH+94
# define FINAL NCH+95
# define RNULLS NCH+96
# define RCAT NCH+97
# define STAR NCH+98
# define PLUS NCH+99
# define QUEST NCH+100
# define DIV NCH+101
# define BAR NCH+102
# define CARAT NCH+103
# define S1FINAL NCH+104
# define S2FINAL NCH+105

# define DEFSECTION 1
# define RULESECTION 2
# define ENDSECTION 5
# define TRUE 1
# define FALSE 0

# define PC 1
# define PS 1

# ifdef DEBUG
# define LINESIZE 110
extern int yydebug;
extern int debug;		/* 1 = on */
extern int charc;
# endif

# ifndef DEBUG
# define freturn(s) s
# endif

#define cfree(a,b,c) free((a))

extern int sargc;
extern char **sargv;
extern char buf[520];
extern int ratfor;		/* 1 = ratfor, 0 = C */
extern int yyline;		/* line number of file */
extern int sect;
extern int eof;
extern int lgatflg;
extern int divflg;
extern int funcflag;
extern int pflag;
extern int casecount;
extern int chset;	/* 1 = char set modified */
extern FILE *fin, *fout, *fother, *errorf;
extern int fptr;
extern char *ratname, *cname;
extern int prev;	/* previous input character */
extern int pres;	/* present input character */
extern int peek;	/* next input character */
extern int *name;
extern long *left;
extern long *right;
extern int *parent;
extern char *nullstr;
extern int tptr;
extern char pushc[TOKENSIZE];
extern char *pushptr;
extern char slist[STARTSIZE];
extern char *slptr;
extern char **def, **subs, *dchar;
extern char **sname, *schar;
extern char *ccl;
extern char *ccptr;
extern char *dp, *sp;
extern int dptr, sptr;
extern char *bptr;		/* store input position */
extern char *tmpstat;
extern int count;
extern int **foll;
extern int *nxtpos;
extern int *positions;
extern int *gotof;
extern int *nexts;
extern char *nchar;
extern int **state;
extern int *sfall;		/* fallback state num */
extern char *cpackflg;		/* true if state has been character packed */
extern int *atable, aptr;
extern int nptr;
extern char symbol[NCH];
extern char cindex[NCH];
extern int xstate;
extern int stnum;
extern int ctable[];
extern int ZCH;
extern int ccount;
extern char match[NCH];
extern char extra[NACTIONS];
extern char *pcptr, *pchar;
extern int pchlen;
extern int nstates, maxpos;
extern int yytop;
extern int report;
extern int ntrans, treesize, outsize;
extern long rcount;
extern int optim;
extern int *verify, *advance, *stoff;
extern int scon;
extern char *psave;
extern char *myalloc();
extern int buserr(), segviol();

/* header.c */
extern void phead1();
extern void chd1();
extern void rhd1();
extern void phead2();
extern void chd2();
extern void ptail();
extern void ctail();
extern void rtail();
extern void statistics();

/* sub1.c */
extern char *getl(char *p);
extern int space(int ch);
extern int digit(int c);
extern int pindex(int a,char *s);
extern int alpha(int c);
extern int printable(int c);
extern void lgate();
extern void scopy(char *s,char *t);
extern int siconv(char *t);
extern int slength(char *s);
extern int scomp(char *x,char *y);
extern int ctrans(char **ss);
extern void cclinter(int sw);
extern int usescape(int c);
extern int lookup(char *s,char **t);
extern int cpyact();
extern int gch();
extern int mn2(int a,int d,int c);
extern int mn1(int a,long d);
extern int mn0(int a);
extern void munput(int t,char *p);
extern int dupl(int n);
extern void allprint(char c);
extern void strpt(char *s);
extern void sect1dump();
extern void sect2dump();
extern void treedump();

/* sub2.c */
extern void cfoll(int v);
extern void pfoll();
extern void add(int **array,int n);
extern void follow(int v);
extern void first(int v);
extern void cgoto();
extern void nextstate(int s,int c);
extern int notin(int n);
extern void packtrans(int st,char *tch,int *tst,int cnt,int tryit);
extern void pstate(int s);
extern int member(int d,char *t);
extern void stprt(int i);
extern void acompute(int s);
extern void pccl();
extern void mkmatch();
extern void layout();
extern void rprint(int *a,char *s,int n);
extern void shiftr(int *a, int n);
extern void upone(int *a,int n);
extern void bprint(char *a,char *s,int n);
extern void padd(int **array,int n);

extern void yyerror();

extern void warning_(char *s);
#define warning(s...) {		\
	char __wb[128];		\
	snprintf(__wb,128,s);	\
	warning_(__wb);		\
	}

extern void error_(char *s);
#define error(s...) {		\
	char __wb[128];		\
	snprintf(__wb,128,s);	\
	error_(__wb);		\
	}
