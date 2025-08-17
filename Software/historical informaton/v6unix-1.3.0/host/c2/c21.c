/*
 * C object code improver-- second part
 */
#include "c2.h"

void	singop();
void	dualop();
int	isreg();
int	findrand();
int	equstr();
void	repladdr();
void	forget();
void	savereg();
int	chkauto();
void	setcc();
int	natural();

/* Perform peephole optimization on the instruction stream */
void rmove()
{
	register struct node *p;
	register int r;
	register int r1;

	for (p=first.forw; p!=0; p = p->forw) {
		switch (p->op) {

		case MOV:
			/* Don't attempt special optimizations on byte moves */
			if (p->subop==BYTE)
				goto dble;
			dualop(p);

			/* If the source is already in the destination register eliminate the MOV,
			 * unless the move was necessary to set the flags (conditional jump or call
			 * to sign extend).
			 */
			if ((r = findrand(regs[RT1])) >= 0) {
				if (r == isreg(regs[RT2]) && p->forw->op!=CBR 
					&& (p->forw->op!=BL || !equstr(p->forw->code, "@sext")) ) {
					p->forw->back = p->back;
					p->back->forw = p->forw;
					redunm++;
					nchange++;
					continue;
				}
			}
			/* Simplify the source addressible if possible */
			repladdr(p, 0);
			r  = isreg(regs[RT1]);
			r1 = isreg(regs[RT2]);
			forget(regs[RT2]);
			/* If the destination is a register, memoize the source value; else if
			 * the source is a register, update it to an alias for the destination
			 */
			if (r1 >= 0)
				savereg(r1, (r>=0) ? regs[r] : regs[RT1]);
			else if (r >= 0)
				savereg(r, regs[RT2]);
			/* destroy memoized source register value if it is autoincremented */
			chkauto(regs[RT1]);
			/* memoize abtract condition codes */
			setcc(regs[RT2]);
			continue;

		case A:
		case S:
		case SOC:
		case SZC:
		case XOR:
		case MPY:
		case DIV:
dble:
			dualop(p);
			repladdr(p, 0);
			chkauto(regs[RT1]);
			forget(regs[RT2]);
			/* for DIV and MPY forget destination Rn+1 as well */
			if ((p->op==DIV || p->op==MPY) && (r = isreg(regs[RT2]))>=0) {
				char tmp[10];
				sprintf(tmp,"r%d",r+1);
				forget(tmp);

			}
			switch	(p->op)
				{
				case	A:
				case	S:
				case	SZC:
				case	SOC:
					setcc(regs[RT2]);
					if (p->subop!=BYTE) break;

				default:
					ccloc[0] = 0;
				}
			continue;

		case LI:
			dualop(p);
			forget(regs[RT1]);
			savereg(isreg(regs[RT1]), regs[RT2]);
			setcc(regs[RT1]);
			/* replace 'li Rn,-1' with 'seto Rn' */
			if (equstr(regs[RT2],"-1") || equstr(regs[RT2],"65535")) {
				p->op = SETO;
				p->code = copy(1, regs[RT1]);
				nlit++;
				nchange++;
			}
			continue;
		
		case AI:
		case ANDI:
		case ORI:
		case CI:
			dualop(p);
			forget(regs[RT1]);
			if (p->op==CI)
				ccloc[0] = 0;
			else
				setcc(regs[RT1]);
			continue;

		case C:
		case COC:
		case CZC:
			dualop(p);
			chkauto(regs[RT1]);
			chkauto(regs[RT2]);
			repladdr(p, 1);
			ccloc[0] = 0;
			continue;

		case STCR:
		case SLA:
		case SRA:
		case SRC:
		case SRL:
			dualop(p);
			forget(regs[RT1]);
			ccloc[0] = 0;
			continue;

		case MPYS:
		case DIVS:
			singop(p);
			chkauto(regs[RT1]);
			repladdr(p, 0);
			forget("r0");
			forget("r1");
			ccloc[0] = 0;
			continue;
	
		case CLR:
		case SETO:
		case INV:
		case NEG:
		case ABS:
		case SWPB:
		case INC:
		case INCT:
		case DEC:
		case DECT:
		case STST:
		case STWP:
			singop(p);
			forget(regs[RT1]);
			if ((r = isreg(regs[RT1])) >= 0) {
				if (p->op==CLR)
					savereg(r, "0");
				else if (p->op==SETO)
					savereg(r, "65535");
			}
			switch (p->op) {
			case INV:
			case NEG:
			case INC:
			case INCT:
			case DEC:
			case DECT:
				setcc(regs[RT1]);
				break;
			default:
				ccloc[0] = 0;			
			}
			continue;

		case LST:
		case TB:
			/* These two only affect condition codes */
			if (p->op==TB)
				ccloc[0] = 0;
			/* fall through */

		case LDCR:
		case SBO:
		case SBZ:
		case LIMI:
		case RSET:
		case CKON:
		case CKOF:
		case LREX:
		case IDLE:
			/* register state unaffected */
			continue;

		case CBR:
		case BJMP:
			/* for now do not attempt peephole jump optimization, as the
			 * benefits seem small compared to the risk of mis-optimization.
			 */
			continue; 

		case 0:
			/* Unrecognized (unparsed) instructions, assignments (~foo=r2), and
			 * data arrive here.  In order to prevent throwing away information
			 * about register contents just because a local assignment is done
			 * we check for the first character being a tilde.
			 */
			if (! p->code || p->code[0] != '~')
				clearreg();
			continue;

		case EROU:
		case SWB:
		case TEXT:
		case DATA:
		case BSS:
		case EVEN:
		case END:
			/* These operations do not affect the register state. Note that an
			 * earlier pass has moved all text, etc. sections together.
			 */
			continue;

		default:
			fprintf(stderr, "Unhandled opcode in rmove (%d)\n", p->op);
			
		case BL:
		case BLWP:
		case B:
		case X:
		case XOP:
		case SYS:
		case LABEL:
		case DLABEL:
		case LWP:
		case LWPI:
			/* Following these instructions we can make no assumptions
			 * about our state, so start afresh.
			 */
			clearreg();
		}
	}
}

