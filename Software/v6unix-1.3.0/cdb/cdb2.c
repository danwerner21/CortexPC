/*
 * C debugger -- part 2
 */

char	ssymbol[];
int	dotinc;
int	dot;


psymoff(v, lim)
{
	register char *w;

	w = vallook(v);
	if (w > lim) {
		printf("%d", v);
		return;
	}
	printf("%.8s", ssymbol);
	if (w)
		printf("+%d", w);
}

#define	ISP	1
#define TYPE1	1
#define TYPE2	2
#define TYPE3	3
#define TYPE4	4
#define TYPE5	5
#define TYPE6	6
#define TYPE7	7
#define TYPE8	8
#define TYPE9	9
#define TYPE10	10
#define TYPE11	11
#define TYPE12	12

struct optab {
	char	*iname;
	int	val;
	int	mask;
	int	itype;
} optab[] = {
	{ "a",      0xa000, 0x0fff, TYPE1 },
	{ "ab",     0xb000, 0x0fff, TYPE1 },
	{ "abs",    0x0740, 0x003f, TYPE3 },
	{ "ai",     0x0220, 0x000f, TYPE5 },
	{ "andi",   0x0240, 0x000f, TYPE5 },
	{ "b",      0x0440, 0x003f, TYPE3 },
	{ "bl",     0x0680, 0x003f, TYPE3 },
	{ "blwp",   0x0400, 0x003f, TYPE3 },
	{ "c",      0x8000, 0x0fff, TYPE1 },
	{ "cb",     0x9000, 0x0fff, TYPE1 },
	{ "ci",     0x0280, 0x000f, TYPE5 },
	{ "ckof",   0x03c0, 0x0000, TYPE9 },
	{ "ckon",   0x03a0, 0x0000, TYPE9 },
	{ "clr",    0x04c0, 0x003f, TYPE3 },
	{ "coc",    0x2000, 0x03ff, TYPE2 },
	{ "czc",    0x2400, 0x03ff, TYPE2 },
	{ "dec",    0x0600, 0x003f, TYPE3 },
	{ "dect",   0x0640, 0x003f, TYPE3 },
	{ "div",    0x3c00, 0x03ff, TYPE2 },
	{ "divs",   0x0180, 0x003f, TYPE3 },
	{ "idle",   0x0340, 0x0000, TYPE9 },
	{ "inc",    0x0580, 0x003f, TYPE3 },
	{ "inct",   0x05c0, 0x003f, TYPE3 },
	{ "inv",    0x0540, 0x003f, TYPE3 },
	{ "jeq",    0x1300, 0x00ff, TYPE6 },
	{ "jgt",    0x1500, 0x00ff, TYPE6 },
	{ "jh",     0x1b00, 0x00ff, TYPE6 },
	{ "jhe",    0x1400, 0x00ff, TYPE6 },
	{ "jl",     0x1a00, 0x00ff, TYPE6 },
	{ "jle",    0x1200, 0x00ff, TYPE6 },
	{ "jlt",    0x1100, 0x00ff, TYPE6 },
	{ "jmp",    0x1000, 0x00ff, TYPE6 },
	{ "jnc",    0x1700, 0x00ff, TYPE6 },
	{ "jne",    0x1600, 0x00ff, TYPE6 },
	{ "jno",    0x1900, 0x00ff, TYPE6 },
	{ "joc",    0x1800, 0x00ff, TYPE6 },
	{ "jop",    0x1c00, 0x00ff, TYPE6 },
	{ "ldcr",   0x3000, 0x03ff, TYPE2 },
	{ "li",     0x0200, 0x000f, TYPE5 },
	{ "limi",   0x0300, 0x0000, TYPE7 },
	{ "lrex",   0x03e0, 0x0000, TYPE9 },
	{ "lst",    0x0080, 0x000f, TYPE4 },
	{ "lwp",    0x0090, 0x000f, TYPE4 },
	{ "lwpi",   0x02e0, 0x0000, TYPE7 },
	{ "mov",    0xc000, 0x0fff, TYPE1 },
	{ "movb",   0xd000, 0x0fff, TYPE1 },
	{ "mpy",    0x3800, 0x03ff, TYPE2 },
	{ "mpys",   0x01e0, 0x003f, TYPE3 },
	{ "neg",    0x0500, 0x003f, TYPE3 },
	{ "ori",    0x0260, 0x000f, TYPE5 },
	{ "rset",   0x0360, 0x0000, TYPE9 },
	{ "rtwp",   0x0380, 0x0000, TYPE9 },
	{ "s",      0x6000, 0x0fff, TYPE1 },
	{ "sb",     0x7000, 0x0fff, TYPE1 },
	{ "sbo",    0x1d00, 0x00ff, TYPE10 },
	{ "sbz",    0x1e00, 0x00ff, TYPE10 },
	{ "seto",   0x0700, 0x003f, TYPE3 },
	{ "sla",    0x0a00, 0x00ff, TYPE8 },
	{ "soc",    0xe000, 0x0fff, TYPE1 },
	{ "socb",   0xf000, 0x0fff, TYPE1 },
	{ "sra",    0x0800, 0x00ff, TYPE8 },
	{ "src",    0x0b00, 0x00ff, TYPE8 },
	{ "srl",    0x0900, 0x00ff, TYPE8 },
	{ "stcr",   0x3400, 0x03ff, TYPE2 },
	{ "stst",   0x02c0, 0x000f, TYPE4 },
	{ "stwp",   0x02a0, 0x000f, TYPE4 },
	{ "swpb",   0x06c0, 0x003f, TYPE3 },
	{ "sys",    0x2c60, 0x0000, TYPE11},
	{ "szc",    0x4000, 0x0fff, TYPE1 },
	{ "szcb",   0x5000, 0x0fff, TYPE1 },
	{ "tb",     0x1f00, 0x00ff, TYPE10 },
	{ "x",      0x0480, 0x003f, TYPE3 },
	{ "xop",    0x2c00, 0x03ff, TYPE2 },
	{ "xor",    0x2800, 0x03ff, TYPE2 },
	{ "???",    0x0000, 0xffff, TYPE12 }
};

