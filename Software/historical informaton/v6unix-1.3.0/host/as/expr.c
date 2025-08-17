/*
 *		Assembler expression evaluation
 */

#include "as.h"

EXPR	trm;		/* accumulator for expression terms */
EXPR	res;

int uflag;

/*
 * Evaluate a single expression term from the input line, leaving its
 * value in trm
 */
void term()
{
	long val;
	int sign, t;

	sign = 1;
	for (;;) {		/* loop through leading + or - signs */
		switch (t=token()) {

		/*
		 * leading sign(s)
		 */
		case MINUS:
			sign = -sign;
		case PLUS:
		case SPACE:
			continue;

		case CON:
			trm.rel = RABS;
			val = conbuf;
			break;

		/*
		 * numeric name -- get its value & relocation bits 
		 */
		case NIDENT:
		{
			struct nlabel *p;

			if( !(p = getnlab(conbuf)) ) {
				trm.rel = RUNDEF;
				val = 0;
			}
			else {
				trm.rel = p->seg;
				val = p->loc;
			}
		}
		break;

		/*
		 * symbolic name -- get its value & relocation bits
		 */
		case IDENT:
		{
			int type;

			val = cursym->value;
			switch (type = (cursym->type & SSEG)) {

			case SUNDEF:
 				trm.rel = (cursym->type & SEXT) || uflag ?
					/* external reference */
					REXT | ((cursym->idx)<<4) :
					/* undefined symbol */
					RUNDEF;
				val = 0;
				break;

			default:
				/* translate symbol type to relocation bits */
				trm.rel = ((type & SSEG)-1) << 1;
			}
		}
		break;

		/*
		 * location counter
		 */
		case DOT:
			val = curseg->loc;
			trm.rel = currel;
			break;

		default:
			xerror(errx);
		}

		if (sign < 0) {
			if (trm.rel != RABS && trm.rel != RUNDEF)
				cerror(errr);
			val = -val;
		}

		trm.val = val;
		return;
    }
}

/*
 * Determine relocatability of result of operation op performed on
 *	operands with relocatability rel1 and rel2
 *
 * Permitted combinations are:
 *	undefined with anything		->	undefined
 *	absolute with absolute		->	absolute
 *	relocatable + absolute		->	relocatable
 *	relocatable - absolute		->	relocatable
 *	absolute + relocatable		->	relocatable
 *	relocatable - relocatable	->	absolute (provided both are in
 *					same segment or in same common block)
 *
 * Note that external references are permitted to take part in expressions
 */
int combine(rel1, rel2, op)
	int rel1, rel2, op;
{
	int ret;

	if (rel1 == RUNDEF || rel2 == RUNDEF)
		ret = RUNDEF;

	else if (pass == 0 && (rel1 & REXT || rel2 & REXT))
		/* forward references in ENTRY statements may change types */
		ret = RUNDEF;

	else if (rel2 == RABS)
		if (rel1 == RABS)
			ret = RABS;
		else if (op <= MINUS)
			ret = rel1;
		else
			cerror(errr);

	else if (rel1 == RABS)
		if (op == PLUS)
			ret = rel2;
		else
			cerror(errr);

	else if (rel1 == rel2 && op == MINUS)
		ret = RABS;

	else
		cerror(errr);

	return(ret);
}

/*
 * Evaluate an expression from the input line, leaving its value in exp
 *	- checks for undefined symbols
 *	- returns the relocatability of the expression
 */
int expr()
{
	int op, val, rel;

	/*
	 * first term
	 */
	term();
	rel = trm.rel;
	val = trm.val;

	/*
	 * check for further binary operators
	 */
	while ( (op = token())==PLUS || op==MINUS || op==STAR ||
			 op==AND || op==OR || op==SLASH)
	{
		term();
		/*
		 * find relocatability of result
		 */
		rel = combine(rel, trm.rel, op);

		switch (op) {

		case PLUS:	val += trm.val; break;
		case MINUS:	val -= trm.val; break;
		case STAR:	val *= trm.val; break;
		case AND:	val &= trm.val;	break;
		case OR:	val |= trm.val; break;
		case SLASH:	
			if (trm.val)
				val /= trm.val;
			else
				cerror(errz);
			break;
		}
	}
	nexttoken = op;
	res.val   = val;

	if (rel == RUNDEF && pass > 0)
		cerror(erru);

	return(res.rel = rel);
}

