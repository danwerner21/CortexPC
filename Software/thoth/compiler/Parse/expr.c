//========================

#include <stdlib.h>
#include <string.h>

#include "parse.h"

struct Expr {
    int name;
    int ref;
    int kind;
    int type;
};
#define left  name
#define right kind
#define value name

#define MAXSTACK 100

struct Expr Stack[MAXSTACK];
int idx = 1;

int
Expr_alloc(int ref, int val, int type)
{
    struct Expr *exp;
    int mark;
    
    mark = idx;
    while( Stack[idx].ref != 0 ) {
        idx++;
        if( idx == MAXSTACK )
            idx = 1;
        if( idx == mark) {
            yyerror("expression table overflow");
            exit(1);
        }
    }
    exp = &Stack[idx];
    if( ref == 0 )
        exp->value = val;
    else
        exp->ref   = ref;
    exp->kind  = 0;
    exp->type  = type;
    return idx;
}

#include <stdio.h>

int
Expr_leaf(int val, int type, int kind)
{
    int idx;
    struct Expr *exp;

    idx = Expr_alloc(0, val, type);
    //printf("leaf %d, %d, %d -> %d\n", val, type, kind, idx);
    exp = &Stack[idx];
    exp->value = val;
    exp->kind = kind;
    exp->ref = 1000;
    return idx;
}

#include "il_opc.h"

int
Expr_eval(int opc, int left, int right)
{
    int ref;
    
    switch( opc ) {
    case IL_ADD:    return left + right;
    case IL_SUB:    return left - right;
    case IL_MUL:    return left * right;
    case IL_DIV:    return left / right;
    case IL_MOD:    return left % right;
    case IL_SHL:    return left << right;
    case IL_SHR:    return left >> right;
    case IL_BITOR:  return left | right;
    case IL_BITAND: return left & right;
    case IL_BITXOR: return left ^ right;
    case IL_NEG:    return -left;
    case IL_NOT:    return !left;
    case IL_BITNOT: return ~left;
    default:
        yyerror("bad constant expression");
        return 0;
    }
}

int
Expr_node(int ref, int type, int left, int right)
{
    int idx;
    struct Expr *exp, *l_exp, *r_exp;

    //printf("node %d, %d, %d", ref, left, right);
    l_exp = &Stack[left];
    r_exp = &Stack[right];
    if( right == 0 ) {
        if( l_exp->kind == TCONST ) {
            l_exp->value = Expr_eval(ref, l_exp->value, 0);
            return left;
        }
    }
    else if( l_exp->kind == TCONST && r_exp->kind == TCONST && ref != IL_COLON ) {
        l_exp->value = Expr_eval(ref, l_exp->value, r_exp->value);
        //printf(" = %d\n", left);
        return left;
    }
    idx = Expr_alloc(ref, 0, type);
    exp = &Stack[idx];
    exp->left  = left;
    exp->right = right;
    //printf(" -> %d\n", idx);
    return idx;
}

void
Expr_check(int node)
{
    struct Expr *exp, *fun;
    
    if( node == 0 ) return;
    exp = &Stack[node];
    //printf("expr_ref %d,%d\n", node, exp->ref);
    if( exp->ref & 1 ) {
        if( exp->ref == IL_NFCTN_CALL ) {
            fun = &Stack[exp->right];
            if( fun->ref == 1000 ) {
                fun->ref = Ref_ident(fun->value, 'g', MAY);
                Expr_check( exp->left );
                return;
            }
        }
        if( exp->ref == IL_ASSIGN ) {
            fun = &Stack[exp->right];
            if( fun->ref == IL_BIDX ) {
                exp->ref = IL_ASBYTE;
            }
        }
        Expr_check( exp->left );
        Expr_check( exp->right );
        return;
    }
    if( exp->ref == 1000 ) {
        switch( exp->kind ) {
        case TCONST: exp->ref = Ref_const(exp->value);          break;
        case TSTR:   exp->ref = Ref_string(exp->value);         break;
        case TNAME:  exp->ref = Ref_ident(exp->value, 0, MUST); break;
        }
    }
}

void
Expr_put1(int node)
{
    struct Expr *exp;

    if( node == 0 ) return;
    exp = &Stack[node];
    if( exp->ref & 1 ) {
        Expr_put1( exp->left );
        Expr_put1( exp->right );
    }
    //printf("put %x\n", exp->ref);
    PutR( exp->ref );
    exp->ref = 0; /* free */
}

void
Expr_put(int top)
{
    //printf("expr %d\n", top);
    //Expr_check( top );
    Expr_put1( top );
    Put( 0 );
}

int
Expr_const(int node)
{
    struct Expr *exp;
    
    exp = &Stack[node];
    //printf("const %d, %d, %d, %d\n", exp->value, exp->ref, exp->kind, exp->type);
    if( exp->kind != TCONST ) 
        yyerror("expression not constant");
    return exp->value;
}

void
Expr_extrn(int node)
{
    struct Expr *exp;
    
    exp = &Stack[node];
    //printf("extrn init %d, %d, %d, %d\n", exp->value, exp->ref, exp->kind, exp->type);
    if( exp->kind == TCONST ) {
        DPutC(exp->value);
    }
    else if( exp->kind == TNAME ) {
        DPut('g'); DPutW(exp->name);
    }
    else if( exp->kind == TSTR ) {
        DPut('"'); DPutW(exp->name);
    }
    else {
        yyerror("expression not constant");
        DPutC(0);
    }
}
