
#include <stdio.h>

#include "code.h"
#include "../Parse/il_opc.h"
#include "ti_opcodes.h"

/* Globals */
intptr_t Action_stack[32];
intptr_t *Action_stack_ptr, *Argument_ptr;

struct Symbol *Scratch, *Right, *Left, *Destination;
struct Symbol *Temp_1, *Temp_2, *Temp_3, *Temp_4, *Temp_5;

struct Symbol *Return_val_locn = &Sym_pseudo[17];
struct Symbol *Nargs_locn  = &Sym_pseudo[16];
struct Symbol *Const_one   = &Sym_pseudo[23];
struct Symbol *Constant_2  = &Sym_pseudo[24];
struct Symbol *Constant_4  = &Sym_pseudo[25];
struct Symbol *Constant_8  = &Sym_pseudo[26];
struct Symbol *Constant_16 = &Sym_pseudo[27];


struct Symbol *R[16] = {
    &Sym_pseudo[0],  &Sym_pseudo[1],  &Sym_pseudo[2],  &Sym_pseudo[3], 
    &Sym_pseudo[4],  &Sym_pseudo[5],  &Sym_pseudo[6],  &Sym_pseudo[7], 
    &Sym_pseudo[8],  &Sym_pseudo[9],  &Sym_pseudo[10], &Sym_pseudo[11], 
    &Sym_pseudo[12], &Sym_pseudo[13], &Sym_pseudo[14], &Sym_pseudo[15]
};

typedef struct Symbol **SYMPP;
typedef intptr_t *INTP;

enum { left = 1, right, dest, scratch, const_1, value_1, const_2,   
    const_4, const_8, value_8, const_16, val_left, const_val_right,
    val_dest, r11, r12, temp1, temp2, temp3, temp4, temp5,     
    stk1, stk2, stk3, arg1, arg2, arg3, return_val_locn, addrmod_dest
};

/* Return address of the operand, which is either a pointer to a
 * symbol or a pointer to an int.
 */
intptr_t *
Expand( int operand )
{    
    switch( operand )
    {
        case    left:       return( (INTP)&Left );
        case    right:      return( (INTP)&Right );
        case    dest:       return( (INTP)&Destination );
        case    scratch:    return( (INTP)&Scratch );
        case    const_1:    return( (INTP)&Const_one );
        case    value_1:    return( (INTP)&Const_one->const_value);
        case    const_2:    return( (INTP)&Constant_2 );
        case    const_4:    return( (INTP)&Constant_4 );
        case    const_8:    return( (INTP)&Constant_8 );
        case    value_8:    return( (INTP)&Constant_8->const_value );
        case    const_16:   return( (INTP)&Constant_16 );
        case    val_left:   return( (INTP)&Left->value );
        case    const_val_right: return( (INTP)&Right->const_value );
        case    val_dest:   return( (INTP)&Destination->value );
        case    r11:        return( (INTP)(R+11) );
        case    r12:        return( (INTP)(R+12) );
        case    temp1:      return( (INTP)&Temp_1 );
        case    temp2:      return( (INTP)&Temp_2 );
        case    temp3:      return( (INTP)&Temp_3 );
        case    temp4:      return( (INTP)&Temp_4 );
        case    temp5:      return( (INTP)&Temp_5 );
        case    stk1:       return( Action_stack_ptr );
        case    stk2:       return( Action_stack_ptr+1 );
        case    stk3:       return( Action_stack_ptr+2 );
        case    arg1:       return( Argument_ptr );
        case    arg2:       return( Argument_ptr+1 );
        case    arg3:       return( Argument_ptr+2 );
        case    return_val_locn: return( (INTP)&Return_val_locn );
        case    addrmod_dest: return( (INTP)&Destination->addrmod );
        default:            return( (INTP)&Scratch );
    }
}

#define WSD_AUTO 255

/* If the symbol is a register or a register-based auto variable,
 * return the register pseudo symbol; otherwise return NULL.
 */
struct Symbol *
Reg(struct Symbol *sym)
{
    if( (sym->addrmod & 0x30) == 0 )
        return( sym );
    if( sym->addrmod == 0x2f && sym->value >= 0 && sym->value <=30 )
        return( R[sym->value>>1] );
    if( sym->addrmod == 0x2b &&  (sym->type & WSD_FIELD) == WSD_AUTO
        && sym->value >= 0 && sym->value <=30 )
            return( R[sym->value>>1] );
    return( NULL );
}

enum { add = 2, backplug, code_var, copy, constant, reg, def_label,   
    get_reg, reg_num, hop, neg, complement, set, shift, sub
};

