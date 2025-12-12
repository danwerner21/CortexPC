
/* general */
#define TRUE    1
#define FALSE   0

/* input.c */
extern int In_string;
extern int In_pushed_back;
extern int Line_no;
extern int Pass;
extern int Err_count;

int  Get_ind(void);
void In_expand(int);
void Error(char *);
void Asm_error(char *, int);

/* symbol.c */
struct Symbol {
    struct Symbol *link;
    int  type;
    int  val;
    int  seg;
    char name[1];
};
extern struct Symbol *Sym;

void Get_ident(void);
struct Symbol* Sym_lookup(int insert);
void Sym_clean( void );
void Sym_dump(void);

#define IDENTIFIER      0
#define UNDEFINED      -1
#define LABEL           1
#define PSEUDO          2
#define OPCODE          3
#define EXTERNAL        4
#define GLOBAL          5
#define MANIFEST        255
#define RB_ABS          0

/* parse.c */
struct Module {
    struct Module *next;
    struct Symbol *entry;
    struct Symbol *local;
    int     seg_used;
    char    no_global;
    char    active;
    char    min;
    char    max;
    char    function;
    char    nargs;
};
extern struct Module *M;

void Init_pseudo( void );
void Parse( void );
void Eat_line( void );

/* ti990.c */
void Init_opcodes( void );
void Gen_opcode( void );
void Gen_DCn( int n );

/* expr.c */
struct Value {
    int val;
    int seg;
};
extern struct Value Result;
extern struct Value Oper;
void Expr( void );
int Abs_expr( void );

/* lib.c */
int  Length(char *s);
int  Equal(char *s1, char *s2);
void Copy(char *src, char *dst);

/* token.c */
#define D 0x10
#define L 0x20

extern char Ctab[];
extern int token;
extern char String[];
void Lex(void);
