#include <stdio.h>
#include "as.h"

SEGMNT text, data, bss, comm;

SYMBOL *curlab;		/* current label */
SEGMNT *curseg;		/* current segment */
SYMBOL *curcomm;	/* current COMN or STRUC name */
int	currel;		/* current relocatability */
int	ifcount;	/* count of open .if's */

#ifndef __ti990__
#include <setjmp.h>
extern jmp_buf err_jmp;
#endif
extern void sym_reset();

void do_pseudo(op)
	int op;
{
	int t, count;

	switch (op) {

	case PIF:
		t = expr();
		if (t != RABS) cerror(errv);
		if (res.val!=0) {
			ifcount++;
			break;
		}
		putline();
		count = 1;
		while (count > 0) {
			t = tok(0);
			if (t == EOL && !get_line() ) {
				cerror(erri);
				count = 0;
			}
			if (t != IDENT || symlook(0) == 0) continue;
			switch (cursym->value) {
			case PIF:	count++; break;
			case PENDIF:	count--; break;
			}
		}
		break;

	case PENDIF:
		if (--ifcount < 0) cerror(erri);
		break;

	case PDATA:
		curseg = &data;
		currel = RDATA;
		align(2);
		nexttoken = EOL;
		break;

	case PTEXT:
		curseg = &text;
		currel = RTEXT;
		align(2);
		nexttoken = EOL;
		break;

	case PBSS:
		curseg = &bss;
		currel = RBSS;
		align(2);
		nexttoken = EOL;
		break;

	case PEVEN:
		align(2);
		break;

	case PBYTE:
		do {
			t = expr();
			if (pass_gen)
				if (t != RABS
					|| res.val < -127 || res.val > 255)
					cerror(errv);
			putb(res.val);
			if (pass_lst)
				lstb(res.val, RABS);

		} while ((t = token()) == COMMA);
		nexttoken = t;
		break;

	case PCOMM:
		if (tok(1)!=IDENT)
			cerror(errx);
		curlab = cursym;
		cursym->type |= SEXT;

		if (tok(1)!=COMMA)
			cerror(errx);
		expr();
		cursym->value = res.val;
		break;

	case PGLOBL:
		do {
			if ((t = tok(1))!=IDENT)
				break;
			else
				cursym->type |= SEXT;
		} while ((t = token()) == COMMA);
		nexttoken = t;
		break;

	}
}

static void do_expr_or_assign()
{
	int t,t2;

	/* first check for assignment */
	sscan();
	t  = token();
	t2 = token();

	/* if not, backtack and assemble an expression */
	if (t2 != EQU) {
		rscan();
		expr();
		putwd(res.val, res.rel);
		return;
	}

	switch (t) {

	case DOT:
		switch (t = expr()) {

			case RTEXT:
				curseg = &text;
				break;

			case RDATA:
				curseg = &data;
				break;

			default:
				if (t == currel)
					break;
				cerror(errr);
				t = RDATA;
		}
		currel = t;
		org(res.val);
		break;

	case IDENT:
		curlab = cursym;
		expr();
		deflab(res.rel, res.val);
		break;

	default:
		xerror(errx);
	}
}

int assemble()
{
	register int t, op;
	register int optype;

	/*
	 * Initialize for assembly pass
	 */
	nextfile();
	seginit();
	nlabinit();

#ifndef __ti990__
	setjmp(err_jmp);
#else
	setexit();
#endif
	for (; get_line();putline()) {

		/* list line address */
		if (pass_lst)
			lstloc();

next_stmt:
		/* skip comments & EOL marker */
		if ((t = tok(0)) == COMMENT || t == EOL)
			continue;

		/* save statement label symbol to be defined below */
		curlab  = 0;

		while (t == LABEL || t == NLABEL) {
			if (t == LABEL) {
				symlook(2);
				curlab = cursym;
				deflab(currel, curseg->loc);
			}
			else {
				defnlab(conbuf, currel, curseg->loc);
			}
			t = tok(0);
		}

		switch (t) {

		case EOL:
			continue;

		case SEMI:
			goto next_stmt;

		case STRING:
			putstr(strsiz);
			goto next_stmt;

		case IDENT:
			if (symlook(0)) {
				optype = RABS;
				op     = cursym->value;
				break;
			};
			symlook(2);

		default:
			nexttoken = t;
			do_expr_or_assign();
			goto next_stmt;
		}

		/* call opcode routine to parse the operands */
		if ( (op & 0xfff0)==0) {
			do_pseudo(op);
		} else {
			do_machine(op);
		}
		goto next_stmt;

	}
	curseg = &text;
	align(2);
	curseg = &data;
	align(2);
	sym_reset();
	return(errcnt);
}