/* clear out memoized values of all registers */
void clearreg()
{
	register int i;

	for (i=0; i<NREG; i++)
		regs[i][0] = '\0';
	conloc[0] = 0;
	ccloc[0] = 0;
}

/* Memoize the value assigned to register ai. If the source is
 * "(Rn)+", don't memoize. If the source is indirect, i.e. "(Rn)" or
 * "@xxx(Rn)" also don't bother, unless it is a local, i.e. @xxx(bp).
 * Hence, memoized are:
 * - constants
 * - registers
 * - locals & parameters
 */
void savereg(ai, as)
int ai;
char *as;
{
	register char *p, *s, *sp;

	sp = p = regs[ai];
	s = as;
	if (chkauto(s))
		return;
	while ((*p++ = *s) != 0) {
		if (s[0]=='(' && s[1]=='r') {
			*sp = 0;
			return;
		}
		if (*s++ == ',')
			break;
	}
	*--p = '\0';
}

/* Destroy the memoized values if a register is modified, i.e. if:
 * (i) the destination auto-increments a register
 * (ii) the destination is a register
 * (iii) a register refers to the destination
 */
void forget(dst)
	register char *dst;
{
	register int i;

	chkauto(dst);
	if ((i = isreg(dst)) >= 0)
		regs[i][0] = 0;
	while ((i = findrand(dst)) >= 0)
		regs[i][0] = 0;
}

/* Place operand in RT1, set RT2 to nil */
void singop(ap)
struct node *ap;
{
	register char *p1, *p2;

	p1 = ap->code;
	p2 = regs[RT1];
	while ((*p2++ = *p1++) != 0);
	regs[RT2][0] = 0;
}

/* Separate operands, place in RT1 and RT2 */
void dualop(ap)
struct node *ap;
{
	register char *p1, *p2;
	register struct node *p;

	p = ap;
	p1 = p->code;
	p2 = regs[RT1];
	if (p1)
		while (*p1 && *p1!=',')
			*p2++ = *p1++;
	*p2++ = 0;
	p2 = regs[RT2];
	*p2 = 0;
	if (! p1)
		return;
	if (*p1++ !=',')
		return;
	while (*p1==' ' || *p1=='\t')
		p1++;
	while ((*p2++ = *p1++) != 0)
		;
}

/* find the register that holds abstract value 'as' */
int findrand(as)
char *as;
{
	register int i;

	for (i = 0; i<NREG; i++) {
		if (equstr(regs[i], as))
			return(i);
	}
	return(-1);
}

