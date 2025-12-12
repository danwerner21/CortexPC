
% thoth.hdr

#.MAXINTEGER = 32767

.Set_time(tenths,minutes,hours)

\ function used to set time of day

{
    extrn .Tenths,.Minutes,.Hours,.Clicks;
    
\ normalise inputs
    hours += (minutes += tenths / 600) / 60;
    minutes = minutes % 60;
    tenths = tenths % 600;
    
    disable;
    .Clicks = 0;
    .Tenths = tenths;
    .Minutes = minutes;
    .Hours = hours;
    enable;
}

.Get_time(tenths,minutes,hours;clicks)

\ function to get time of day

{
    extrn .Tenths,.Minutes,.Hours,.Clicks;
    disable;
    *tenths = .Tenths;
    *minutes = .Minutes;
    *hours = .Hours;
    if (.Nargs() > 3) *clicks = .Clicks;
    enable;
}

.Delay(tenths;minutes,hours)

\ function to sleep for a set period of time

{
    auto a,b,c;
    if (.Nargs() < 3) hours = 0;
    if (.Nargs() < 2) minutes = 0;
    .Get_time(&a,&b,&c);
    .Sleep(a + tenths, b + minutes, c + hours);
}

.Sleep(tenths,minutes,hours)

\ function to sleep until correct time

{
    extrn .Tocid;
    auto t[2];
\ normalise input;
    t[2] = hours + (minutes += tenths / 600) / 60;
    t[1] = minutes % 60;
    t[0] = tenths % 600;
    .Send(.Tocid,t);
}

.Toc()

\ process that is woken by .Tic when it is time to wakeup a process
\ it also runs the wakeup queue which is the same as its send-queue

{
    extrn .Active,.Wake_tenths,.Wake_minutes,.Wake_hours;
    extrn .Rtc_process,.Tenths,.Minutes,.Hours;
    extrn .Pid_base,.Idmask;
    auto pid,p,t,newtenths,newminutes,newhours;
    auto difftenths,diffminutes,diffhours,time;
    RECQ_HEAD[.Active] = 0;
    .Wake_hours = .MAXINTEGER;
    repeat
    {
        pid = .Wait_sender();
        if (pid == .Rtc_process)
        {           \ then time to wake someone
            .Wake_hours = .MAXINTEGER;
            .Receive(pid);      \ let .Tic go
            
\ while somenone is in queue
            while (p = RECQ_HEAD[.Active])
            {         
                difftenths = MESSAGE[p][0] - .Tenths;
                diffminutes = MESSAGE[p][1] - .Minutes;
                diffhours = MESSAGE[p][2] - .Hours;
                
                if  ((diffhours > 0) ||
                    ((diffhours == 0) && ((diffminutes > 0 ) ||
                    ((diffminutes == 0) && (difftenths > 0)))))
                    break;
                RECQ_HEAD[.Active] = LINK[p];
                disable;
                .Add_ready(p);
                enable;
            }          
        }
        else    \ someone whishes to sleep
        {
\ remove him from the send-queue
            p = .Pid_base[pid & .Idmask];   \ get PD pointer
            disable;
            BLOCK_BACK[BLOCK_FWD[p]] = BLOCK_BACK[p];
            BLOCK_FWD[BLOCK_BACK[p]] = BLOCK_FWD[p];
            enable;
            STATUS[p] = REC_BLKED;

\ get times for this process
            newtenths = MESSAGE[p][0];
            newminutes = MESSAGE[p][1];
            newhours = MESSAGE[p][2];
            
\ place in correct spot in the receive-queue
            pid = &RECQ_HEAD[.Active] - LINK;
            while (t = LINK[pid])
            {
                difftenths = (time = MESSAGE[t])[0];
                           - newtenths;
                diffminutes = time[1] - newminutes;
                diffhours = time[2] - newhours;
                if  ((diffhours > 0) ||
                    ((diffhours == 0) && ((diffminutes > 0 ) ||
                    ((diffminutes == 0) && (difftenths > 0)))))
                    break;
                pid = t;
            }
            LINK[pid] = p;
            LINK[p] = t;
        }
        if (RECQ_HEAD[.Active])
        {
            .Wake_tenths = (t = MESSAGE[RECQ_HEAD[.Active]])[0];
            .Wake_minutes = t[1];
            .Wake_hours = t[2];
        }
    }
}
