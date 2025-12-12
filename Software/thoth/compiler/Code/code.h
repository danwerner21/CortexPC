#include <stdint.h>

struct Symbol {
    struct Symbol *link;
    int ref;
    int value;
    int addrmod;
    int type;
    int str_id;
    intptr_t const_value;
    struct Symbol *clink;
    struct Defer *dlist;
};

struct Expr {
    struct Expr *left;
    int   op;
    struct Expr *right;
    struct Symbol *resultin;
    struct Symbol *scratch;
};

struct Node {
    void *link;
    int   ref;
};

#define TRUE  1
#define FALSE 0

#define SYMBOL_DEFINED  0x8000
#define TYPE_FIELD      0x7800
#define REL_WSD_FIELD   0x07ff
#define WSD_FIELD       0x00ff

#define CONST_TYPE      0x0800
#define STRING_TYPE     0x1000
#define AUTO_TYPE       0x1800
#define EXTRN_TYPE      0x2000
#define LABEL_TYPE      0x2a02

#define RD_DATA         0x0100

#define MACHADDR_FACTOR 2

#define AUTO_ADDRMOD        0x2b
#define ARG_ADDRMOD         0x2f
#define EXTRN_ADDRMOD       0x20
#define STRING_ADDRMOD      0x20
#define CONST_ADDRMOD       0x20
#define INDEXED_MODE        0x20

#define ADDR_RELDESC        1
#define PTR_RELDESC         3
#define STRING_RELDESC      4
#define AUTO_RELDESC        5

#define ABSOLUTE_RB         0
#define DATA_RB             1
#define CODE_RB             2
#define STRING_RB           3
#define EXTRN_RB            1

extern int *Counter;
extern int Prolog_done;

/* defer.c */
struct Defer *Defer_gen( int instruction, struct Symbol *symbol, int length, int displacement );
void Backplug_gen( struct Symbol *symbol );
void Check_addressability( int skip, int range );
void Backplug_symbol( struct Symbol *symbol );
void Branch_gen( struct Symbol *label );
void Jump_gen( struct Symbol *label );
void Hop_gen( int op, struct Symbol *label );

/* main.c */
void Error(char *msg);
int  Get(void);
int  GetW(void);
int  GetR(void);
void Process_string( int str_id, struct Symbol *p, int type );
void Get_name(int id, char *name);
struct Symbol *Next_temp( void );

/* symbol.c */
struct Symbol *Int_label( void );
struct Symbol *Sym_find( int ref, int insert );
//void   Def_const( int ref, int cval );
struct Symbol *Constant( int cval );
struct Symbol *Sym_label( int ref );
struct Symbol *Sym_alloc( void );
void Sym_clean(void);
extern struct Symbol Sym_pseudo[];

/* code.c */
void Code( int op, struct Symbol *left_oper, struct Symbol *right_oper );

/* expr.c */
struct Expr *Expr_alloc( void );
struct Expr *Build_tree( void );
struct Expr *Allocate_registers(struct Expr *tree, struct Symbol *force);
void Expr_print( struct Expr *tree );

/* twit.c */
void Twit_gen( struct Expr *expr );

extern struct Op_dope {
    int reverse;
    int type;
} Dope[];

#define B    1
#define U    2
#define N    4
#define C    8
#define E   16
#define FA  32
#define RA  64

#define BINARY(op)  (Dope[op>>1].type & B)
#define UNARY(op)   (Dope[op>>1].type & U)
#define NONARY(op)  (Dope[op>>1].type & N)
#define COMMUTE(op) (Dope[op>>1].type & C)
#define EFFECT(op)  (Dope[op>>1].type & E)
#define IS_FA(op)   (Dope[op>>1].type & FA)
#define IS_RA(op)   (Dope[op>>1].type & RA)
#define REVERSE(op) (Dope[op>>1].reverse)
#define HASEFF      32767

/* arith.c */
struct Symbol *Reg(struct Symbol *sym);
void Logical_expr_gen( struct Expr *tree, struct Symbol *label, int jmp_true );
void Arith_expr_gen( struct Expr *tree );

extern struct Symbol *R[], *Const_one;

/* modify.c */
struct Expr *Modify(struct Expr *tree);

/* select.c */
void Start_select(void);
void Do_case( int min, int max );
void Def_case(void);
void Str_select(void);
void Wdselect_gen(void);
