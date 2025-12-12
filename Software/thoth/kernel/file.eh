
% thoth.hdr
% device.hdr

\#PR These functions are not in the Melen thesis and hence re-creations
\#PR based on surrounding code and descriptions in various sources.
\#PR Note that these are the 1976 versions, and different from the later
\#PR versions as described in e.g. Cheriton.

.Open( pathname; modestr )
{
    extrn .Device_table, .Active;
    auto dev_ptr, p, mode, c, fcb;
    dev_ptr = .Device_table;
    while ( (p=.PATHNAME[dev_ptr]) && !.Equal(p, pathname) )
        dev_ptr += .DEV_TAB_WIDTH;
    if (!p) return( ERROR );
    mode = p = 0;
    if( .Nargs() > 1 )
    {
        while ( c=modestr{p++} ) {
            if( c=='r' ) { mode |= .INPUT.; next; }
            if( c=='w' ) { mode |= .OUTPUT.; next; }
        }
    }
    fcb = (*.INIT_FCB_RTN[dev_ptr])( pathname,dev_ptr,mode );
    disable;
    .FCB_LINK[fcb] = .FCB_LINK[.Active];    \ link in FCB chain
    .FCB_LINK[.Active] = fcb;
    enable;
    ++.REF_COUNT[dev_ptr];
    return( fcb );
}

.Close( fcb )
{
    extrn .Active;
    auto dev_ptr, p;
    if (*fcb != *.DEV_TAB_PTR[fcb]) \ check FCB pointer
        return( ERROR );
    p = .Active;
    disable;
    while( p && .FCB_LINK[p] != fcb ) p = .FCB_LINK[p];
    if (p == 0) return( ERROR );
    .FCB_LINK[p] = .FCB_LINK[fcb];  \ unlink
    dev_ptr = .DEV_TAB_PTR[fcb];
    (*.TERMINATE_RTN[dev_ptr])( fcb );
    enable;
    .Free( fcb );
    --.REF_COUNT[dev_ptr];
}

.Get( fcb )
{
    auto datum;
    if (*fcb != *.DEV_TAB_PTR[fcb]) \ check FCB pointer
        return( ERROR );
    if (.ACCESS_MODE[fcb] & .INPUT. )
    {
        datum = (*.GET_DATA_RTN[fcb])( fcb );
        return( datum );
    }
    return( ERROR );
}

.Put( fcb, datum )
{
    if (*fcb != *.DEV_TAB_PTR[fcb]) \ check FCB pointer
        return( ERROR );
    if (.ACCESS_MODE[fcb] & .OUTPUT. )
    {
        datum = (*.PUT_DATA_RTN[fcb])( fcb, datum );
        return( datum );
    }
    return( ERROR );
}
