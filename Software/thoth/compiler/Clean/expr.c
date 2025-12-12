
#include <stdio.h>

#include "../Parse/il_opc.h"

/* note: we now have 70 IL operators. Bonkowski says there are 72 total */

struct Op_dope {
    int reverse;
    int type;
};

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

struct Op_dope Dope[] =
{
    /* IL_ASSIGN     */ { IL_RASGN,   B|E|FA   },
    /* IL_ASDIV      */ { IL_RASDIV,  B|E|FA   },
    /* IL_ASMUL      */ { IL_RASMUL,  B|E|FA   },
    /* IL_ASMOD      */ { IL_RASMOD,  B|E|FA   },
    /* IL_ASADD      */ { IL_RASADD,  B|E|FA   },
    /* IL_ASSUB      */ { IL_RASSUB,  B|E|FA   },
    /* IL_ASSHL      */ { IL_RASSHL,  B|E|FA   },
    /* IL_ASSHR      */ { IL_RASSHR,  B|E|FA   },
    /* IL_ASAND      */ { IL_RASAND,  B|E|FA   },
    /* IL_ASOR       */ { IL_RASOR,   B|E|FA   },
    /* IL_ASXOR      */ { IL_RASXOR,  B|E|FA   },
    /* IL_EQU        */ { IL_EQU,     B        },
    /* IL_NEQU       */ { IL_NEQU,    B        },
    /* IL_GT         */ { IL_LTE,     B        },
    /* IL_GTE        */ { IL_LT,      B        },
    /* IL_LTE        */ { IL_GT,      B        },
    /* IL_LT         */ { IL_GTE,     B        },
    /* IL_SHR        */ { IL_RSHR,    B        },
    /* IL_SHL        */ { IL_RSHL,    B        },
    /* IL_BITOR      */ { IL_BITOR,   B|C      },
    /* IL_BITAND     */ { IL_BITAND,  B|C      },
    /* IL_BITXOR     */ { IL_BITXOR,  B|C      },
    /* IL_DIV        */ { IL_RDIV,    B        },
    /* IL_MOD        */ { IL_RMOD,    B        },
    /* IL_PREINC     */ { 0,          U|E|RA   },
    /* IL_PREDEC     */ { 0,          U|E|RA   },
    /* IL_POSTINC    */ { 0,          U|E|RA   },
    /* IL_POSTDEC    */ { 0,          U|E|RA   },
    /* IL_BITNOT     */ { 0,          U        },
    /* IL_NOT        */ { 0,          U        },
    /* IL_WIDX       */ { IL_WIDX,    B        },
    /* IL_BIDX       */ { IL_RBIDX,   B        },
    /* IL_COMMA      */ { 0,          B        },
    /* IL_QUERY      */ { 0,          B        },
    /* IL_COLON      */ { 0,          B        },
    /* IL_LOGOR      */ { 0,          B        },
    /* IL_LOGAND     */ { 0,          B        },
    /* IL_ADD        */ { IL_ADD,     B|C      },
    /* IL_SUB        */ { IL_RSUB,    B        },
    /* IL_MUL        */ { IL_MUL,     B|C      },
    /* IL_LVAL       */ { 0,          U        },
    /* IL_RVAL       */ { 0,          U        },
    /* IL_ASBYTE     */ { IL_RASBYTE, B|E|FA   },
    /* IL_BFETCH     */ { 0,          U        },
    /* IL_WFETCH     */ { 0,          U        },
    /* IL_NFCTN_CALL */ { 0,          B|E      },
    /* IL_FCTN_CALL  */ { 0,          B|E      },
    /* IL_MARK       */ { 0,          N        },
    /* IL_RQD        */ { 0,          U        },
    /* IL_RASGN      */ { IL_ASSIGN,  B|E|RA   },
    /* IL_RASDIV     */ { IL_ASDIV,   B|E|RA   },
    /* IL_RASMUL     */ { IL_ASMUL,   B|E|RA   },
    /* IL_RASMOD     */ { IL_ASMOD,   B|E|RA   },
    /* IL_RASADD     */ { IL_ASADD,   B|E|RA   },
    /* IL_RASSUB     */ { IL_ASSUB,   B|E|RA   },
    /* IL_RASSHL     */ { IL_ASSHL,   B|E|RA   },
    /* IL_RASSHR     */ { IL_ASSHR,   B|E|RA   },
    /* IL_RASAND     */ { IL_ASAND,   B|E|RA   },
    /* IL_RASOR,     */ { IL_ASOR,    B|E|RA   },
    /* IL_RASXOR     */ { IL_ASXOR,   B|E|RA   },
    /* IL_RSHR       */ { IL_SHR,     B        },
    /* IL_RSHL       */ { IL_SHL,     B        },
    /* IL_RSUB       */ { IL_SUB,     B        },
    /* IL_RASBYTE    */ { IL_ASBYTE,  B|E|RA   },
    /* IL_RDIV       */ { IL_DIV,     B        },
    /* IL_RMOD       */ { IL_MOD,     B        },
    /* IL_RBIDX      */ { IL_BIDX,    B        },
    /* IL_NULL       */ { 0,          B        },
    /* IL_BITANDN    */ { 0,          B        },
    /* IL_ADDR       */ { 0,          B        },
    /* IL_NEG        */ { 0,          U        }
};

