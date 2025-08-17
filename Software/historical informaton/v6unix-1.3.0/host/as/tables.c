#include "as.h"

struct optab {
	char   name[8];	
	int    opval;		
};

/*
 * Opcodes and pseudo-ops
 */
struct optab opcodes[] = {

	/* data-definition pseudo-ops */
	{ ".byte",	PBYTE  },
	{ ".even",	PEVEN  },
	{ ".globl",	PGLOBL },
	{ ".if",	PIF    },
	{ ".endif",	PENDIF },
	{ ".text",	PTEXT  },
	{ ".data",	PDATA  },
	{ ".bss",	PBSS   },
	{ ".comm",	PCOMM  },

	/* TI-990, TMS9995 machine ops */
	{ "a",      0xa000 | type_1 },
	{ "ab",     0xb000 | type_1 },
	{ "abs",    0x0740 | type_3 },
	{ "ai",     0x0220 | type_5 },
	{ "andi",   0x0240 | type_5 },
	{ "bjmp",   0x1000 | type_11 },
	{ "bjeq",   0x1010 | type_11 },
	{ "bjne",   0x1020 | type_11 },
	{ "bjgt",   0x1030 | type_11 },
	{ "bjgs",   0x1040 | type_11 },
	{ "bjlt",   0x1050 | type_11 },
	{ "bjls",   0x1060 | type_11 },
	{ "bjh",    0x1070 | type_11 },
	{ "bjhe",   0x1080 | type_11 },
	{ "bjl",    0x1090 | type_11 },
	{ "bjle",   0x10a0 | type_11 },
	{ "b",      0x0440 | type_3 },
	{ "bl",     0x0680 | type_3 },
	{ "blwp",   0x0400 | type_3 },
	{ "c",      0x8000 | type_1 },
	{ "cb",     0x9000 | type_1 },
	{ "ci",     0x0280 | type_5 },
	{ "ckof",   0x03c0 | type_9 },
	{ "ckon",   0x03a0 | type_9 },
	{ "clr",    0x04c0 | type_3 },
	{ "coc",    0x2000 | type_2 },
	{ "czc",    0x2400 | type_2 },
	{ "dec",    0x0600 | type_3 },
	{ "dect",   0x0640 | type_3 },
	{ "div",    0x3c00 | type_2 },
	{ "divs",   0x0180 | type_3 }, /* TMS9995, TI-990/10A, /12 */
	{ "idle",   0x0340 | type_9 },
	{ "inc",    0x0580 | type_3 },
	{ "inct",   0x05c0 | type_3 },
	{ "inv",    0x0540 | type_3 },
	{ "jeq",    0x1300 | type_6 },
	{ "jgt",    0x1500 | type_6 },
	{ "jh",     0x1b00 | type_6 },
	{ "jhe",    0x1400 | type_6 },
	{ "jl",     0x1a00 | type_6 },
	{ "jle",    0x1200 | type_6 },
	{ "jlt",    0x1100 | type_6 },
	{ "jmp",    0x1000 | type_6 },
	{ "jnc",    0x1700 | type_6 },
	{ "jne",    0x1600 | type_6 },
	{ "jno",    0x1900 | type_6 },
	{ "joc",    0x1800 | type_6 },
	{ "jop",    0x1c00 | type_6 },
	{ "ldcr",   0x3000 | type_2 },
	{ "ldd",    0x07c0 | type_3 },
	{ "lds",    0x0780 | type_3 },
	{ "li",     0x0200 | type_5 },
	{ "lim",    0x0070 | type_4 }, /* TI990/12 */
	{ "limi",   0x0300 | type_7 },
	{ "lmf",    0x0320 | type_12 }, /* TI990/10, /10A, /12 */
	{ "lrex",   0x03e0 | type_9 },
	{ "lst",    0x0080 | type_4 }, /* TMS9995, TI-990/10A, /12 */
	{ "lwp",    0x0090 | type_4 }, /* TMS9995, TI-990/10A, /12 */
	{ "lwpi",   0x02e0 | type_7 },
	{ "mov",    0xc000 | type_1 },
	{ "movb",   0xd000 | type_1 },
	{ "mpy",    0x3800 | type_2 },
	{ "mpys",   0x01e0 | type_3 },
	{ "neg",    0x0500 | type_3 },
	{ "nop",    0x1000 | type_9 },
	{ "ori",    0x0260 | type_5 },
	{ "rset",   0x0360 | type_9 },
	{ "rtwp",   0x0380 | type_9 },
	{ "s",      0x6000 | type_1 },
	{ "sb",     0x7000 | type_1 },
	{ "sbo",    0x1d00 | type_10 },
	{ "sbz",    0x1e00 | type_10 },
	{ "seto",   0x0700 | type_3 },
	{ "sla",    0x0a00 | type_8 },
	{ "soc",    0xe000 | type_1 },
	{ "socb",   0xf000 | type_1 },
	{ "sra",    0x0800 | type_8 },
	{ "src",    0x0b00 | type_8 },
	{ "srl",    0x0900 | type_8 },
	{ "stcr",   0x3400 | type_2 },
	{ "stst",   0x02c0 | type_4 },
	{ "stwp",   0x02a0 | type_4 },
	{ "swpb",   0x06c0 | type_3 },
	{ "sys",    0x2c60 | type_2 },
	{ "szc",    0x4000 | type_1 },
	{ "szcb",   0x5000 | type_1 },
	{ "tb",     0x1f00 | type_10 },
	{ "x",      0x0480 | type_3 },
	{ "xop",    0x2c00 | type_2 },
	{ "xor",    0x2800 | type_2 },
	{ {0}, 0 }
};

