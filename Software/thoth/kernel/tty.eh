
% thoth.hdr
% device.hdr
% ti_opc.hdr

#.RTS.      = $10
#.RRQ.      = $10
#.WRQ.      = $11
#.RIEN.     = $12
#.WIEN.     = $13
#.INT.      = $1f

#TTY0_BASE  = 0
#TTY1_BASE  = $80

\ special option
#.733       = 0574

.Init_tty( pathname, dev_ptr, mode )
{
    extrn .Active, .Ttodriver, .Ttidriver;
    auto fcb, tty_ind, i;
    
    tty_ind = pathname{8} - '0';
    if (.DEV_PROCESS_2[dev_ptr] == 0)       \ driver not created
    {
        .DEV_PROCESS_2[dev_ptr] = .Ready(
            .Create( .Ttodriver, 1, 100), TTY_OUT+tty_ind);
    }
    
    if ( mode & .INPUT. )
        if (.DEV_PROCESS_1[dev_ptr] == 0 )  \ driver not created
        {
            .DEV_PROCESS_1[dev_ptr] = .Ready(
                .Create( .Ttidriver, 1, 100), TTY_IN+tty_ind);
        }
    
    fcb = .Zero(.Alloc_vec( .MIN_FCB_SIZE ), .MIN_FCB_SIZE );
    .Copy( fcb, dev_ptr, .SHARED_PART );
    .ACCESS_MODE[fcb] = mode;
    .DEV_TAB_PTR[fcb] = dev_ptr;
    .OPTIONS_WORD[fcb] = .ECHO | .CRLF;
    if ( tty_ind == 0 )
        .OPTIONS_WORD[fcb] |= .733;
    return( fcb );
}

.Ttodriver(dev_code)
{
    auto char,base;
    select( dev_code ) {
        case TTY0_OUT:  base = TTY0_BASE;
        case TTY1_OUT:  base = TTY1_BASE;
    }
    twit(.MOV.,"r12",base);
    twit(.SBO.,.INT.);  \ reset 9902
    twit(.SBO.,.RTS.);  \ enable xmit
    twit(.SBZ.,.WIEN.); \ disable xint
    repeat
    {
        char = .Receive(.Wait_sender());
        twit(.SWPB.,char);
        twit(.MOV.,"r12",base);
        twit(.LDCR.,8,char);
        twit(.SBO.,.WIEN.); \ enable xint
        .Await_interrupt(dev_code);
    }
}

.Ttidriver(dev_code)
{
    auto char,base,pid;
    select( dev_code ) {
        case TTY0_IN:  base = TTY0_BASE;
        case TTY1_IN:  base = TTY1_BASE;
    }
    twit(.MOV.,"r12",base);
    twit(.SBZ.,.RIEN.);  \ disable rint
    repeat
    {
        pid = .Wait_receiver();
        twit(.MOV.,"r12",base);
        twit(.SBO.,.RIEN.);  \ enable rint
        .Await_interrupt(dev_code);
        twit(.MOV.,"r12",base);
        twit(.STCR.,8,char);
        twit(.SWPB.,char);
        .Send(pid, char & $7f);
    }
}

.Int_level_4()
{
    extrn .Active, .Readyqhead, .Readyqtail, .Intid;
    auto p;         \ must be first auto
    
    disable;
    if( .Active )
    {   p = (&SAVE_WP[.Active])<<1;
        twit( .MOV., "ria10", "r13" );
        twit( .MOV., "ria10", "r14" );
        twit( .MOV., "ria10", "r15" );
    }
    twit( .LI., "r12", TTY0_BASE );
    twit( .TB., .INT. );    \ any int?
    twit( .JEQ., ttyo );
    return;

ttyo:
    twit( .TB., .WRQ. );
    twit( .JEQ., ttwrite );
    twit( .TB., .RRQ. );
    twit( .JEQ., ttread );
    putchar('Z');
    return;
    
ttwrite:
    twit( .LI., "r12", TTY0_BASE );
    twit( .SBZ., .WIEN. );
    if( .Active = .Intid[TTY0_OUT] )
    {   STATUS[.Active] = READY;
        if( !(LINK[.Active] = .Readyqhead[p=PRIORITY[.Active]]) )
            .Readyqtail[p] = .Active;
        .Readyqhead[p] = .Active;
        .Intid[TTY0_OUT] = 0;
        p = (&SAVE_WP[.Active])<<1;
        twit( .MOV., "r13", "ria10" );
        twit( .MOV., "r14", "ria10" );
        twit( .MOV., "r15", "ria10" );
        return;
    }
    .Dispatch();
    
ttread:
    twit( .LI., "r12", TTY0_BASE );
    twit( .SBZ., .RIEN. );
    if( .Active = .Intid[TTY0_IN] )
    {   STATUS[.Active] = READY;
        if( !(LINK[.Active] = .Readyqhead[p=PRIORITY[.Active]]) )
            .Readyqtail[p] = .Active;
        .Readyqhead[p] = .Active;
        .Intid[TTY0_IN] = 0;
        p = (&SAVE_WP[.Active])<<1;
        twit( .MOV., "r13", "ria10" );
        twit( .MOV., "r14", "ria10" );
        twit( .MOV., "r15", "ria10" );
        return;
    }
    .Dispatch();
}

