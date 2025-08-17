/*
 * Header for object code improver
 *
 * Several character buffers (used to store contents of registers,
 * constants, etc) needed to be increased in size to handle the
 * larger symbols passed thru from the compiler.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	MAXCPS	32

#ifndef	CHECK
#define	CHECK(x)
#endif

#define	BJMP	1
#define	CBR	2
#define	JMP	3
#define	LABEL	4
#define	DLABEL	5
#define	FLABEL	6
#define	EROU	7
#define SWB	8	/* pseudo op for labels in data tables */
#define A	9
#define ABS	10
#define AI	11
#define ANDI	12
#define B	13
#define BL	14
#define BLWP	15
#define C	16
#define CI	17
#define CKOF	18
#define CKON	19
#define CLR	20
#define COC	21
#define CZC	22
#define DEC	23
#define DECT	24
#define DIV	25
#define DIVS	26
#define IDLE	27
#define INC	28
#define INCT	29
#define INV	30
#define LDCR	31
#define LI	32
#define LIMI	33
#define LREX	34
#define LST	35
#define LWP	36
#define LWPI	37
#define MOV	38
#define MPY	39
#define MPYS	40
#define NEG	41
#define ORI	42
#define RSET	43
#define RTWP	44
#define S	45
#define SBO	46
#define SBZ	47
#define TB	48
#define SOC	49
#define SZC	50
#define SETO	51
#define SLA	52
#define SRA	53
#define SRC	54
#define SRL	55
#define STCR	56
#define STST 	57
#define STWP	58
#define SWPB	59
#define SYS	60
#define X	61
#define XOP	62
#define XOR	63
#define	TEXT	64
#define	DATA	65
#define	BSS	66
#define	EVEN	67
#define	END	68

#define	JEQ	0
#define	JNE	1
#define	JLS	2
#define	JGS	3
#define	JLT	4
#define	JGT	5
#define	JL	6
#define	JH	7
#define	JLE	8
#define	JHE	9
#define JNC	10
#define JOC	11

#define	BYTE	100
#define	LSIZE	512

struct node {
	char	op;
	char	subop;
	struct	node	*forw;
	struct	node	*back;
	struct	node	*ref;
	int	labno;
	char	*code;
	int	refc;
};

extern struct optab {
	char	*opstring;
	int	opcode;
} optab[];

char	line[LSIZE];
struct	node	first;
char	*curlp;
int	nbrbr;
int	nsaddr;
int	redunm;
int	iaftbr;
int	njp1;
int	nrlab;
int	nxjump;
int	ncmot;
int	nrevbr;
int	loopiv;
int	nredunj;
int	nskip;
int	ncomj;
int	nsob;
int	nrtst;
int	nlit;

int	nchange;
int	isn;
int	debug;
int	lastseg;
char	*lasta;
char	*lastr;
char	*alasta;
char	*alastr;
char	*firstr;
char	revbr[12];
char	regs[18][MAXCPS + 1];
char	conloc[MAXCPS + 1];
char	conval[MAXCPS + 1];
char	ccloc[MAXCPS + 1];

#define	NREG	16
#define	RT1	16
#define	RT2	17
#define	LABHS	127
#define	OPHS	157

struct optab *ophash[OPHS];
struct	node *nonlab();
char	*copy();
char	*sbrk();
char	*findcon();
struct	node *insertl();
struct	node *codemove();
char	*sbrk();
char	*alloc();
void	movedat();
void	clearreg();
void	rmove();
int	jumpsw();
void	addsob();
void	decref();
int	equop();
	

