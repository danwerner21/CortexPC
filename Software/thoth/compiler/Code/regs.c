
/* This file implements the expression register allocator as described in
 * section 4.6.2 of the Bonkowsi thesis. There is a furhter description in
 * his "Porting Zed" paper in the section "expression evaluation".
 *
 * Register allocation is machine specific and the internal constraints are
 * that the left operand of *, / and % must be in register 0 and 1 (for the
 * signed MPYS and DIVS operations, but this is also a good choice for the
 * unsigned MPY and DIV opeations); for the << and >> operations, where when
 * the right operand is not constant it must be in register 0.
 */

#include <stdio.h>

#include "code.h"
#include "../Parse/il_opc.h"

extern struct Symbol *R[];
extern struct Symbol *Return_val_locn;

/* Available registers are kept in a bitvector. Pick the lowest
 * available register and remove it from the the available list.
 * TODO: aliase bit position 11-15 to 5 temp locations.
 */
struct Symbol *
Get_reg(int *regs)
{
    int i, r, m;

    r = *regs;
    m = 1;
    for(i=0; i<16; ++i)
    {
        if( r & 1 ) break;
        r >>= 1;
        m <<= 1;
    }
    if( i == 16 )
        Error("allocator ran out of registers");
    *regs &= ~m;
    return R[i];
}

/* If symbol is located in a register, remove it from the vector
 * of available registers. Return the new vector.
 */
int
Regs_out(struct Symbol *sym, int orig)
{
    int mask;

    // an INDIR sym is always a reg
    if( !((sym->addrmod & 0x30) == 0x10) ) {
        sym = Reg( sym );
    }
    mask = 1 << (sym ? (sym->addrmod & 0xf) : 0);
//fprintf(stderr, "sym %p, sym_reg=%d, mask = %04x\n", sym, 1<<(sym->addrmod&15), mask);
    return( orig & (~mask) );
}

#define BASE(r)  &Sym_pseudo[ ((r)->addrmod & 0xf) ]
#define INDIR(r) &Sym_pseudo[ ((r)->addrmod & 0xf) + 18 ]

/* Walk the the given tree in right-first post-order and allocate
 * registers using the Seti-Ullmann algorithm. Vary on the algorithm
 * to deal with the internal constraints of the TI-990 architecture.
 */
