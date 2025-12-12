
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "disc.h"

struct FCB *fcb = &FCB;
    
/*
 * FILE OPERATIONS
 */

void
Update_nodesize( struct FCB *fcb )
{
    struct Mem_node *node = fcb->node;

    if( fcb->pos_blk > node->Size_blk )
    {
        node->Size_blk = fcb->pos_blk;
        node->Size_idx = fcb->index;
    }
    else if( fcb->pos_blk == node->Size_blk &&
               fcb->index > node->Size_idx )
    {
        node->Size_idx = fcb->index;
    }
    
    node->flags = WRITTEN_ON;
}

void
File_next( struct FCB *fcb, int alloc )
{
    int nxt_blk;

    nxt_blk = Next_block( fcb->disc_blk, alloc );
    if( nxt_blk < 0 ) return;
    if( fcb->flags & WRITTEN_ON ) {
        Disc_write( fcb->disc_blk, fcb->buffer );
        fcb->flags &= ~WRITTEN_ON;
    }
    Disc_read( nxt_blk, fcb->buffer );
    fcb->pos_blk++;
    fcb->index = 0;
    Update_nodesize( fcb );
    fcb->disc_blk = nxt_blk;
}

// seek 'how' methods
#define ABS_BYTE 1
#define REL_BYTE 2
#define ABS_BLK  3

void
Seek( struct FCB *fcb, int offset, int how )
{
    int i;

    Update_nodesize( fcb ); // in case offset < 0
    if( fcb->flags & WRITTEN_ON ) {
        Disc_write( fcb->disc_blk, fcb->buffer );
        fcb->flags &= ~WRITTEN_ON;
    }
    switch( how ) {
    case ABS_BYTE: fcb->index  = offset + BASE; fcb->pos_blk = 0; break;
    case REL_BYTE: fcb->index += offset; break;
    case ABS_BLK:  fcb->index  = BASE; fcb->pos_blk = offset; break;
    }
    fcb->pos_blk += fcb->index >> SECTSZ_LOG2;
    fcb->index &= 0x1ff;
    if( fcb->pos_blk < 0 ) fcb->pos_blk = 0;
    if( fcb->pos_blk == 0 && fcb->index < BASE ) fcb->index = BASE;
    fcb->disc_blk = fcb->node->self; 
    i = fcb->pos_blk;
    while( i-- )
        fcb->disc_blk = Next_block( fcb->disc_blk, 1 );
    Update_nodesize( fcb ); // in case offset > 0
}

int
Where( struct FCB *fcb )
{
    return( ((fcb->pos_blk << SECTSZ_LOG2) | fcb->index) - BASE );
}

int
Get( void )
{
    if( fcb->index >= SECTOR_SIZE )
        File_next( fcb, 0 );
    return( (fcb->buffer[fcb->index++] & 0xff) );
}

void
Put(char byte)
{
    if( fcb->index >= SECTOR_SIZE )
        File_next( fcb, 1 );
    fcb->buffer[fcb->index++] = byte;
    fcb->flags |= WRITTEN_ON;
}

struct FCB *
Open( char *path, char *mode )
{
    int i;

    fcb->node = Seek_node( path, NULL );
    if( !fcb->node ) return( ERROR );
    fcb->disc_blk = fcb->node->self;
    if( strchr(mode, 'a') ) {
        fcb->pos_blk  = fcb->node->Size_blk;
        fcb->index    = fcb->node->Size_idx;
        i = fcb->pos_blk;
        while( i-- )
            fcb->disc_blk = Next_block( fcb->disc_blk, 0 );
    } else {
        fcb->pos_blk  = 0;
        fcb->index    = BASE;
    }
    fcb->flags    = 0;
    fcb->buffer   = buf;
    Disc_read(fcb->disc_blk, fcb->buffer);
    return( fcb );
}

void
Close(struct FCB *fcb)
{
    if( fcb->flags & WRITTEN_ON )
        Disc_write(fcb->disc_blk, fcb->buffer);
    Update_nodesize( fcb );
    fcb->flags = 0;
    Put_node( fcb->node, CLOSE );
    Get_map( FLUSH );
}
