
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code.h"
#include "../Parse/il_opc.h"

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

struct Symbol*
Is_constant(struct Expr *expr)
{
    struct Symbol *sym;
    
    if( expr->op & 1 )
        return( NULL );
        
    sym = (struct Symbol*)expr;
    if( (sym->type & TYPE_FIELD) == CONST_TYPE )
        return( sym );

    return( NULL );
}

struct Expr *operands[16];
int op_index;

struct Expr *
Expr_alloc( void )
{
    struct Expr *p;

    p = (struct Expr *)malloc(sizeof(struct Expr));
    memset(p, 0, sizeof(struct Expr));
    return(p);
}

struct Expr *
Build_tree(void)
{
    int ref, i, tmp = 100;
    struct Symbol *sym;
    struct Expr *expr;
    extern int In_twit; // currently processing twit statement

    i = 0;
    operands[0] = NULL;
    while( (ref = GetR()) != 0 ) {
        if( ref & 1 ) {
            expr = Expr_alloc();
            expr->op = ref;
            if( BINARY(ref) ) {
                expr->right = operands[--i];
                expr->left  = operands[--i];
            }
            else if( UNARY(ref) ) {
                expr->left  = operands[--i];            
            }
            operands[i++] = expr;
        }
        else {
            if( (sym = Sym_find( ref, FALSE )) == NULL ) {
                if( In_twit )
                    sym = Sym_label( ref );
                else
                    Error("undefined operand in expression");
            }
            operands[i++] = (struct Expr *)sym;
        }
    }
    //Expr_print( operands[0] );
    return operands[0];
}


/* ============= */

char *opcstr[] = {
    "=", "/=", "*=", "%=", "+=", "-=", "<<=", ">>=",  /*   1-15  */
    "&=", "|=", "^=", "==", "!=", ">",  ">=", "<=",   /*  17-31  */
    "<",  ">>", "<<", "|",  "&",  "^",  "/",  "%",    /*  33-47  */
    "++pre", "--pre", "++post", "--post",             /*  49-55  */
    "~",  "!",  "[",  "{",  ",",  "?",  ":",  "||",   /*  57-71  */ 
    "&&", "+",  "-",  "*",  "LV", "RV", "{=",         /*  73-85  */
    "By_fet", "Wd_fet", "(nargs", "(", "mark", "rqd", /*  87-97  */
    "R=", "R/=", "R*=", "R%=", "R+=", "R-=", "R<<=",  /*  99-111 */
    "R>>=", "R&=", "R|=", "R^=", "R>>", "R<<", "R-",  /* 113-125 */  
    "R{=", "R/", "R%", "R{", "N,", "&~", "ADDR",      /* 127-139 */
    "U-"                                              /* 141     */
};

static int indent;
    
void
Expr_print( struct Expr *expr )
{
    int ref, i;
    
    if( expr == NULL ) return;
    
    for( i = 0; i < indent; ++i )
                    fprintf( stderr, "  " );

    ref = expr->op;
    if( ref & 1 ) {
        /* interior node */
        fprintf( stderr, "%s (%02x-%d)\n", opcstr[ref>>1],
            expr->resultin ? expr->resultin->addrmod : 0,
            expr->resultin ? expr->resultin->value   : 0 );
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
        fprintf( stderr, "O%d\n", expr->op );
    }
}

