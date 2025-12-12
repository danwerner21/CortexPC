
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "disc.h"

/*
 * DISC MAP OPERATIONS
 */
 
short mapbuf[SECTOR_WORDS];
int   mapsec;

void
Get_map(int sec)
{
    if( sec == mapsec ) return;
    if( mapsec ) Disc_write(mapsec, (void*)mapbuf);
    if( sec != FLUSH ) {
        Disc_read(sec, (void*)mapbuf);
        mapsec = sec;
    }
}

int
Alloc_block( void )
{
    int sec, ofs;
    
    sec = 2;
    while( sec < Geom.first ) {
        ofs = 0;
        Get_map(sec);
        while( ofs < SECTOR_WORDS && mapbuf[ofs] ) ++ofs;
        if( ofs < SECTOR_WORDS ) {
            mapbuf[ofs] = -1;
            Disc_write(sec, (void*)mapbuf);
            return( (sec-2)*SECTOR_WORDS + ofs + Geom.first );
        }
        ++sec;
    }
    return( ERROR );
}

int
Next_block(int block, int alloc)
{
    int sec, ofs, new;
    
    if( block < 0 ) return( -1  );
    block -= Geom.first;
    sec = (block >> 8) + 2;
    Get_map(sec);
    ofs = (block & 0xff);
    if( mapbuf[ofs] < 0 && alloc ) {
        new = Alloc_block();
        if( new == ERROR ) return( ERROR );
        Get_map(sec);
        mapbuf[ofs] = SWAP(new);
    }
    return( SWAP(mapbuf[ofs]) );
}

void
Free_chain(int block)
{
    int sec, ofs;

    while( block != -1 ) {
        block -= Geom.first;
        sec = (block >> 8) + 2;
        Get_map(sec);
        ofs = (block & 0xff);
        block = mapbuf[ofs];
        mapbuf[ofs] = 0;
    }
    Get_map( FLUSH );
}
