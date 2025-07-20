/*
 *		Assembler Machine Instruction Processing
 */

#include "as.h"

/* evaluate an expression which is expected to return
   a 4-bit absolute value */
static int regexpr()
{
	int ret, r;

	ret = 0;
	r = expr();
	if (pass && (r != RABS || (ret = res.val) & ~0xf))
		cerror(errv);
	return(res.val);
}

static void skip_comma()
{
	int t;

	while ((t=token())==SPACE) ;
	if (t!=COMMA)
		cerror(errs);
}

#define WR	0x00   /* register: R */
#define WRI	0x10   /* register indirect: (R) */
#define WRIA 	0x30   /* register indirect, autoincrement: (R)+ */
#define MD	0x20   /* memory direct: @M or indexed @M(R) */
#define MASK	0x30   /* mask for mode bits */

static EXPR sav_res;

/* R, (R), (R)+, @abs, @abs(R) */
static int full_reg()
{
	int t, bits;

	t = token();

	if (t==LPAREN) {
		bits = regexpr();
		if (token()!=RPAREN)
			cerror(errs);
		if ((t=token())==PLUS) {
			bits |= WRIA;
		} else {
			bits |= WRI;
			nexttoken = t;
		}
		return bits;
	}

	if (t==AT) {
		expr();
		bits = MD;
		if ((t=token())==LPAREN) {
			sav_res = res;
			bits |= regexpr();
			res = sav_res;
			if (bits==0)
				cerror(errv);
			if (token()!=RPAREN)
				cerror(errs);
		} else {
			nexttoken = t;
		}
		return bits;
	}

	nexttoken = t;
	bits = regexpr() | WR;
	return bits;
}

/* format 1: Ts, S, Td, D */
void opc_1(opc)
	int opc;
{
	EXPR tmp;
	
	opc |= full_reg();
	tmp = res;
	skip_comma();
	opc |= (full_reg()<<6);
	putwd(opc, RABS);
	if ((opc & MASK)==MD)
		putwd(tmp.val, tmp.rel);
	if ((opc & (MASK<<6))==(MD<<6))
		putwd(res.val, res.rel);
	return;
}

/* format 2: S, Td, D */
void opc_2(opc)
	int opc;
{
	EXPR tmp;

	/* handle opcode 'sys syscall', which is 'xop @syscall,1' */
	if (opc==0x2c60) {
		if (expr() != RABS) cerror(errv);
		putwd(opc, RABS);
		putwd(res.val, RABS);
		return;
	}
	/* handle normal format 2 opcodes */
	opc |= full_reg();
	tmp = res;
	skip_comma();
	opc |= (regexpr()<<6);
	putwd(opc, RABS);
	if ((opc & MASK)==MD)
		putwd(tmp.val, tmp.rel);
	return;
}

/* format 3: Td, D */
void opc_3(opc)
	int opc;
{
	opc |= full_reg();
	putwd(opc, RABS);
	if ((opc & MASK)==MD)
		putwd(res.val, res.rel);
	return;
}

/* format 4: R */
void opc_4(opc)
	int opc;
{
	regexpr();
	opc |= res.val;
	putwd(opc, RABS);
	return;
}

/* format 5: R, immediate */
void opc_5(opc)
	int opc;
{
	regexpr();
	opc |= res.val;
	skip_comma();	
	expr();
	putwd(opc, RABS);
	putwd(res.val, res.rel);
	return;
}

/* format 6: relational jumps */
void opc_6(opc)
	int opc;
{
	int offs, r;

	r = expr();
	if (pass && (r != currel) && (r != RUNDEF) )
		cerror(errv);
	if (r!=RUNDEF && !(r&REXT)) {
		offs = (res.val - (curseg->loc+2))>>1;
		if (offs < -128 || offs > 127)
			cerror(errv);
	}
	opc |= (offs & 0xff);
	putwd(opc, RABS);
	return;
}

/* format 7: immediate */
void opc_7(opc)
	int opc;
{
	expr();
	putwd(opc, RABS);
	putwd(res.val, res.rel);
	return;
}

/* format 7: count, R */
void opc_8(opc)
	int opc;
{
	regexpr();
	opc |= res.val;
	skip_comma();	
	regexpr();
	opc |= (res.val<<4);
	putwd(opc, RABS);
	return;
}

/* format 9: none */
void opc_9(opc)
	int opc;
{
	putwd(opc, RABS);
	return;
}

/* format 10: cru */
void opc_10(opc)
	int opc;
{
	int r;

	r = expr();
	if (pass && (r != RABS || res.val & ~0xff))
		cerror(errv);
	opc |= res.val;
	putwd(opc, RABS);
	return;
}

/* format 11: synthetic conditional far branches */
void opc_11(opc)
	int opc;
{
	int offs, r, idx;

	r = expr();
	if (pass && (r != currel) && (r != RUNDEF) )
		cerror(errv);
	idx = (opc & 0xf0)>>4;
	if (r!=RUNDEF && !(r&REXT)) {
		offs = (res.val - (curseg->loc+2))>>1;
		if (offs < -127 || offs > 126) {
			/* turn jmp to b */
			if( opc==0x1000 ) {
				putwd(0x460, RABS);
				putwd(res.val, res.rel);
			return;
			}
			/* reverse jump over branch */
			idx = branchtab[idx].reverse;
			opc = branchtab[idx].opc2;
			if( opc ) {
				putwd(opc|3, RABS);
			}
			opc = branchtab[idx].opc1;
			putwd(opc|2, RABS);
			/* and branch to far destination */
			putwd(0x460, RABS);
			putwd(res.val, res.rel);
			return;
		}
	}
	offs &= 0xff;
	opc = branchtab[idx].opc2;
	if( opc ) {
		putwd(opc|offs, RABS);
		offs--;
	}
	opc = branchtab[idx].opc1;
	putwd(opc|offs, RABS);
	return;
}

/* format 12: R, mf */
void opc_12(opc)
	int opc;
{
	regexpr();
	opc |= res.val;
	skip_comma();
	regexpr();
	if (pass && (res.val > 1))
		cerror(errf);
	opc |= (res.val<<4);
	putwd(opc, RABS);
	return;
}



/* branch table for opcode routines - indexed by class of opcode */
#ifdef __STDC__
static void (*oproutine[])(int) = {
#else
static void (*oproutine[])() = {
#endif
	&opc_1,
	&opc_2,
	&opc_3,
	&opc_4,
	&opc_5,
	&opc_6,
	&opc_7,
	&opc_8,
	&opc_9,
	&opc_10,
	&opc_11,
	&opc_12
};

void do_machine(op)
	int op;
{
	int opc, opctype;

	/* check even alignment */
	if (curseg->loc & 0x01)
		cerror(erra);

	/* call the handler */
	opc     = op & 0xfff0;
	opctype = op & 0x000f;
	(*oproutine[opctype])(opc);
}
