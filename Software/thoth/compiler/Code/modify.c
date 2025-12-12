
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code.h"
#include "../Parse/il_opc.h"

int Modify_calls(struct Expr *tree);

int
Process_call(struct Expr *fcall)
{
    struct Expr *asgn, *arg, *lst, *tmp;
    int i, j, n, m;

    /* test argument list for sub-calls */
    arg = fcall->left;
    lst = NULL;
    i = 0;
    m = n = ((struct Symbol *)fcall->right)->const_value;

    while( 1 ) {
        arg = arg->left;
        if( arg->op == IL_MARK ) break;
        arg->op = IL_NULL;

        j = Modify_calls( arg->right );
        i = (j>i) ? j : i;
        if( i >= n ) {
            /* update argument to use safe storage */
            tmp = arg->right->right;
            arg->right->right = (struct Expr*)Next_temp();
            /* build re-assignemnt from safe to arg pos */
            asgn = Expr_alloc();
            asgn->op = IL_ASSIGN;
            asgn->left = arg->right->right;
            asgn->right = tmp; 
            /* add assignment to list */
            tmp = Expr_alloc();
            tmp->op = IL_NULL;
            tmp->left = lst;
            tmp->right = asgn;
            lst = tmp;
        };
        --n;
    }
    /* insert re-assign list above arg nodes */
    if( lst != NULL ) {
        tmp = fcall->left->left;
        fcall->left->left = lst;
        while( lst->left ) lst = lst->left;
        lst->left = tmp;
    }

    return( m );
}

int
Modify_calls(struct Expr *tree)
{
    int op, a1, a2;
    
    if( tree == NULL )
        return 0;
    
    op = tree->op;
    if( !(op & 1) || NONARY(op) )
        return 0;
    
    if( UNARY(op) ) {
        return Modify_calls( tree->left );
    }
    
    if( op == IL_NFCTN_CALL || op == IL_FCTN_CALL ) {
        return( Process_call(tree) );
    }
    else {
        a1 = Modify_calls( tree->left  );
        a2 = Modify_calls( tree->right );
        return( (a1>a2) ? a1 : a2 );
    }
}

struct Symbol *
Assign_arg(int argno)
{
    struct Symbol *sym;
    int offset;
    
    sym = Sym_alloc();
    offset = (argno < 8) ? (11 - argno) * 2 : (7 - argno) * 2;
    sym->value = offset;
    sym->addrmod = AUTO_ADDRMOD;
    sym->type = AUTO_TYPE;
    return( sym );
}

void Modify_walk(struct Expr *tree);

int
Modify_arg( struct Expr *comma )
{
    struct Expr *asgn;
    int n;

    if( comma->op == IL_MARK ) return( 0 );
    
    n = Modify_arg( comma->left ) + 1;

    Modify_walk( comma->right );
    //if( comma->right->op & 1 ) {
    //    comma->right->resultin = Assign_arg( n );
    //    return( n );
    //}
    asgn = Expr_alloc();
    asgn->op = IL_ASSIGN;
    asgn->left = comma->right;
    asgn->right = (struct Expr *)Assign_arg( n );
    comma->right = asgn;    
    return( n );
}

/* Bonkowski pp. 63-64 and figure 4.5
 */
void
Modify_args(struct Expr *fcall)
{
    struct Expr *tmp;
    int n;

    /* insert assignment for each arg */
    n = Modify_arg( fcall->left );

    /* add IL_ADDR op & nargs to call */
    tmp = Expr_alloc();
    tmp->op = IL_ADDR;
    tmp->right = fcall->right;
    tmp->left = fcall->left;
    fcall->left = tmp;
    fcall->right = (struct Expr *)Constant( n );
}

void
Modify_walk(struct Expr *tree)
{
    struct Symbol *sym;
    struct Expr *t;
    int op;
    
    if( tree == NULL )
        return;
    
    op = tree->op;
    if( !(op & 1) || NONARY(op) )
        return;
    
    if( UNARY(op) ) {
        Modify_walk( tree->left );
        return;
    }
    
    if( op == IL_NFCTN_CALL || op == IL_FCTN_CALL )
        return( Modify_args( tree ) );

    Modify_walk( tree->left  );
    Modify_walk( tree->right );

    if( op == IL_BITAND ) {
        if( tree->right->op == IL_BITNOT ) {
            t = tree->right;
            tree->right = t->left;
            tree->op = IL_BITANDN;
            free( t );
            return;
        }
        if( tree->left->op == IL_BITNOT ) {
            t = tree->left;
            tree->left = tree->right;
            tree->right = t->left;
            tree->op = IL_BITANDN;
            free( t );
            return;
        }
    }
    return;
}

struct Expr *
Modify(struct Expr *tree)
{
    struct Expr *t;
    
    if( tree == NULL ) return( tree );

    t = tree;
    if( tree->op == IL_RQD ) {
        tree = tree->left;
        free( t );
    }
    
    Modify_walk( tree );
    Modify_calls( tree );

    return( tree );
}
