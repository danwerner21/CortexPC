
\ externals
.Active;            \ PD pointer of active process
.Hangmanid;         \ process id of hangman

\ ready queues - links all ready processes
.Readyqhead;
.Readyqtail;
.Readyqnose;

.Rover;             \ roving pointer to available memory
.Mark;
.Memsize;           \ initial amount of available memory

.Max_user_priority: 4;  \ default number of prio levels
.Max_process: 128;      \ default number of processes

\ externals used for confirming a process id
\ .Max_process is the size of the PID space
.Pid_base;          \ pointer to start of PID space
.Idmask;            \ mask using for hashing to pidspace
.Newid;             \ process id of last created process

\ clock externals
.Clicks;
.Tenths;
.Minutes;
.Hours;
.Wake_tenths;
.Wake_minutes;
.Wake_hours;
.Rtc_process;
.Tocid;

\ the default stack size
.Ehstack.: 200;
