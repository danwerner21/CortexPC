
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "disc.h"

extern struct Disc_node *dnode;
extern char nodbuf[];

/*
 * NODE OPERATIONS
 */

char Name[32];

struct Mem_node *
Seek_sons( struct Mem_node *dir_node )
{
    int node, next;
    struct Mem_node *work_node;
    
    node = dir_node->son;
    while( node ) {
        work_node = Get_node( node );
        if( strncmp(work_node->Name, Name, 32) == 0 ) {
            return( work_node );
        }
        next = work_node->brother;
        Put_node( work_node, CLOSE );
        node = next;
    }
    return( ERROR );
}

// Returns open node if found, or the parent node if
// creatable, or ERROR otherwise. A path is creatable if the
// path can be fully resolved except for the last node.
//
struct Mem_node *
Seek_node( char *path, int *creatable )
{
    struct Mem_node *dir, *file;
    int i;
    char c;

    // set the starting point of the path
    dir = Cur_node; dir->refcount++;
    c = *path;
    if( c == '*' || c == '@' ) {
        if( c == '*' ) { dir = Root_node; ++path; }
        else if( c == '@' ) ++path;
        c = *path++;
        if( c && c != '/' ) return( ERROR );
    }

    // walk the path
    if( creatable ) *creatable = ERROR;
    while( 1 ) {
        while( (c = *path++) == '/' ) ;
        if( !c ) return( dir );
        Name[0] = c;
        for( i = 1; (c = *path++) != '/'; ++i ) {
            if( i < 32 ) Name[i] = c;
            if( !c ) break;
        }
        if( i < 32 ) Name[i] = 0;
        if( (file = Seek_sons( dir )) == ERROR ) {
            if( creatable && !c ) *creatable = OK;
            return( dir );
        }
        Put_node( dir, CLOSE );
        if( !c ) return( file );
        dir = file;
    }
}

int
Remove_node(char *path)
{
    int self, brother, node;
    struct Mem_node *dir_node, *work_node;

    if( !(work_node = Seek_node( path, NULL )) ) return( ERROR );
    self = work_node->self;
    brother = work_node->brother;
    dir_node = Get_node( work_node->father );
    node = dir_node->son;
    if( node == self ) {
        dir_node->son = brother;
        dir_node->flags |= WRITTEN_ON;
    }
    else while ( node ) {
        Put_node( work_node, CLOSE );
        work_node = Get_node( node );
        node = work_node->brother;
        if( node == self ) {
            work_node->brother = brother;
            work_node->flags |= WRITTEN_ON;
            break;
        }
    }
    Put_node( work_node, CLOSE );
    Put_node( dir_node, CLOSE );
    Free_chain( self ); // delay until last node close ?
    return( OK );
}

int
Make_node(char *path) 
{
    int entry, creatable;
    struct Mem_node *father;
    
    father = Seek_node( path, &creatable );
    if( creatable != OK ) return( FAIL );
    if( (entry = Alloc_block()) == ERROR ) return( ERROR );
    memset(nodbuf, 0, SECTOR_SIZE);
    strncpy(dnode->Name, Name, 32);
    dnode->father  = SWAP(father->self);
    dnode->brother = SWAP(father->son);
    father->son = entry;
    father->flags |= WRITTEN_ON;
    Get_map( FLUSH );
    Disc_write(entry, nodbuf);
    Put_node( father, CLOSE );
    return( entry );
}
