
% thoth.hdr
% device.hdr
% ti_opc.hdr

\ The Device Table

.Device_table[24]:

"/dev/tty0",    .Get_data_c,    .Put_data_c,    .No_dev,    .No_dev,
                .No_dev,        0,              0,          .No_dev,
                0,              .Null_dev,      .Init_tty,

"/dev/clock",   .No_dev,        .No_dev,        .No_dev,    .No_dev,
                .No_dev,        0,              0,          .No_dev,
                0,              .No_dev,        .Init_clock,

0,              .No_dev,        .No_dev,        .No_dev,    .No_dev,
                .No_dev,        0,              0,          .No_dev,
                0,              .No_dev,        .No_dev;

\
\ support routines for device drivers and the device table
\ 

.No_dev()   { return( ERROR ); }

.Null_dev() { return( OK );    }

\#PR Get_data_c and Put_data_c are not in the Melen thesis, but appear
\#PR to have been generic routines to handle terminal I/O, such as echo
\#PR and line ending conversion.

.Get_data_c( fcb )
{
    auto dev_ptr, c;
    dev_ptr = .DEV_TAB_PTR[fcb];
    c = .Receive( .DEV_PROCESS_1[dev_ptr] );
    return( c );
}

.Put_data_c( fcb, datum )
{
    auto dev_ptr;
    dev_ptr = .DEV_TAB_PTR[fcb];
    .Send( .DEV_PROCESS_2[dev_ptr], datum );
    return( OK );
}

.Intid[DISK];

.Await_interrupt(dev_code)

\ wait for an interrupt

{
    extrn .Intid, .Active;
    disable;
    .Remove_ready(.Active);
    STATUS[.Active] = REC_BLKED;
    BLOCKED_ON[.Active] = -1;
    .Intid[dev_code] = .Active;
    dev_code = (&SAVE_WP[.Active])<<1;
    twit( .MOV., "ria10", "r13" );
    twit( .MOV., "ria10", "r14" );
    twit( .MOV., "ria10", "r15" );
    .Active = 0;
    enable;
    .Dispatch();
}

.Machine_init()
{
    extrn .Int_level_3, .Int_level_4;
    auto mem;
    
    \ set up int vectors
    mem = 0;
    mem[6] = $f080;
    mem[7] = .Int_level_3 << 1;
    mem[8] = $f100;
    mem[9] = .Int_level_4 << 1;
}