void
Action( int operator, INTP op1, INTP op2, int op3 )
{
    struct Symbol **sop1 = (SYMPP)op1, **sop2 = (SYMPP)op2;
    int n;
    
    switch( operator )
    {
        case add:           *op1 += *op2; break;
        case backplug:      Backplug_gen( *sop1 );
        case code_var:      Code( *(INTP)Expand( op3 ), *sop1, *sop2 ); break;
        case copy:          *op1 = *op2; break;
        case constant:      *sop1 = Constant( *op2 ); break;
        case reg:           *sop1 = R[*op2]; break;
        case def_label:     *sop1 = Int_label(); break;
        case get_reg:       *sop1 = R[(*sop2)->addrmod & 0xf]; break;
        case reg_num:       *op1 = Reg(*sop2)->addrmod; break;
        case hop:           Hop_gen( op3, *sop1 ); break;
        case neg:           *op1 = -*op1; break;
        case complement:    *op1 = ~*op1; break;
        case set:           *op1 = op3; break;
        case shift:         (n=*op2)<0 ? *op1 >>= -n : (*op1 <<= n); break;
        case sub:           *op1 -= *op2; break;
    }
}

int
Is_in(struct Symbol *a, struct Symbol *b)
{
    return( a->addrmod == b->addrmod && a->value == b->value );
}

int
Volatile(struct Symbol *sym)
{
    return( FALSE );
}

/* Test codes */
enum { const_or_str = 2, in_reg, is_arg1, is_arg2, is_auto, is_const_0,   
    is_const_1, is_const_2, is_constant, is_extrn, is_gt_0, is_gt_15,     
    is_in_arg1, is_in_dest, is_left, is_neg_1, is_right, is_string,    
    is_volatile
};

int
Test( int operator, struct Symbol *op1 )
{
    int type;

//fprintf(stderr, "t %d\n", operator);
    type = op1->type & TYPE_FIELD;
    switch( operator )
    {
        case const_or_str:  return( type == STRING_TYPE || type == CONST_TYPE );
        case in_reg:        return( Reg( op1 ) != NULL );
        case is_auto:       return( type == AUTO_TYPE );
        case is_const_0:    return( type == CONST_TYPE && op1->const_value == 0 );
        case is_const_1:    return( type == CONST_TYPE && op1->const_value == 1 );
        case is_const_2:    return( type == CONST_TYPE && op1->const_value == 2 );
        case is_constant:   return( type == CONST_TYPE );
        case is_extrn:      return( type == EXTRN_TYPE );
        case is_gt_0:       return( type == CONST_TYPE && op1->const_value > 0  );
        case is_gt_15:      return( type == CONST_TYPE && op1->const_value > 15 );
        case is_in_arg1:    return( Is_in( op1, *(SYMPP)Argument_ptr ) ) ;
        case is_in_dest:    return( Is_in( op1, Destination ) ) ;
        case is_left:       return( op1 == Left );
        case is_neg_1:      return( type == CONST_TYPE && op1->const_value == -1 );
        case is_right:      return( op1 == Right );
        case is_string:     return( type == STRING_TYPE );
        case is_volatile:   return( Volatile( op1 ) );
        default:            return( TRUE );
    }
}

/* Action_info holds information about the selected action
 * Values are et in Check_conditions and used in Test_and_act
 */
struct {
    int action_ptr;
    int action_count;
} Action_info;

#define CONTINUE   0x8000
#define COUNT      0x7c00
#define TEST_OP    0x03e0
#define TEST_CODE  0x001f

extern unsigned short Op_list[];
extern unsigned short Conditions[];
extern unsigned short Actions[];

/* Until the first true condition is found, call Test to
 * perform the tests for a condition.
 * If any test is false, skip the remaining tests and
 * action pointer (if any) and check the next condition.
 * Set up the action count and the pointer to the set
 * of actions when the true condition is found.
 */
void
Check_conditions( int op )
{
    int condition, result;
    unsigned short *condition_ptr;
    
    condition_ptr = &Conditions[Op_list[op>>1]];
    
    while(1)
    {
        condition = *condition_ptr++;
        Action_info.action_count = (condition&COUNT) >> 10;
        
        if( (op = condition&TEST_CODE) )
        {
            result = Test( op, *(SYMPP)Expand( (condition&TEST_OP)>>5 ) );
            
            while( result==TRUE && (condition&CONTINUE) )
            {
                condition = *condition_ptr++;
                result &= Test( condition&TEST_CODE, *(SYMPP)Expand( (condition&TEST_OP)>>5 ) );
            }
        }
        else result = TRUE;
        
        if( result==TRUE )
        {
            Action_info.action_ptr = *condition_ptr;
            return;
        }

        while( condition&CONTINUE )   /* skip remaining tests */
            condition = *condition_ptr++;
        if( Action_info.action_count ) /* skip action ptr */
            ++condition_ptr;
    }
}

