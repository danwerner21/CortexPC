
% thoth.hdr
% ti_opc.hdr

.Set_stack(start,process)

\ set up values in process's stack
\ this is of course machine specific

{
    extrn .Suicide;
    auto i,stkptr;
    SAVE_PC[process] = start << 1;      \ set up start address
    select(PRIORITY[process])
    {
        case 0: i = 0;
        case 1: i = 5;
        case 2: i = 10;
        default: i = 15;
    }
    SAVE_ST[process] = i;       \ get status based upon priority
    STACK_PTR[process] = (STACK_START[process] - 20 
                         + STACK_START[process][-1]);
    stkptr = STACK_PTR[process];
    stkptr[15] = SAVE_ST[process];
    stkptr[14] = (.Suicide) << 1;
    stkptr[13] = stkptr << 1;
    \printf("wp=%d,pc=%d,st=%d*n", stkptr[13], start<<1, stkptr[15]);
}

.Create( fctn, prio, stksize )

\ create a new process

{
    extrn .Active, .Newid, .Pid_base, .Idmask;
    extrn .Max_user_priority;
    auto newprocess, offset;
    
\ check the priority level
    if (prio >= .Max_user_priority + 3) return( ERROR );
    
    newprocess = .Alloc_vec(PD_SIZE);
    STACK_START[newprocess] = .Alloc_vec(stksize);
    
\ initialize entries in the block queues
    RECQ_HEAD[newprocess] = &RECQ_EMPTY[newprocess];
    RECQ_TAIL[newprocess] = &(RECQ_HEAD - BLOCK_BACK)[newprocess];
    SENDQ_HEAD[newprocess] = & SENDQ_EMPTY[newprocess];
    SENDQ_TAIL[newprocess] = &(SENDQ_HEAD - BLOCK_BACK)[newprocess];
    
    STATUS[newprocess] = EMBRYO;
    PRIORITY[newprocess] = prio;
    FCB_LINK[newprocess] = 0;
    
\ call .Set_stack to place certain values in the stack
    .Set_stack( fctn, newprocess );
    
    if (DEBUG) START_ADDR[newprocess] = fctn;

\ allocate process id pointer
    repeat
    {
        enable;
        disable;
        if (++.Newid == 0) .Newid = 1;  \ have wrapped around
        if (.Pid_base[offset = .Newid&.Idmask]) next; \ in use
        .Pid_base[offset] = newprocess;
        ID[newprocess] = .Newid;
        enable;
        break;
    }
    return( ID[newprocess] );
}

.Hangman()

\ the process which executes this function will destroy
\ a process and release its PD and stack

{
    extrn .Pid_base, .Idmask;
    auto pdptr, p, sender;
    repeat
    {
        sender = .Wait_sender();
        
        pdptr = .Pid_base[sender & .Idmask];
        .Pid_base[sender & .Idmask] = 0;
        
\ release the stack
        .Free(STACK_START[pdptr]);
        
\ remove process from this block queue
        disable;
        BLOCK_BACK[BLOCK_FWD[pdptr]] = BLOCK_BACK[pdptr];
        BLOCK_FWD[BLOCK_BACK[pdptr]] = BLOCK_FWD[pdptr];
        
\ release the process descriptor
        .Free(pdptr);
    }
}

.Remove_ready(pdptr)

\ remove a PD from the FRONT of a ready queue
\ this is called with interrupts disabled so that the queues
\ are not screweed by intervening interrupts

{
    extrn .Readyqhead, .Readyqtail;
    auto p;
    if (!(.Readyqhead[p = PRIORITY[pdptr]] = LINK[pdptr]))
        .Readyqtail[p] = &.Readyqhead[p];
}

