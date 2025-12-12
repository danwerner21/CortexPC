
% thoth.hdr

Main()
{
    .Thoth_init();
}

.Thoth_init()
{
    extrn .Active, .Readyqhead, .Readyqtail, .Readyqnose;
    extrn .Hangman, .Hangmanid, .Pid_base, .Idmask, .Newid;
    extrn .Max_process, .Max_user_priority, .Fred;
    extrn .Ehstack., ..End..;
    auto i, max;
    
    .Init_core( &..End.., 8192 );
    
    .Machine_init();
    
\ create the ready queues
    .Readyqhead = .Alloc_vec(max = .Max_user_priority + 3);
    .Readyqtail = .Alloc_vec(max);
    .Readyqnose = .Readyqhead - 1;
    for (i=0; i<=max; ++i)
        .Readyqtail[i] = &.Readyqhead[i];
    \ .Readyqheads are already zero

\ set up the process id space
    --.Max_process;
\ find out how big it is
    for ( i=1; i<.Max_process; i <<= 1 );
    .Max_process = i;
    .Pid_base = .Alloc_vec(.Max_process);
    
\ set the first entry to non-zero so id=0 will not confirm
    .Pid_base[0] = &.Max_process[-ID];
    .Idmask = .Max_process - 1;
    .Newid = 0;
    
\ create the process that destroys other processes
    .Ready( .Hangmanid = .Create( .Hangman, 3, 100 ) );
    
\ create and start the user process
    .Ready( .Create( .Fred, 4, .Ehstack.) );
    
    .Active = 0;
    .Dispatch();            \ never returns
}