#define OP3_EXISTS 0x8000
#define OPCODE     0x7c00
#define OP1        0x03e0
#define OP2        0x001f
#define CODE       0
#define CALL       1

/* Increment Action_stack_ptr and Argument_ptr by the
 * value in nargs.
 * Call Check_conditions to perform the tests and use
 * information set up by Check_conditions to perform
 * the required action.
 * If the action is CODE, call Code.
 * Otherwise, call Action to perform the actions.
 */
void
Test_and_act( int operator, int nargs )
{
    intptr_t *op1, *op2;
    unsigned short *action_ptr;
    int action_word, action_count;
    intptr_t *old_args;
    
    old_args = Argument_ptr;
    Argument_ptr = Action_stack_ptr;
    Action_stack_ptr += nargs;
    
    Check_conditions( operator );
    action_ptr   = &Actions[Action_info.action_ptr];
    action_count = Action_info.action_count;
    
    while( action_count-- )
    {
        action_word = *action_ptr++;
        operator = (action_word&OPCODE) >> 10;
        op1 = Expand( (action_word&OP1) >> 5 );
        op2 = Expand( (action_word&OP2) );
        
        switch(operator)
        {
            case CALL:
                Test_and_act( *action_ptr++, action_word&OP1 >> 5 );
                break;
            case CODE:
                Code( *action_ptr++, *(SYMPP)op1, *(SYMPP)op2 );
                break;
            default:
                Action( operator, op1, op2, (action_word & OP3_EXISTS) ? *action_ptr++ : 0 );
        }
    }
    
    Action_stack_ptr = Argument_ptr;
    Argument_ptr = old_args;
}

#define OPERATOR & 1

/* If expr is a leaf, return the leaf; if
 * expr is an expr, return the location of
 * the result.
 */
struct Symbol*
Operand( struct Expr *expr )
{
    if( expr->op OPERATOR )
        return expr->resultin;
    else
        return (struct Symbol*)expr;
}

int Last_nargs = -1, Nargs_passed;

/* This recursive function walks the
 * expression tree Left-to-Right (depth first)
 * generating TI machine code for evaluating
 * the expression
 */
void
Arith_expr_gen( struct Expr *tree )
{

    int p, op;
    struct Symbol *label, *done, *destination;

    if( !((op=tree->op) OPERATOR) ) return;
    if( op == IL_MARK) return;
    
    switch( op )
    {
    case IL_QUERY:
      {
        label = Int_label();
        Logical_expr_gen( tree->left, label, FALSE );
        Arith_expr_gen( tree->right->left );
        destination = tree->resultin;
        
        Left = Operand( tree->right->left );
        p = Left->type & TYPE_FIELD;
        if( !Is_in(Left, destination) ) {
            if( p==CONST_TYPE || p==STRING_TYPE )
                if( p==CONST_TYPE && Left->const_value==0 )
                    Code( _CLR_, destination, NULL );
                else if( p==CONST_TYPE && Left->const_value==-1 )
                    Code( _SETO_, destination, NULL );
                else if( Reg(destination) )
                    Code( _LI_, destination, Left );
                else Code( _MOV_, destination, Left );
            else Code( _MOV_, destination, Left ) ;
        }
        
        Jump_gen( (done = Int_label()) ) ;
        Backplug_gen( label );
        Last_nargs = -1;
        
        Arith_expr_gen( tree->right->right );
        Right = Operand( tree->right->right );
        p = Right->type & TYPE_FIELD;
        if( !Is_in(Right, destination) ) {
            if( p==CONST_TYPE || p==STRING_TYPE )
                if( p==CONST_TYPE && Right->const_value==0 )
                    Code( _CLR_, destination, NULL );
                else if( p==CONST_TYPE && Right->const_value==-1 )
                    Code( _SETO_, destination, NULL );
                else if( Reg(destination) )
                    Code( _LI_, destination, Right );
                else Code( _MOV_, destination, Right );
            else Code( _MOV_, destination, Right ) ;
        }
            
        Backplug_gen( done );
        Last_nargs = -1;
        return;
      }

    case IL_EQU:
    case IL_NEQU:
    case IL_GTE:
    case IL_LTE:
    case IL_GT:
    case IL_LT:
    case IL_LOGOR:
    case IL_LOGAND:
    case IL_NOT:
      {
        destination = tree->resultin;
        label = Int_label();
        Code( _CLR_, destination, NULL );
        Logical_expr_gen( tree, label, FALSE );
        Code( _INC_, destination, NULL );
        Backplug_gen( label );
        return;
      }
    }
    
    Arith_expr_gen( tree->left );
    if( BINARY(op) )
    {
        Arith_expr_gen( tree->right );
        Right = Operand( tree->right );
    }
    Left = Operand( tree->left );
    Destination = tree->resultin;
    Scratch = tree->scratch;
    
    if( op == IL_NFCTN_CALL )
    {
        Nargs_passed = TRUE;
        p = Right->const_value;
        if( p == 0 )
        {
            if( Last_nargs != 0 ) Code( _CLR_, Nargs_locn, NULL);
        }
        else if( Last_nargs != -1 )
            switch( p - Last_nargs )
            {
            case -2: Code( _DECT_, Nargs_locn, NULL); break;
            case -1: Code( _DEC_,  Nargs_locn, NULL); break;
            case  0: break;
            case  1: Code( _INC_,  Nargs_locn, NULL); break;
            case  2: Code( _INCT_, Nargs_locn, NULL); break;
            default: Code( _MOV_,  Nargs_locn, Right); break;
            }
        else Code( _MOV_, Nargs_locn, Right);
        Last_nargs = p;
    }
    
    Action_stack_ptr = Action_stack;
    Argument_ptr = Action_stack ;
//fprintf(stderr, "op=%d\n", op);
    Test_and_act( op, 0 );
}