struct branch branchtab[] = {
	/*  0: bjmp */ { 0x1000, 0x0000,  0 },
	/*  1: bjeq */ { 0x1300, 0x0000,  2 },
	/*  2: bjne */ { 0x1600, 0x0000,  1 },
	/*  3: bjgt */ { 0x1500, 0x0000,  6 },
	/*  4: bjgs */ { 0x1300, 0x1500,  5 },
	/*  5: bjlt */ { 0x1100, 0x0000,  4 },
	/*  6: bjls */ { 0x1300, 0x1100,  3 },
	/*  7: bjh  */ { 0x1b00, 0x0000, 10 }, 
	/*  8: bjhe */ { 0x1400, 0x0000,  9 },
	/*  9: bjl  */ { 0x1a00, 0x0000,  8 },
	/* 10: bjle */ { 0x1200, 0x0000,  7 }
};
  
struct optab predefs[] = {
	/* TMS9900 register names */
	{ "r0", 	0x0000 },
	{ "r1", 	0x0001 },
	{ "r2", 	0x0002 },
	{ "r3", 	0x0003 },
	{ "r4", 	0x0004 },
	{ "r5", 	0x0005 },
	{ "r6", 	0x0006 },
	{ "r7", 	0x0007 },
	{ "r8", 	0x0008 },
	{ "r9", 	0x0009 },
	{ "r10",	0x000a },
	{ "r11",	0x000b },
	{ "r12",	0x000c },
	{ "r13",	0x000d },
	{ "r14",	0x000e },
	{ "r15",	0x000f },
	{ "sp",		0x000a },
	{ "bp",		0x0009 },
	{ {0}, 0 }
};

void sym_init()
{
	struct optab  *op;
	SYMBOL *sym;

	for (op = opcodes; op->name[0]; op++) {
		sym = sym_enter(op->name);
		sym->type  = OPCODE;
		sym->value = op->opval;
	}
	for (op = predefs; op->name[0]; op++) {
		sym = sym_enter(op->name);
		sym->type  = SABS;
		sym->value = op->opval;
	}
	linksym  = 0;
	symcount = 0;
}
