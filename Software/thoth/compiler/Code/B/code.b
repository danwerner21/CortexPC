
\ Action_info holds information about the selected action
\ Values are et in Check_conditions and used in Test_and_act
\
Action_info[2];
ACTION_PTR   = 0;
ACTION_COUNT = 1;

Check_conditions( op )
    \ Until the first true condition is found, call Test to
    \ perform the tests for a condition.
    \ If any test is false, skip the remaining tests and
    \ action pointer (if any) and check the next condition.
    \ Set up the action count and the pointer to the set
    \ of actions when the true condition is found.
{
    extrn Conditions_and_actions, Action_info, Op_list;
    auto condition, result, condition_ptr;
    
    condition_ptr=Conditions_and_actions[ Op_list[op>>1] ];
    
    repeat
    {
        condition = *condition_ptr++;
        ACTION_COUNT[Action_info] = condition&COUNT >> 10
        
        if( op = condition&TEST_CODE )
        {
            result = Test( op, *Expand(condition&TEST_OP>>5) );
            
            while( result==TRUE && condition&CONTINUE )
            {
                condition = *condition_ptr++;
                result &= Test( condition&TEST_CODE, *Expand(condition&TEST_OP>>5) );
            }
        }
        else result = TRUE;
        
        if( result==TRUE )
        {
            ACTION_PTR[Action_info] = *condition_ptr;
            return;
        }

        while( condition&CONTINUE )     \ skip remaining tests
            condition = *condition_ptr++;
        if( ACTION_COUNT[Action_info] ) \ skip action ptr
            ++condition_ptr;
    }
}

\ Globals
Action_stack_ptr;
Argument_ptr;

Test_and_act( operator, nargs )
    \ Increment Action_stack_ptr and Argument_ptr by the
    \ value in nargs.
    \ Call Check conditions to perform the tests and use
    \ information set up by Check conditions to perform
    \ the required action.
    \ If the action is CODE, call Code.
    \ Otherwise, call Action to perform the actions.
{
    extrn Action_stack_ptr, Argument_ptr, Action_info;
    auto op1, op2, action_ptr, action_count;
    auto action_word, old_args;
    
    old_args = Argument_ptr;
    Argument_ptr = Action_stack_ptr;
    Action_stack_ptr += nargs;
    
    Check_conditions( operator );
    action_ptr = ACTION_PTR[Action_info];
    action_count = ACTION_COUNT[Action_info];
    
    while( action_count-- )
    {
        action_word = *action ptr++;
        operator = action_word&OPCODE >> 10;
        op1 = Expand( action_word&OP1 >> 5 );
        op2 = Expand( action_word&OP2 );
        
        select(operator)
        {
            case CALL:
                Test_and_act( *action_ptr++, action_word&OP1 >> 5 );
            case CODE:
                Code( *action ptr++, *op1, *op2 );

            default:
                Action( operator, op1, op2, action word&OP3_EXISTS ? *action_ptr++ : 0 );
        }
    }
    
    Action_stack_ptr = Argument_ptr;
    Argument_ptr = old_args;
}

\ Globals
Action_stack[10];
Scratch;

