#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "as.h"

#define HASHSIZE 199		/* size of hashed index */
#define hash(name)	((((unsigned *)name)[0]+((unsigned *)name)[1])%HASHSIZE)

SYMBOL	*hashtab[HASHSIZE];	/* index to symbol table */
SYMBOL	*cursym;
char	symbuf[8];
int	symcount;

void sym_reset();

/* enter a symbol into the table */

SYMBOL *sym_enter(name)
	char* name;
{
	register int i;

	for (i=0; i<8; i++)
		if ((symbuf[i] = *name))
			name++;
	symlook(2);
	return cursym;
}

/* maintain a linked list in allocation order, for use with
   a.out format symbol table indexing */
SYMBOL	*linksym;
SYMBOL	*firstsym;

/*
 * Look up symbol name in symbuf in hash-indexed symbol table
 *	flag =	0; opcode expected
 *		1: user symbol expected
 *		2: user symbol expected; enter into index if not found
 *
 *	returns	0: symbol was not previously defined
 *		1: symbol was previously defined
 *	cursym: points to symbol table entry
 */
int symlook(flag)
	int flag;
{
	SYMBOL **lpp, *sp;

	lpp = &hashtab[hash(&symbuf)];

	/* First see if the symbol has already been defined */
	for (sp = *lpp; sp; lpp = &(sp->next), sp = sp->next) {
		if (!strncmp(sp->name, symbuf, 8)) {
			if (flag) {
				if (sp->type==OPCODE) continue;
			} else {
				if (sp->type!=OPCODE) continue;
			}
			if (sp->name[0]=='~') sp->name[0] = '1'; 
			cursym = sp;
			return(1);
		}
	}

	/* Enter new symbol into symbol table and index */
	if (flag > 1) {
		if (symbuf[0]=='~') symbuf[0] = '1';
		cursym = (struct symbol*) malloc(sizeof(SYMBOL));
		strncpy(cursym->name, symbuf, 8);
		cursym->next  = NULL;
		cursym->type  = 0;
		cursym->value = 0;

		/* link in index order */
		cursym->idx   = symcount++;
		cursym->link  = NULL;
		if (linksym)
			linksym->link = cursym;
		else
			firstsym = cursym;
		linksym = cursym;

		/* link in hash order */
		*lpp = cursym;
	}
	return(0);
}

void sym_reset()
{
	SYMBOL *sp;

	for (sp = firstsym; sp; sp = sp->link) {
		if (sp->name[0]=='1') sp->name[0]='~';
	}
}

void sym_iter()
{
	SYMBOL *sp;

	for (sp = firstsym; sp; sp = sp->link) {
		printf("%8s: %5x, %5x\n", sp->name, sp->type, sp->value);
	}
}

/*
 * Handle the definition and reference of numeric labels, e.g. 1f
 */
int nlabmax = 0;
int nlabidx = -1;

#define NLABMAX	512

struct nlabel nlabtab[512];

void defnlab(num, seg, loc)
	long num;
	int loc;
	char seg;
{
	struct nlabel *p;

	if (nlabidx++==NLABMAX)
		cerror(errt);
	if (nlabmax<=nlabidx)
		nlabmax = nlabidx + 1;
	p = &nlabtab[nlabidx];
	p->seg = seg;
	p->num = (num<0) ? -num : num;
	p->loc = loc;
}

struct nlabel *getnlab(num)
	long num;
{
	struct nlabel *p;
	int i;

	i = nlabidx;
	if (num<0) {
		num = -num;
		for(; i>=0; i--) {
			p = &nlabtab[i];
			if (p->num==num && p->seg==currel)
				break;
		}		
	}
	else {
		for(i++; i<nlabmax; i++) {
			p = &nlabtab[i];
			if (p->num==num && p->seg==currel)
				break;
		}
	}
	return((i<0||i>=nlabmax) ? NULL : p);
}

void nlabinit()
{
	nlabidx = -1;
}