void
Register_allot(struct Expr *tree, int *regs)
{
    struct Expr *t;
    int op, orig, d1, d2;
    
    if( tree == NULL )
        return;
    
    orig = *regs;
    
    op = tree->op;
    if( !(op & 1) || NONARY(op) )
        return;
    
    if( UNARY(op) ) {

        Register_allot( tree->left, regs );
        
        /* special case WFETCH, because it is a no-op in the code table */
        if( op == IL_WFETCH )
            tree->resultin = tree->left->resultin;
        else
            tree->resultin = (tree->left->op & 1) ? BASE(tree->left->resultin) :  Get_reg( regs );
        
        if( op == IL_LVAL ) {
            tree->scratch = tree->resultin;
            tree->resultin = INDIR(tree->scratch);
        }

        *regs = Regs_out( tree->resultin, orig );
        return;
    }
    
    /* Special case 1: deal with specifics of MPY/MPYS */
    if( op == IL_MUL ) {
        *regs = orig & (~3); /* grab reg 0 and reg 1 */
        Register_allot( tree->right, regs );
        *regs = orig & (~3);
        Register_allot( tree->left,  regs );
        if( tree->left->op & 1 ) {
            tree->left->resultin = R[0];
        }
        *regs = orig & (~3);
        tree->resultin = ((orig & 3) != 3) ? Get_reg( regs ) : R[1];
        *regs = Regs_out( tree->resultin, orig );
        return;
    }

    /* Special case 2: deal with specifics of DIV/DIVS. Undo RDIV
     * and RMOD, as this does not help on the TI-990.
     */
    if( op == IL_RDIV || op == IL_RMOD ) {
        t = tree->left;
        tree->left = tree->right;
        tree->right = t;
        op = tree->op = (op == IL_RDIV) ? IL_DIV : IL_MOD;
    }
    if( op == IL_DIV || op == IL_MOD ) {
        *regs = orig & (~3); /* grab reg 0 and reg 1 */
        Register_allot( tree->right, regs );
        *regs = orig & (~3);
        Register_allot( tree->left,  regs );
        if( tree->left->op & 1 ) {
            tree->left->resultin = R[1];
        }
        *regs = orig & (~3);
        tree->resultin = ((orig & 3) != 3) ? Get_reg( regs ) : ((op==IL_DIV) ? R[0] : R[1]);
        *regs = Regs_out( tree->resultin, orig );
        return;
    }
    
    /* Special case 3: deal with >> and << with a variable shift count,
     * which must be located in r0. Undo RSHR and RSHL, as this does
     * not help on the TI-990.
     */
    if( op == IL_RSHR || op == IL_RSHL ) {
        t = tree->left;
        tree->left = tree->right;
        tree->right = t;
        op = tree->op = (op == IL_RSHR) ? IL_SHR : IL_SHL;
    }
    if( op == IL_SHR || op == IL_SHL ) {
        
        *regs = orig & (~1); /* grab reg 0 */
        Register_allot( tree->right, regs );
        *regs = orig & (~1);
        Register_allot( tree->left,  regs );
        if( tree->right->op & 1 ) {
            tree->right->resultin = R[0];
        }
        tree->resultin = (tree->left->op & 1) ? BASE(tree->left->resultin) :  Get_reg( regs );
        *regs = Regs_out( tree->resultin, orig );
        return;
    }
    
    /* Special case 4: deal with function calls. See also Bonkowski
     * section 4.6.1 for the modified expression tree of calls.
     */
    if( op == IL_ADDR ) {
        Register_allot( tree->right, regs );
        *regs = orig;
        Register_allot( tree->left, regs );
        *regs = orig;
        tree->resultin = Get_reg( regs );
        return;
    }
    if( op == IL_NULL ) {
        t = tree->right;
        Register_allot( t, regs );
        /* no need to keep the result in a reg, always on stack */
        t->resultin = (t->right->op & 1) ? t->right->resultin : (struct Symbol *)(t->right);
        tree->resultin = t->resultin;
        *regs = orig;
        Register_allot( tree->left, regs );
        return;
    }

    /* For all other 'regular' cases use the base Seti-Ullmann algorithm */
    d1 =  tree->left->op  & 1;
    d2 = (tree->right->op & 1) << 1;
    
    switch( d1 + d2 ) {
    case 0:
        tree->resultin = Get_reg( regs );
        break;
    case 1:
        Register_allot( tree->left,  regs );
        tree->resultin = BASE(tree->left->resultin);
        break;
    case 2:
        Register_allot( tree->right,  regs );
        tree->resultin = BASE(tree->right->resultin);
        break;
    case 3:
        Register_allot( tree->left, regs );
        if( op == IL_LOGAND || op == IL_LOGOR )
            *regs = orig; // each part is a separate arith expression
        Register_allot( tree->right,  regs );
        tree->resultin = BASE(tree->right->resultin);
        break;
    }
    
    /* Add indirection addrmod to [ and { operators */
    if( op == IL_WIDX || op == IL_BIDX || op == IL_RBIDX ) {
        switch( d1 + d2 ) {
        case 0: case 3: tree->scratch = tree->resultin;  break;
        case 1: tree->scratch = (op == IL_BIDX)  ? Get_reg( regs ) : tree->resultin; break;
        case 2: tree->scratch = (op == IL_RBIDX) ? Get_reg( regs ) : tree->resultin; break;
        }
        tree->resultin = INDIR(tree->scratch);
    }

    *regs = Regs_out( tree->resultin, orig );
    return;
}

struct Expr *
Allocate_registers(struct Expr *tree, struct Symbol *force)
{
    int op, regs = 0x000f;
    
    /* operator-less expressions are meaningful when used in return
     * or select expressions. Insert an assign to forced location.
     */
    if( !(tree->op & 1) || tree->op == IL_WFETCH || tree->op == IL_LVAL ) {
        if( force ) {
            struct Expr *expr = Expr_alloc();
            expr->op = IL_RASGN;
            expr->left = (struct Expr*)force;
            expr->right = tree;
            tree = expr;
        }
        else
            fprintf(stderr, "internal error: operator-less expression\n");
    }

    Register_allot( tree, &regs);
    //Expr_print( tree );

    /* Set the top level result register. Use the forced location
     * if given.
     */
    if( force ) {
        tree->resultin = force;
        return( tree );
    }

    /* no need to keep the result of a top-level assign or function call.
     */
    op = tree->op;
    if( IS_FA(op) ) {
        tree->resultin = (tree->right->op & 1) ? tree->right->resultin
                                               : (struct Symbol *)(tree->right);
    }
    if( IS_RA(op) ) {
        tree->resultin = (tree->left->op & 1)  ? tree->left->resultin 
                                               : (struct Symbol *)(tree->left);
    }
    
    /* Functions always return their value in R12 on the TI990.
     */
    if( op == IL_NFCTN_CALL || op == IL_FCTN_CALL ) {
        tree->resultin = R[12];
    }
    return( tree );
}
