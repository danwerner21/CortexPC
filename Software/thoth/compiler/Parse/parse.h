
#define TRUE  1;
#define FALSE 0;

/* Expression nodes and trees */

int  Expr_leaf(int ref, int type, int kind);
int  Expr_node(int ref, int type, int left, int right);
void Expr_check(int node);
void Expr_put(int top);
int  Expr_const(int node);
void Expr_extrn(int node);

/* Symbol table for reference management
 * Values must not overlap with valid expression stack indices
 */

#define TNAME  1001
#define TCONST 1002
#define TSTR   1003
#define TLABEL 1004
#define TEXTRN 1005
#define TAUTO  1006

#define MUST 1      /* id must already have a ref */
#define MAY  2      /* id may already have a ref, otherwise create new ref */
#define NEW  3      /* id must not have a ref yet, create a new ref */

void Ref_clear(void);
int  Ref_const(int val);
int  Ref_string(int str_id);
int  Ref_ident(int name_id, int type, int seek);
int  Ref_label(void);
void Ref_print(void);

/* Parsil output */
void Put (int byte);
void PutR(int ref);
void PutW(int word);

void DPut (int byte);
void DPutW(int word);
void DPutC(int val);

/* yacc */
extern int yylval;
int yyparse(void);
int yylex(void);
void yyerror(char*);