struct Expr {
    int op;
    int left;       /* for constants: value */
    int right;
    int degree;     /* sethi-ullman label, or -1 for constants */
};

#define ISCONST(node) (Tree[node].degree == -1)
#define MAXEXPR 200

struct Expr Tree[MAXEXPR];
int alloc;
int operands[32];

int Check_const(int ref, int *d);

int
Expr_read( char *refptr )
{
    int ref, i, d, node;
    struct Expr *expr;

    node = 0;
    i = 0;
    operands[0] = 0;

    while( (ref = *refptr++ & 0xff ) != 0 )
    {
        if( ref & 0x80 ) {
            ref = (ref & 0x7f) | (*refptr++ << 7);
        }
        ++node;
        expr = &Tree[node];
        if( ref & 1 ) {
            /* interior node */
            expr->op = ref;
            expr->right = expr->left = expr->degree = 0;
            if( BINARY(ref) ) {
                expr->right = operands[--i];
                expr->left  = operands[--i];
            }
            else if( UNARY(ref) ) {
                expr->left  = operands[--i];         
            }
        }
        else {
            /* leaf node */
            expr->op     = ref;
            expr->left   = Check_const(ref, &d);
            expr->right  = 0;
            expr->degree = d;
        }
        operands[i++] = node;
    }
    alloc = node;
    return( node );
}

static char *ptr;

static void
Expr_putnode(int node)
{
    struct Expr *expr;
    int ref;

    if( node == 0 ) return;
    expr = &Tree[node];
    ref = expr->op;
    if( ref & 1 ) {
        Expr_putnode( expr->left );
        Expr_putnode( expr->right );
    }
    //printf("put %d\n", exp->ref);
    if( ref < 128 )
        *ptr++ = ref;
    else
    {
        *ptr++ = (ref & 0x7f) | 0x80;
        *ptr++ = (ref >> 7);
    }
}

int
Expr_write(int expr, char* start)
{
    ptr = start;
    Expr_putnode( expr );
    *ptr++ = 0;
    return( ptr - start );
}

/* ================ */

/* The below functions implements the various parts of
 * Stafford section 5.4, 'Expression Modification'.
 */

int
max(int a, int b)
{
    return (a > b) ? a : b;
}

int Expr_walk(int node);

/* Stafford does not clearly say whether the Eh compiler implements
 * Seti-Ullman "algorithm 3" for commutative operators. Just in case,
 * the below implements this, using the approach of the DMR C compiler
 * so that it is at least rooted in contemporary precedent.
 */
 
struct acl {
    int nextn; /* next "node" slot */
    int nextl; /* next "leaf" slot */
    struct Expr *nlist[20]; /* list of "nodes"   */
    struct Expr *llist[21]; /* list of "leafs" */
};

static void
Insert(int op, int node, struct acl *list)
{
    int d;
    int d1, i;
    struct Expr *t, *expr;

    /* if commutable node: */
    expr = &Tree[node];
    if (expr->op == op) {
ins:	list->nlist[list->nextn++] = expr;
        Insert(op, expr->left,  list);
        Insert(op, expr->right, list);
        return;
    }
    node = Expr_walk( node );
    expr = &Tree[node];
    if (expr->op == op)
        goto ins;

    /* if "leaf", sorted insert: */
    d = expr->degree;
    for (i=0; i<list->nextl; i++) {
        if( (d1 = list->llist[i]->degree) < d )
        {
            t = list->llist[i];
            list->llist[i] = expr;
            expr = t;
            d = d1;
        }
    }
    list->llist[list->nextl++] = expr;
}

