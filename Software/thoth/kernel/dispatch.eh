
#.START.ABSLDR.AREA. = $00c0

% thoth.hdr
% ti_opc.hdr

.Dispatch()

\
\ dispatcher for the TI-990
\ in general, this runs enabled
\ the dispatcher does not resume execution if interrupted
\ it restarts from the beginning
\

{
    extrn .Active, .Readyqnose, .Max_user_priority;
    auto ptr;       \ this MUST be first auto
    auto i, readyhead;
    disable;
    if (.Active != 0)   \ else were interrupted in dispatcher
    {
        ptr = (&SAVE_WP[.Active]) << 1;
        twit(.MOV.,"ria10","r13");  \ get wp
        twit(.MOV.,"ria10","r14");  \ get pc
        twit(.MOV.,"ria10","r15");  \ get st
        .Active = 0;
    }
    twit(.LWPI.,.START.ABSLDR.AREA. -  20 );
    twit(.STWP.,"r11");
    twit(.AI.,"r11",-40);
    repeat      \ find someone to dispatch
    {
        readyhead = .Readyqnose;
        if ( *++readyhead || *++readyhead || *++readyhead ||
             *++readyhead || *++readyhead )
        {
            .Active = *readyhead;
            ptr = (&SAVE_WP[.Active]) << 1;
            twit(.MOV.,"r13","ria10");
            twit(.MOV.,"r14","ria10");
            twit(.MOV.,"r15","ria10");
            twit(.RTWP.);
        }
        twit(.LIMI., 15 );      \ allow any interrupt
        i = .Max_user_priority - 2; \#PR first two levels covered in above 5
        while ( --i )
        {
            if ( *++readyhead )
            {
                disable;
                .Active = *readyhead;
                ptr = (&SAVE_WP[.Active]) << 1;
                twit(.MOV.,"r13","ria10");
                twit(.MOV.,"r14","ria10");
                twit(.MOV.,"r15","ria10");
                twit(.RTWP.);
            }
        }
        twit(.IDLE.);
    }
}

.Add_ready( pdptr )

\ add a PD to the end of the ready queue
\ this is called with interrupts disabled so that the queues
\ are not screwed up by intervening interrupts

{
    extrn .Readyqtail;
    auto p;
    .Readyqtail[p] = LINK[.Readyqtail[p = PRIORITY[pdptr]]]
                   = pdptr;
    LINK[pdptr] = 0;
}

.Ready( ? )

\ Change the process state from embryo to ready and pass arguments

{
    extrn .Pid_base, .Idmask;
    auto i, nargs, pdptr, pid;
    disable;
    if ((ID[pdptr = .Pid_base[(pid = .Arg(1)) & .Idmask]] != pid )
       || (STATUS[pdptr] != EMBRYO))
    {
        enable;
        return(ERROR);
    }
    *(STACK_PTR[pdptr]+16) = ( nargs = .Nargs() ) - 1;
    for (i=2;i<=nargs;++i)
    {
        enable;
        disable;
        *(STACK_PTR[pdptr] + 12 - i) = .Arg(i);
    }
    STACK_PTR[pdptr] <<= 1;
    .Add_ready( pdptr );
    enable;
    return( pid );
}

.Suicide()

\ destroy the active process
\ this routine is needed as it is called when a process
\ returns

{
    extrn .Active, .Hangmanid;
    auto p;
    
\ close all files
    while (p = FCB_LINK[.Active]) .Close( p );
    
\ send .Hangman any value
    .Send( .Hangmanid, 0 );
}

.Condemn( process )

\ condemn a process to death
\ if condemning the active process
\ call suicide else modify PD so
\ suicide is called

{
    extrn .Active, .Pid_base, .Idmask;
    if (ID[.Pid_base[process&.Idmask]] != process) return(ERROR);
    if (process == ID[.Active]) .Suicide();
    STACK_PTR[process] = (STACK_START[process] - 20 
                         + STACK_START[process][-1]) << 1;
    return( OK );
}

.Panic()
{
    twit(.LIMI., 0);
    twit(.IDLE.);
}