/* If as refers to a register return its number, else return -1 */
int isreg(as)
char *as;
{
	register char *s;

	s = as;
	if (s[0]=='r') {
		if (s[2]==0 && s[1]>='0' && s[1]<='9')
			return(s[1]-'0');
		if (s[1]=='1' && s[3]==0 && s[2]>='0' && s[2]<='3')
			return(s[2]-'0'+10);
	}
	return(-1);
}

/* Check that the list has not become circular */
void check()
{
	register struct node *p, *lp;
	register int count;

	lp = &first;
	count = 0;
	for (p=first.forw; p!=0; p = p->forw) {
		if (++count > 10000)
			exit(1);
		if (p->back != lp)
			exit(1);
		lp = p;
	}
}

/* If source operand ap modifies its register, destroy its memoized value */
int chkauto(ap)
char *ap;
{
	register char *p1, *p2;
	register int r;

	p1 = ap;
	p2 = p1;
	if (*p1==0)
		return(0);
	while (*p2++);
	if (*p1=='(' && *(p2-3)==')' && *(p2-2)=='+') {
		while (*p1 && *p1++!='r');
		if (*p1=='1' && p1[1]>='0' && p1[1]<='3') {
			r  = p1[1] -'0' + 10;
			regs[r][0] = 0;			
		}
		if (*p1>='0' && *p1<='9') {
			r  = *p1 -'0';
			regs[r][0] = 0;
		}
		return(1);
	}
	return(0);
}

/* If the addressible in RT1 is already in a register, simplify the
 * addresible to that register. If f is non-zero also make such replacement
 * for RT2 (used for instance with 'C <src>,<dst>').
 */
void repladdr(p, f)
struct node *p;
int f;
{
	register int r;
	int r1;
	register char *p1, *p2;
	static char rt1[50], rt2[50];

	/* Is there a replacable addressible ? */
	if (f)
		r1 = findrand(regs[RT2]);
	else
		r1 = -1;
	r = findrand(regs[RT1]);

	/* If so, make the replacement */
	if (r>=0 || r1>=0) {

		p2 = regs[RT1];
		for (p1 = rt1; (*p1++ = *p2++) != 0; );
		if (regs[RT2][0]) {
			p1 = rt2;
			*p1++ = ',';
			for (p2 = regs[RT2]; (*p1++ = *p2++) != 0; );
		} else
			rt2[0] = 0;

		if (r>=0) {
			rt1[0] = 'r';
			if (r<10) {
				rt1[1] = r + '0';
				rt1[2] = 0;
			} else {
				rt1[1] = '1';
				rt1[2] = r - 10 + '0';
				rt1[3] = 0;				
			}
			nsaddr++;
			nchange++;
		}
		if (r1>=0) {
			if (r<10) {
				rt2[1] = r1 + '0';
				rt2[2] = 0;
			} else {
				rt2[1] = '1';
				rt2[2] = r1 - 10 + '0';
				rt2[3] = 0;				
			}
			nsaddr++;
			nchange++;
		}
		p->code = copy(2, rt1, rt2);
	}
}

/* Compare strings ap1 and ap2 for equality, returning 0 or 1 */
int equstr(ap1, ap2)
char *ap1, *ap2;
{
	register char *p1, *p2;

	p1 = ap1;
	p2 = ap2;
	do {
		if (*p1++ != *p2)
			return(0);
	} while (*p2++);
	return(1);
}

/* Is the addressible ap 'natural', i.e. of the form
 * 'r' or '@nnn(bp)'? If so, it is considered non-volatile
 * and unaliassed; hence, it can be optimised.
 */
int natural(ap)
char *ap;
{
	register char *p;

	p = ap;
	if (*p=='r' )
		return(1);
	while (*p++);
	if (p[-2] == '+' || (p[-2] ==')' && p[-4] != 'b'))
		return(0);
	return(1);
}

/* If the destination is stable, it as the addresible that has
 * last been compared to zero, setting the condition codes.
 */
void setcc(ap)
char *ap;
{
	register char *p, *p1;

	p = ap;
	if (!natural(p)) {
		ccloc[0] = 0;
		return;
	}
	p1 = ccloc;
	while ((*p1++ = *p++) != 0);
}



