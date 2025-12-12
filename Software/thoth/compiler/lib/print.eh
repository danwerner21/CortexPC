

printf( fmt; ? )
{
    auto c, i, j;

    i = 0;
    j = 2;
    repeat {
        while( (c = fmt{i++}) != '%' )
        {
            if( c == 0 )
                return;
            putchar( c );
        }
        print_fmt( fmt{i++}, .Arg( j++ ) );
    }
}

print_fmt( c, arg )
{
    auto i;

    if( c == 'd' ) {
        if( arg < 0 ) {
            putchar( '-' );
            arg = -arg;
        }
    }
    if( c == 'd' || c == 'o' )
        printn( arg, c=='o' ? 8 : 10 );
    else if( c == 'c' )
        putchar( arg );
    else if( c == 's' ) {
        i = 0;
        while( c = arg{i++} )
            putchar( c );
    }
}

printn( n, b )
{
    auto a;

    if( a = n / b )
        printn( a, b );
    putchar( n % b + '0' );
}