Arith_expr_gen( tree )
    \ This recursive function walks the
    \ expression tree Left-to-Right (depth first)
    \ generating TI machine code for evaluating
    \ the expression
{
    extrn Last_nargs, Nargs_passed; Nargs_locn;
    extrn Right, Left, Destination;
    extrn Action_stack_ptr, Argument_ptr, Scratch;
    extrn Action_stack; -
    auto destination, p, label, op, done;

    if( !((op=OP[tree]) OPERATOR) ) return;
    if( op == IL_ARG_MARK) return;
    
    select( op )
    {
    case IL_QUERY:
      {
        label = Tree_label();
        Logical_expr_gen( LEFT[tree], label, FALSE );
        Arith_expr_gen( LEFT[RIGHT[tree]] );
        destination = RESULTIN[tree];
        
        Left = Operand( LEFT[RIGHT[tree]] );
        p = TYPE[Left] & TYPE_FIELD;
        if( !Is_in(Left, destination) )
            if( p==CONST_TYPE || p==STRING_TYPE )
                if( p==CONST_TYPE && CONST_VALUE[Left]==O )
                    Code( .CLR., destination );
                else if( P==CONST_TYPE && CONST_VALUE[Left]==-1 )
                    Code( .SETO., destination );
                else if( Reg(destination) )
                    Code( .LI., destination, Left );
                else Code( .MOV., destination, Left );
            else Code( .MOV., destination, Left ) ;
        
        Hop_gen( .JMP., done=Tree 1abe1 ( ) ) ;
        Backplug gen( label );
        Last_nargs = -1;
        
        Arith_expr_gen( RIGHT[RIGHT[tree]] );
        Right = Operand( RIGHT[RIGHT[tree]] );
        p = TYPE[Right] & TYPE_FIELD;
        if( !Is_in(Right, destination) )
            if( p==CONST_TYPE || p==STRING_TYPE )
                if( P==CONST_TYPE && CONST_VALUE[Right]==O )
                    Code( .CLR., destination );
                else if( P==CONST_TYPE && CONST_VALUE[Right]==-1 )
                    Code( .SETO., destination );
                else if( Reg(destination) )
                    Code( .LI., destination, Right );
                else Code( .MOV., destination, Right );
            else Code( .MOV., destination, Right );
            
        Backplug gen( done );
        Last nargs = -1;
        return;
      }
    case IL EQU:
    case IL-NEQU:
    case IL-GTE:
    case IL-LTE:
    case IL-GT:
    case IL-LT:
    case IL-LOGOR:
    case IL-LOGAND:
    case IL-NOT:
      {
        destination = RESULTIN[tree];·
        label = Tree_label();
        Code( .CLR., destination );
        Logical_expr_gen( tree, label, FALSE );
        Code( .INC., destination );
        Backplug_gen( label );
        return;
      }
    }
    
    Arith_expr_gen( LEFT[tree J );
    if( op NOT_UNARY )
    {
        Arith_expr_gen( RIGHT[tree] );
        Right = Operand( RIGHT[tree] );
    }
    Left = Operand( LEFT[tree]);
    Destination = RESULTIN[tree];
    Scratch = SCRATCH[tree];
    
    if( op == IL_NFCTN_CALL || op == TI_FETCH_NFCTN_VAL)
    {
        Nargs_passed = TRUE;
        p = CONST_VALUE[Right];
        if( p == 0 )
        {
            if( Last_nargs ! = 0 ) Code( .CLR ., Nargs_locn);
        }
        else if( Last_nargs != -1 )
            select( p - Last_nargs )
            {
            case -2: Code( .DECT., Nargs locn);
            case -1: Code( .DEC.,  Nargs_Iocn);
            case  0: 
            case  1: Code( .INC.,  Nargs locn);
            case  2: Code( .INCT., Nargs locn);
            default: Code( .MOV.,  Nargs_Iocn, Right);
            }
        else Code( .MOV., Nargs_locn, Right);
        Last nargs = p;
    }
    
    Action_stack_ptr = Action_stack;
    Argument_ptr = Action_stack ;
    Test_and_act( op, 0 );
}

Expand( operand )
    \ Return address of the operand
{
    extrn Left, Right, Destination, Scratch, R;
    extrn Temp_1, Temp_2, Temp_3, Temp_4, Temp_5, Constant_16;
    extrn Const_one, Constant_2, Constant_4, Constant_8;
    extrn Action_stack_ptr, Argument_ptr, Return_val_locn;
    
    select( operand )
    {
        case    left:       return( &Left );
        case    right:      return( &Right );
        case    dest:       return( &Destination );
        case    scratch:    return( &Scratch );
        case    const_1:    return( &Const_one );
        case    value_1:    return( Const_one+CONST_VALUE);
        case    const_2:    return( &Constant_2 );
        case    const_4:    return( &Constant_4 );
        case    const_8:    return( &Constant_8 );
        case    value_8:    return( Const_8+CONST_VALUE );
        case    const_16:   return( &Constant_16 );
        case    val_left:   return( Left+VALUE );
        case    const_val_right: return( Right+CONST_VALUE );
        case    val_dest:   return( Destination+VALUE );
        case    r11:        return( R+11 );
        case    r12:        return( R+12 );
        case    temp1:      return( &Temp_1 );
        case    temp2:      return( &Temp_2 );
        case    temp3:      return( &Temp_3 );
        case    temp4:      return( &Temp_4 );
        case    temp5:      return( &Temp_5 );
        case    stk1:       return( Action_stack_ptr );
        case    stk2:       return( Action_stack_ptr+1 );
        case    stk3:       return( Action_stack_ptr+2 );
        case    arg1:       return( Argument_ptr );
        case    arg2:       return( Argument_ptr+1 );
        case    arg3:       return( Argument_ptr+2 );
        case    return_val_locn: return( &Return_val_locn );
        case    addrmod_dest: return( Destination+ADDRMOD );
    }
}

Action( operator; op1, op2, op3 )
{
    extrn R;
    
    select( operator )
    {
        case add:           *op1 += *op2;
        case backplug:      Backplug_gen( *op1 );
        case code_var:      Code( *Expand( op3 ), *op1, *op2 );
        case copy:          *op1 = *op2;
        case constant:      *op1 = Constant( *op2 );
        case register:      *op1 = R[*op2];
        case def_label:     *op1 = Tree_label();
        case get_reg:       *op1 = R[ADDRMOD[*op2]&$F];
        case reg_num:       *op1 = ADDRMOD[Reg(*op2)];
        case hop:           Hop_gen( op3, *op1 );
        case neg:           *op1 = -*op1;
        case complement:    *op1 = ~*op1;
        case set:           *op1 = op3;
        case shift:         (op2=*op2)<0 ? *op1>>=-op2 : (*op1<<=op2);
        case sub:           *op1 -= *op2;
    }
}







































 