/* commutable: + * | & ^ */

static int
Do_commutable( int op, int node )
{
    struct acl acl;
    int d, i;
    struct Expr *t1, **t2, *t, *expr;

    /* build acl list */
    acl.nextl = 0;
    acl.nextn = 0;
    Insert(op, node, &acl);
    acl.nextl--;

    /* rewrite nodes with sorted leafs */
    expr = *(t2 = &acl.llist[0]);
    d = max(expr->degree, 1);
    for (i=0; i<acl.nextl; i++) {
        t1 = acl.nlist[i];
        t1->right = (t = *++t2) - Tree;
        t1->degree = d = (t->degree >= d) ? d+1 : d;
        t1->left = expr - Tree;
        expr = t1;
    }
    return( expr - Tree );
}

/* This routine implements Stafford section 5.4.4,
 * 'Modify operators'.
 */
static int
Unary_optim(int node)
{
    struct Expr *expr, *left;
    int op;
    
    expr = &Tree[node];
    op = expr->op;
    left = &Tree[expr->left];
    expr->degree = EFFECT(op) ? HASEFF : left->degree;
    
    switch( op ) {
    case IL_LVAL:
        if( left->op == IL_RVAL )
            node = left->left;
        else if( left->op == IL_ADD ) {
            left->op = IL_WIDX;
            node = expr->left;
        }
        break;
            
    case IL_RVAL:
        if( left->op == IL_LVAL )
            node = left->left;
        else if( left->op == IL_WIDX ) {
            left->op = IL_ADD;
            node = expr->left;
        }
        break;
    }

    expr = &Tree[node];
    expr->left  = Expr_walk( expr->left  );    

    return node;    
}

/* This routine implements Stafford section 5.4.5: removing
 * 'useless' operands.
 */
int
Binary_optim(int node)
{
    struct Expr *expr, *right, *left;

    expr  = &Tree[node];
    right = &Tree[expr->right];
    left  = &Tree[expr->left];
    
    /* At this point a constant is always on the right.
     * If right-hand size is zero, try to simplify.
     */
    if( right->degree < 0 && right->left == 0 ) {
        switch( expr->op ) {
        
        /* case 1 */
        case IL_MOD:    case IL_ADD:    case IL_SUB:    case IL_SHR:
        case IL_SHL:    case IL_BITOR:  case IL_BITXOR:
        case IL_RASMOD: case IL_RASADD: case IL_RASSUB: case IL_RASSHR:
        case IL_RASSHL: case IL_RASOR:  case IL_RASXOR:
            return( expr->left );
            
        /* case 2 */
        case IL_MUL:    case IL_RMOD:   case IL_RDIV:   case IL_BITAND:
        case IL_LOGAND:
            if( left->degree != HASEFF )
                return( expr->right );

        /* case 3 */
        case IL_DIV:
            fprintf(stderr, "warning: division by zero\n");
            break;
        
        /* case 4 */
        case IL_WIDX:
            expr->op = IL_LVAL;
            expr->right = 0;
            break;
        
        /* case 5 */
        case IL_RSUB:
            expr->op = IL_NEG;
            expr->right = 0;
            break;
        
        /* case 6 */
        case IL_RASMUL:
            expr->op = IL_RASGN;
            break;
        }
    }
    return( node );
}

/* This routine walks the expression tree in postorder (left,
 * right, node). As part of its walk, it performs various aptimisations
 * to the expression tree as discussed in Stafford section 5.4.
 */
