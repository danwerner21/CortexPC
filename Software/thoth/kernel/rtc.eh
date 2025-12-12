
% thoth.hdr
% device.hdr
% ti_opc.hdr

#CLICKSPERTENTH = 5

.Init_clock( pathname, dev_ptr, mode )
{
    extrn .Tic,.Toc,.Rtc_process,.Tocid;
    auto fcb;
    if (.DEV_PROCESS_1[dev_ptr] == 0)       \ .Tic/.Toc not yet created
    {
        .Tocid = .DEV_PROCESS_1[dev_ptr] = .Ready(
            .Create( .Toc, 3, 200) );
        .Rtc_process = .DEV_PROCESS_2[dev_ptr] = .Ready(
            .Create( .Tic, 3, 200), .Tocid );
    }
    fcb = .Zero(.Alloc_vec( .MIN_FCB_SIZE ), .MIN_FCB_SIZE );
    .Copy( fcb, dev_ptr, .SHARED_PART );
    .ACCESS_MODE[fcb] = mode;
    .DEV_TAB_PTR[fcb] = dev_ptr;
    .OPTIONS_WORD[fcb] = 0;
    twit( .LI., "r12", $1ee0 );
    twit( .SBO., 1 );
    return( fcb );
}

.Tic(toc)
{
    extrn .Active;
    extrn .Tenths, .Minutes, .Hours, .Clicks;
    extrn .Wake_tenths,.Wake_minutes,.Wake_hours;
    auto difftenths,diffminutes,diffhours;

    repeat
    {
        disable;
        .Await_interrupt( RTC );
        disable;
        if( ++.Clicks > CLICKSPERTENTH )
        {
            .Clicks = 0;
            if (++.Tenths >= 600 ) {
                .Tenths = 0;
                if (++.Minutes >= 60 )
                {
                    .Minutes = 0;
                    ++.Hours;
                }
            }
            diffhours = .Hours - .Wake_hours;
            diffminutes = .Minutes - .Wake_minutes;
            difftenths = .Tenths - .Wake_tenths;

            enable;
            if  ((diffhours > 0) ||
                ((diffhours == 0) && ((diffminutes > 0 ) ||
                ((diffminutes == 0) && (difftenths > 0)))))
                .Send(toc, 0);
        }
    }
}

.Int_level_3()
{
    extrn  .Active, .Readyqhead, .Readyqtail, .Intid;
    auto  p;        \ must be first auto
    
    disable;
    if( .Active )
    {   p = (&SAVE_WP[.Active])<<1;
        twit( .MOV., "ria10", "r13" );
        twit( .MOV., "ria10", "r14" );
        twit( .MOV., "ria10", "r15" );
    }
    if( .Active = .Intid[RTC] )
    {   STATUS[.Active] = READY;
        if( !(LINK[.Active] = .Readyqhead[p=PRIORITY[.Active]]) )
            .Readyqtail[p] = .Active;
        .Readyqhead[p] = .Active;
        .Intid[RTC] = 0;
        p = (&SAVE_WP[.Active])<<1;
        twit( .MOV., "r13", "ria10" );
        twit( .MOV., "r14", "ria10" );
        twit( .MOV., "r15", "ria10" );
        return;
    }
    .Dispatch();
}