/* Evaluate the left and right subtrees and generate
 * a compare instruction on the results. As the TI-990
 * does not have JGE and JLE instructions, swap the operands
 * to make it JLT and JGT instead.
 */

int Rev_rel[] = {
    IL_NEQU, IL_EQU, IL_LTE, IL_LT, IL_GT, IL_GTE
};
 
int
Reverse_sense( int op )
{
     return( Rev_rel[ (op -  IL_EQU) >> 1 ] );
}

void
Set_condition_register( struct Expr *tree, int jmp_true )
{
    struct Expr *left, *right;
    int op;

    op = tree->op;
    if( jmp_true == FALSE )
        op = tree->op = Reverse_sense( op );

    Arith_expr_gen( (left = tree->left) );
    Arith_expr_gen( (right = tree->right) );
    Code( _C_, (right->op & 1) ? right->resultin : (struct Symbol *)right,
               (left->op & 1)  ? left->resultin  : (struct Symbol *)left );
}

/* If the node is a relational operator, generate code to
 * perform a comparison. Otherwise the node is a variable
 * for which condition code can be set explicitly, or an
 * operator which normally sets condition codes.
 */
void
Relation_gen( struct Expr *tree, struct Symbol *label, int jmp_true )
{
    int op;
   
    switch( (op = tree->op) )
    {
    case IL_GT:
    case IL_GTE:
    case IL_LT:
    case IL_LTE:
    case IL_EQU:
    case IL_NEQU:
        Set_condition_register( tree, jmp_true );
        Hop_gen( tree->op, label );
        break;
        
    default:
        if( op & 1 ) {
            Arith_expr_gen( tree );
            // if top operator is *, fetch value
            if( tree->op == IL_LVAL )
                Code( _MOV_, R[12], tree->resultin );
        }
        else
            Code( _MOV_, R[12], (struct Symbol *)tree ); /* to set indicators */
        Hop_gen( jmp_true ? IL_NEQU : IL_EQU, label );
    }
}

/* Generate code to evaluate the logical expression tree.
 * If the value is !O and jmp true==TRUE, or if the value
 * is O and jmp_true==FALSE, branch to label
 */
void
Logical_expr_gen( struct Expr *tree, struct Symbol *label, int jmp_true )
{ 
    struct Symbol *t;

    switch( tree->op )
    {
    case IL_NOT:
        Logical_expr_gen( tree->left, label, !jmp_true );
        break;

    case IL_LOGAND:
        t = jmp_true ? Int_label() : label;
        Logical_expr_gen( tree->left, t, FALSE );
        Logical_expr_gen( tree->right, label, jmp_true );
        if( jmp_true) Backplug_gen( t );
        break;

    case IL_LOGOR:
        t = jmp_true ? label : Int_label();
        Logical_expr_gen( tree->left, t, TRUE );
        Logical_expr_gen( tree->right, label, jmp_true );
        if( !jmp_true) Backplug_gen( t );
        break;

    default:
        Relation_gen( tree, label, jmp_true );
    }
}
