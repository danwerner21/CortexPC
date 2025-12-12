\
\ startup code, inits stack and calls Main
\ always first module to load
\
            .rel    2
            .ent    ..start
            .ext    Main
..start:    stwp    r11             \ loader sets wp to himem
            ai      r11,-34         \ build frame
            mov     @Main,r12       \ jump to Main
            sla     r12,1
            blwp    r11
            idle
            .end

\
\ .Nargs function. Get the argument count from the
\ caller's frame and return that (in r12)
\
            .rel    2
            .ent    .Nargs
            .formal 0,0
            .function
            .nargs
start:      mov	    @32[r13], r12
            rtwp
            .rel    1
.Nargs:     .ptr    start
            .end

\
\ .Args function. Get the nth argument. First argumenta are
\ in r10-r4, continues in r0-2, r0-4 etc.
\
            .rel    2
            .ent    .Arg
            .formal 1,1
            .function
start:      sla     r10, 1   
            li      r9, 22    
            s       r10, r9    
            ci      r10, 16   
            jlt     L1       
            ai      r9, -8    
L1:         mov     r13, r0  
            a       r9, r0     
            mov     [r0], r12         
            rtwp
            .rel    1
.Arg:       .ptr    start
            .end

\
\ ..selstr routine. Enter by a branch. Works with the jump
\ table pointed to by r3 to find the matching string and
\ jumps to vector. Also see "select.c"
\
            .rel    2
            .ent    ..selstr
..selstr:   sla     r12, 1
L0:         mov     [r3]+, r2
            mov     [r3]+, r0
            jeq     L1
            mov     r12,r1
L2:         cb      [r0], [r1]+
            jne     L0
            cb      [r0]+, @zero
            jne     L2
L1:         b       [r2]
zero:       .dc1    0
            .end
            
\
\ ..End.. is located at the end of the binary.
\
            .rel    7
            .ent    ..End..
..End..:    .dc1    ..End..
            .end

\
\ Output one char to the tty on the Mini Cortex. Not part
\ of the compiler, but good for debugging
\
            .rel    2
            .ent    putchar
            .formal 1,1
start:      clr     r12
            swpb    r10
            ldcr    r10, 8
            rtwp
            .rel    1
putchar:    .ptr    start
            .end
