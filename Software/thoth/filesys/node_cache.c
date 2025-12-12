
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "disc.h"

/*
 * NODE CACHING
 */

#define MAXNODES 20

char nodbuf[SECTOR_SIZE];
struct Disc_node *dnode = (struct Disc_node *)nodbuf;

struct Mem_node N[MAXNODES];

struct Mem_node *
Get_node(int block)
{
    struct Mem_node *mnode;
    int i, free;
    
    // find the node in the node cache
    free = -1;
    for( i = 0; i < MAXNODES; ++i ) {
        mnode = &N[i];
        if( mnode->refcount == 0 ) free = i;
        if( mnode->self == block ) {
            mnode->refcount++;
            return( mnode );
        }
    }
    if( free < 0 ) {
        printf("Node cache full\n");
        exit(1);
    }
    mnode = &N[free];

    // load the cache from disk
    Disc_read(block, nodbuf);
    strncpy( mnode->Name, dnode->Name, 32);
    mnode->Size_blk = SWAP(dnode->Size_blk);
    mnode->Size_idx = SWAP(dnode->Size_idx);
    mnode->father   = SWAP(dnode->father);
    mnode->son      = SWAP(dnode->son);
    mnode->brother  = SWAP(dnode->brother);
    mnode->self     = block;
    mnode->flags    = 0;
    mnode->refcount = 1;
    return( mnode );
}

int
Put_node( struct Mem_node *mnode, int op )
{
    if( mnode->refcount == 0 ) return( ERROR );
    if( mnode->flags & WRITTEN_ON || op == FLUSH ) {
        Disc_read(mnode->self, nodbuf);
        strncpy( dnode->Name, mnode->Name, 32);
        dnode->Size_blk = SWAP(mnode->Size_blk);
        dnode->Size_idx = SWAP(mnode->Size_idx); 
        dnode->father   = SWAP(mnode->father);
        dnode->son      = SWAP(mnode->son);
        dnode->brother  = SWAP(mnode->brother);
        Disc_write(mnode->self, nodbuf);
        mnode->flags &= ~WRITTEN_ON;
    }
    if( op == CLOSE ) mnode->refcount--;
    return( OK );
}