int
Expr_walk(int node)
{
    struct Expr *expr, *e;
    int op, d1, d2, rev, t;
    
    if( node == 0 )
        return 0;
    
    expr = &Tree[node];
    op = expr->op;
    if( !(op & 1) || NONARY(op) )
        return node;
    
    /* Do the mods of section 5.4.4 */
    if( UNARY(op) )
        return( Unary_optim( node ) );
    
    /* Node is now known to be binary. First check for
     * commutable associative operators and apply Sethi-Ulmann
     * 'algorithm 3'.
     */
    if( COMMUTE(op) ) {
        node = Do_commutable( op, node );
        return( Binary_optim( node ) );
    }
    
    expr->left  = Expr_walk( expr->left  );
    expr->right = Expr_walk( expr->right );
    
    /* Section 5.4.3: remove fetch operators again in lval context */
    if( op == IL_ASSIGN || op == IL_ASBYTE ) {
        t = Tree[expr->right].op;
        if( t == IL_WFETCH || t == IL_BFETCH )
            expr->right = Tree[expr->right].left;
    }

    /* Section 5.4.2: if the op is swappable, put most complex operand
     * on the left. Constants are always on the right. The parser has already
     * reduced operators with all constant operands to a single constant.
     */
    d1 = Tree[expr->left].degree;
    d2 = Tree[expr->right].degree;
    rev = REVERSE(op);
    if( ((d1 < 0) || (d1 != HASEFF && d2 != HASEFF && d2 > d1)) && rev != 0 )
    {
        t = expr->left;
        expr->left = expr->right;
        expr->right = t;
        expr->op = rev;
    }

    /* Section 5.4.1: apply the main Sethi-Ulmann algorithm and set
     * the degree (= 'label').
     */
    d1 = max(d1, 1); d2 = max(d2, 0);
    if( op == IL_QUERY || op == IL_COLON )
        expr->degree = max(d1, d2);
    else if( EFFECT(op) )
        expr->degree = HASEFF;
    else
        expr->degree = (d1 == d2) ? ++d1: max(d1, d2);
        
    /* Section 5.4.5: remove 'useless' expression clauses and
     * reset local vars (as 'node' may be changed by the optimisation).
     */
    node = Binary_optim( node );
    expr = &Tree[node];
    op = expr->op;

    /* Section 5.4.3: insert fetch operators above all indexing; the
     * fetch operators in lval contexts are removed later.
     */
    if( op == IL_WIDX || op == IL_BIDX || op == IL_RBIDX ) {
        e = &Tree[++alloc];
        e->op = (op == IL_WIDX) ? IL_WFETCH : IL_BFETCH;
        e->left = node;
        e->right = 0;
        e->degree = expr->degree;
        node = alloc;
    }
    
    return( node );
}

/* Optimise the expression tree. If the expression has side effects,
 * add a IL_RQD node to the top of the tree. This is used by the
 * statement (flow) optimiser.
 */
int
Expr_optim(int node)
{
    struct Expr *expr, *e;

    node = Expr_walk( node );
    expr = &Tree[node];
    if( expr->degree == HASEFF ) {
        e = &Tree[++alloc];
        e->op = IL_RQD;
        e->left = node;
        e->right = 0;
        node = alloc;
    }
    return( node );
}

/* ============= */

char *opcstr[] = {
    "=", "/=", "*=", "%=", "+=", "-=", "<<=", ">>=",  /*   1-15  */
    "&=", "|=", "^=", "==", "!=", ">",  ">=", "<=",   /*  17-31  */
    "<",  ">>", "<<", "|",  "&",  "^",  "/",  "%",    /*  33-47  */
    "++pre", "--pre", "++post", "--post",             /*  49-55  */
    "~",  "!",  "[",  "{",  ",",  "?",  ":",  "||",   /*  57-71  */ 
    "&&", "+",  "-",  "*",  "LV", "RV", "{=",         /*  73-85  */
    "By_fet", "Wd_fet", "(nargs", "(", "mark", "",    /*  87-97  */
    "R=", "R/=", "R*=", "R%=", "R+=", "R-=", "R<<=",  /*  99-111 */
    "R>>=", "R&=", "R|=", "R^=", "R>>", "R<<", "R-",  /* 113-125 */  
    "R{=", "R/", "R%", "R{", "N,", "&~", "ADDR",      /* 127-139 */
    "U-"                                              /* 141     */
};

static int indent;
 
void
Expr_print( int node )
{
    struct Expr *expr;
    int ref, i;
    
    if( node == 0 ) return;
    
    for( i = 0; i < indent; ++i )
        fprintf( stderr, "  " );

    expr = &Tree[node];
    ref = expr->op;
    if( ref & 1 ) {
        /* interior node */
        fprintf( stderr, "%s\n", opcstr[ref>>1] );
        ++indent;
        if( BINARY(ref) ) {
            Expr_print(expr->right);
            Expr_print(expr->left);
        }
        else if( UNARY(ref) )
            Expr_print(expr->left);
        --indent;
    }
    else {
        /* leaf node */
        if( expr->degree == -1 )
            fprintf( stderr, "C%o = %d\n", expr->op, expr->left );
        else
            fprintf( stderr, "A%o\n", expr->op );
    }
}

