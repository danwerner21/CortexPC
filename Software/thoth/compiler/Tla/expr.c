#include "stdio.h"

#include "tla.h"
#include "tokens.h"

struct Value Result, Oper;

void Term( void )
{
    int sign;

    sign = 1;
    while( TRUE ) {
        switch( token ) {
        
        case MINUS:
            sign = -sign;
            Lex();
            continue;
        
        case NUMBER:
            break;
            
        case ID:
            if( Sym->type == LABEL || Sym->type == EXTERNAL || Sym->type == GLOBAL ) {
                Oper.val = Sym->val;
                Oper.seg = Sym->seg;
            }
            else if( Sym->type == OPCODE ) {
                Oper.val = Sym->val;
                Oper.seg = RB_ABS;
            }
            else {
                Asm_error("Undefined identifier", 0 );
            }
            break;
            
        default:
            Asm_error( "Unexpected token in operand", 0 );
        }
        Oper.val *= sign;
        Lex();
        return;
    }
}

int
Combine( int op )
{
    switch( op ) {

    case PLUS:
        if( Result.seg == RB_ABS && Oper.seg != UNDEFINED )
            return Oper.seg;
        else if( Oper.seg != RB_ABS )
            return UNDEFINED;
        else
            return Result.seg;
            
    case MINUS:
        if( Result.seg == Oper.seg )
            return RB_ABS;
        if( Oper.seg == RB_ABS )
            return Result.seg;
        else
            return UNDEFINED;
    
    case ASTERISK:      case MODULO:        case DIVIDE:      
    case AMPERSAND:     case BITWISE_OR:    case EXCLUSIVE_OR:  
    case LEFT_SHIFT:    case RIGHT_SHIFT:
        if( Result.seg == RB_ABS && Oper.seg == RB_ABS ) {
            return RB_ABS;
        }
        if( Pass == 2 ) {
            Asm_error( "Operand not absolute", 0 );
        }
        break;
    }
    return UNDEFINED;
}

void
Expr( void )
{
    int op;

    Term();
    Result = Oper;
    
    while( TRUE ) {
    
        switch( op = token ) {
        case PLUS: case MINUS: case ASTERISK: case DIVIDE: case MODULO:
        case AMPERSAND: case BITWISE_OR: case EXCLUSIVE_OR: case LEFT_SHIFT:
        case RIGHT_SHIFT:
            Lex();
            break;

        default:
            if( Pass == 2 && Result.seg == UNDEFINED ) {
                Asm_error( "Undefined expression", 0 );
            }
            return;
        }

        Term();
        Result.seg = Combine( op );
        
        switch( op ) {
        case PLUS:          Result.val  += Oper.val; break;
        case MINUS:         Result.val  -= Oper.val; break;
        case ASTERISK:      Result.val  *= Oper.val; break;
        case MODULO:        Result.val  %= Oper.val; break;
        case AMPERSAND:     Result.val  &= Oper.val; break;
        case BITWISE_OR:    Result.val  |= Oper.val; break;
        case EXCLUSIVE_OR:  Result.val  ^= Oper.val; break;
        case LEFT_SHIFT:    Result.val <<= Oper.val; break;
        case RIGHT_SHIFT:   Result.val >>= Oper.val; break;
        
        case DIVIDE:
            if( Oper.val == 0 ) {
                Asm_error( "Divide by zero", 0 );
                Result.val = 0;
                Result.seg = UNDEFINED;
            }
            else 
                Result.val  /= Oper.val;
            break;
        }
    }
}

int
Abs_expr( void )
{
    Expr();
    if( Result.seg != RB_ABS ) {
        Asm_error( "Value not constant", 0 );
    }
    return( Result.val );
}