char *systab[] = {
	"indir",
	"exit",
	"fork",
	"read",
	"write",
	"open",
	"close",
	"wait",
	"creat",
	"link",
	"unlink",
	"exec",
	"chdir",
	"time",
	"mknod",
	"chmod",
	"chown",
	"break",
	"stat",
	"seek",
	"getpid",
	"mount",
	"umount",
	"setuid",
	"getuid",
	"stime",
	"ptrace",
	"27",
	"fstat",
	"29",
	"smdate",
	"stty",
	"gtty",
	"33",
	"nice",
	"sleep",
	"sync",
	"kill",
	"switch",
	"39",
	"40",
	"dup",
	"pipe",
	"times",
	"profil",
	"45",
	"setgid",
	"getgid",
	"signal",
	"49",
	"50",
	"51",
	"52",
	"53",
	"54",
	"55",
	"56",
	"57",
	"58",
	"59",
	"60",
	"61",
	"62",
	"63",
};

char	*regname[] = { "r0", "r1", "r2",  "r3",  "r4",  "r5",  "r6", "r7",
		       "r8", "r9", "r10", "r11", "r12", "r13", "bp", "sp"};

printins(f)
{
	register ins, w;
	register struct optab *p;

	dotinc = 2;
	ins = cget(dot, ISP);
	if (vallook(dot)==0)
		printf("%.8s:", ssymbol);
	printf("\t");
	for (p=optab;; p++)
		if ((ins & ~p->mask) == p->val)
			break;
	printf("%s", p->iname);
	switch (p->itype) {

	case TYPE1:
		if (ins>0)
			printf("b");
		printf("\t");
		paddr(ins);
		printf(",");
		paddr(ins>>6);
		return;

	case TYPE2:
		printf("\t");
		paddr(ins);
		printf(",");
		paddr((ins>>6)&017);
		return;

	case TYPE3:
		printf("\t");
		paddr(ins);
		return;

	case TYPE4:
		printf("\t");
		paddr(ins&017);
		return;

	case TYPE5:
		printf("\t");
		paddr(ins&017);
		printf(",");
		psymoff(cget(dot+dotinc, ISP), 010000);
		dotinc += 2;
		return;

	case TYPE6:
		printf("\t");
		ins &= 0377;
		if (ins&0200)
			ins |= 0177400;
		ins = dot + (ins<<1) + 2;
		psymoff(ins, 010000);
		return;

	case TYPE7:
		printf("\t");
		psymoff(cget(dot+dotinc, ISP), 010000);
		dotinc += 2;
		return;

	case TYPE8:
		printf("\t%d,",(ins&0160)>>4);
		paddr(ins&017);
		return;

	case TYPE9:
	case TYPE12:
		return;

	case TYPE10:
		ins &= 0377;
		printf("\t%d", ins);
		return;
		
	case TYPE11:
		printf("\t%s", systab[cget(dot+dotinc, ISP)]);
		dotinc += 2;
		return;

	default:
		printf("\t%.1o", ins);
	}
}

paddr(aa)
{
	register a, r;

	a = aa;
	r = a&017;
	a &= 060;
	if (r==0 && a==040) {
		printf("@");
		psymoff(cget(dot+dotinc, ISP), 010000);
		dotinc += 2;
		return;
	}
	r = regname[r];
	switch (a) {
	/* r */
	case 000:
		printf("%s", r);
		return;

	/* (r) */
	case 020:
		printf("(%s)", r);
		return;

	/* @x(r) */
	case 040:
		printf("@");
		psymoff(cget(dot+dotinc, ISP), 010000);
		dotinc += 2;
		printf("(%s)", r);
		return;

	/* (r)+ */
	case 060:
		printf("(%s)+", r);
		return;
	}
}
