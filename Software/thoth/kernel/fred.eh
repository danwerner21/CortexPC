
.In;
.Out;
.Err;
.Clock;

.Fred()

\ main test user function

{
    extrn .Freya, .Ehstack., .Tenths;
    extrn .In, .Out, .Err, .Clock, .Buffer;
    auto argc, argv[16];
    auto i, id, was;
    
    \ create the clock process
    .Clock = .Open("/dev/clock");
    .Err = .Out = .Open("/dev/tty0", "w");
    .In = .Open("/dev/tty0", "r");

    \.Put(.Out, .Get(.In) ); .Put(.Out, '*n' );
    
    argc = .Parse_comm( argv );
    for (i=0; i<argc; ++i)
    {
        printf("%s ", argv[i]);
    }
    putchar('*n');

    .Ready( id = .Create( .Freya, 4, .Ehstack.) );

    for (i=0; i<10; ++i)
    {
        was = .Tenths; while( was == .Tenths ) ;
        printf("Hi %d! - %d*n", id, .Tenths);
        .Send( id, 2);
        .Receive( id );
        .Dispatch();    \ yield
    }
}

.Freya()

\ Secondary test user function

{
    auto i, id;
    repeat
    {
        id = .Wait_sender();
        printf("Ho %d!*n", id);
        .Receive( id );
        .Send( id, 3);
        .Dispatch();    \ yield
    }
}

.Buffer[128];

.Parse_comm( argv )
{
    extrn .Buffer, .In, .Out;
    auto i, c, argc, spaces;
    argc = i = 0;
    spaces = 1;
    argv[0] = .Buffer;
    .Put( .Out, '>' ); .Put( .Out, ' ' );
    repeat
    {
        c = .Get( .In );
        if (c==8) \ BS
        {
            if (i>0)
            {
                .Put(.Out, 8); .Put(.Out, ' '); .Put(.Out, 8);
                --i;
            }
            next;
        }
         .Put( .Out, c );
        if (c==13)  \ CR
        {
            .Buffer{i} = 0;
            .Put( .Out, '*n' );
            return( ++argc );
        }
        if (c==' ') \ SPACE
        {
            if (spaces) next;
            ++spaces;
            .Buffer{i++} = 0;
            if (i & 1) .Buffer{i++} = 0;
            ++argc;
            argv[argc] = .Buffer + (i>>1);
            next;
        }
        if(c==0||i>250) next;
        .Buffer{i++} = c;
        spaces = 0;
    }
}

\ library functions

.Zero(addr,len)
{
    while( len ) addr[len--] = 0;
    addr[0] = 0;
    return( addr );
}

.Copy( dst, src, len )
{
    while( len ) dst[len--] = src[len];
    dst[0] = src[0];
}

.Equal(s1, s2)
{
    auto i, c;
    i = 0;
    while( (c = s1{i}) != 0 && c == s2{i} ) ++i;
    return( c ==  s2{i} );
}

