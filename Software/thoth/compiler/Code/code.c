/* TI990 code generation */

#include <stdio.h>
#include <stdlib.h>

#include "code.h"
#include "load.h"
#include "ti_opcodes.h"


int
Raw_value(struct Symbol *operand)
{
    return( (operand->type & TYPE_FIELD) == CONST_TYPE) ? operand->const_value : operand->value;
}

/* Called from Code and Twit gen to, when necessary output an instruction
 * address. If the operand is a constant, the value from a literal pool
 * is used (and put into a pool if none exists) unless the immediate
 * argument is nonzero. In such cases the immediate operand of the
 * instruction is generated.
 */
void
Cond_addr_gen( struct Symbol *operand, int immediate )
{
    int type, rel_wsd, value;
    
    if( (operand->addrmod & MODE_BITS) != INDEXED_MODE )
        return;
        
    type    = operand->type & TYPE_FIELD;
    rel_wsd = operand->type & REL_WSD_FIELD;
    value   = Raw_value( operand );
    
    //fprintf(stderr, "type=%x, %x, %x, %d\n", operand->type, type, rel_wsd, value );
    if( type == CONST_TYPE || type == STRING_TYPE ) {
//fprintf(stderr, "type=%x, %x, %x, %d\n", operand->type, type, rel_wsd, value );
        if( immediate )
        {
            operand->type  = type | (ADDR_RELDESC<<8) | WSD_CODE | SYMBOL_DEFINED;
            operand->value = *Counter;
            if( type == CONST_TYPE ) rel_wsd = 0;
        }
        else if( !(operand->type & SYMBOL_DEFINED) )
        {
            //fprintf(stderr, "type=%x, %x, %x\n", operand->type, type, rel_wsd );
            Set_loc( WSD_DATA );
            operand->value = *Counter;
            if( rel_wsd == 0 )
                Load_word( value );
            else
                Rload_word( rel_wsd>>8, rel_wsd&WSD_FIELD, value );
            rel_wsd = ADDR_RELDESC<<8 | WSD_DATA;
            Set_loc( WSD_CODE );
            operand->type  = type | rel_wsd | SYMBOL_DEFINED;
            value = operand->value;
        }
        else value = operand->value;
    }
        
    //fprintf(stderr, "rel_wsd=%x, %x, %x, %x\n", rel_wsd, rel_wsd>>8, rel_wsd&WSD_FIELD, value );
    if( rel_wsd == 0 )
        Load_word( value );
    else {
        Rload_word( rel_wsd>>8, rel_wsd&WSD_FIELD, value);
    }
}

/* Output one TI instruction. It may be more than one word in length;
 * Right_oper and left_oper are pointers to symbo1 tab1e entries, where
 * "1eft" and "right" refer to the order in which the operands appear
 * in the machine instruction_
 */
void
Code( int op, struct Symbol *left_oper, struct Symbol *right_oper )
{
    int disp, lop;
    struct Symbol *rr;
    
    Check_addressability( TRUE, 0 );
    if( left_oper  && (rr=Reg(left_oper))  ) left_oper  = rr;
    if( right_oper && (rr=Reg(right_oper)) ) right_oper = rr;
    
    switch( op )
    {
        case _A_:
        case _AB_:
        case _C_:
        case _CB_:
        case _S_:
        case _SB_:
        case _SOC_:
        case _SOCB_:
        case _SZC_:
        case _SZCB_:
        case _MOV_:
        case _MOVB_:
        {
            //fprintf(stderr, "op=%04x, left op=%d, right op=%d\n", op, left_oper->addrmod, right_oper->ref);
            Load_word( op | (left_oper->addrmod << 6) | right_oper->addrmod );
            Cond_addr_gen( right_oper, FALSE);
            Cond_addr_gen( left_oper,  FALSE);
            break;
        }
        case _COC_:
        case _CZC_:
        case _XOR_:
        case _MPY_:
        case _DIV_:
        {
            //fprintf(stderr, "op=%04x, left op=%d, right op=%d\n", op, left_oper->addrmod, right_oper->ref);
            Load_word( op | (left_oper->addrmod << 6) | right_oper->addrmod );
            Cond_addr_gen( right_oper, FALSE);
            break;
        }
        case _B_:
        case _BL_:
        case _BLWP_:
        case _CLR_:
        case _SETO_:
        case _INV_:
        case _NEG_:
        case _ABS_:
        case _SWPB_:
        case _INC_:
        case _INCT_:
        case _DEC_:
        case _DECT_:
        case _X_:
        case _LDS_:
        case _LDD_:
        {
            Load_word( op | left_oper->addrmod );
            Cond_addr_gen( left_oper, FALSE);
            break;
        }
        case _LDCR_:
        case _STCR_:
        case _XOP_:
        {
            Load_word( op | (left_oper->const_value << 6) | right_oper->addrmod );
            Cond_addr_gen( right_oper, FALSE);
            break;
        }
        case _SBO_:
        case _SBZ_:
        case _TB_:
        {
            lop = left_oper->const_value;
            disp = lop >= 0 ? lop & 0xFF : (~-lop) & 0xFF + 1;
            Load_word( op | disp );
            break;
        }
        case _JEQ_:
        case _JGT_:
        case _JH_:
        case _JHE_:
        case _JL_:
        case _JLE_:
        case _JLT_:
        case _JMP_:
        case _JNC_:
        case _JNE_:
        case _JNO_:
        case _JOC_:
        case _JOP_:
        {
            disp = (left_oper->value - *Counter - 2)/2;
            disp = disp >= 0 ? disp & 0xFF : ((~-disp) & 0xFF) + 1;
            Load_word( op | disp );
            break;
        }
        case _SLA_:
        case _SRA_:
        case _SRC_:
        case _SRL_:
        case _LMF_:
        {
            Load_word( op | (left_oper->const_value << 4) | right_oper->addrmod );
            break;
        }
        case _AI_:
        case _ANDI_:
        case _CI_:
        case _LI_:
        case _ORI_:
        {
            Load_word( op | left_oper->addrmod );
            Cond_addr_gen( right_oper, TRUE );
            break;
        }
        case _LWPI_:
        case _LIMI_:
        {
            Load_word( op );
            Cond_addr_gen( left_oper, TRUE );
            break;
        }
        case _STST_:
        case _STWP_:
        {
            Load_word( op | left_oper->addrmod );
            break;
        }
        case _RTWP_:
        case _IDLE_:
        case _RSET_:
        case _CKON_:
        case _CKOF_:
        case _LREX_:
        {
            Load_word( op );
            break;
        }
        default: Error( "invalid opcode in a twit" ) ;
    }
}
