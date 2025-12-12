
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "disc.h"

extern struct Disc_node *dnode;
extern char nodbuf[];
extern int dsk;

// disk size in sectors (0x5000 = 20.480)
#define DISK_SIZE   0x5000
#define IMG_NAME    "thoth.dsk"

void
Check_image( void )
{
    int fd, root;

    // create blank image file if needed
    if( (fd = open(IMG_NAME, O_RDWR)) < 0 ) {
        printf("%s image not found, creating\n", IMG_NAME );
        if( (fd = open(IMG_NAME, O_RDWR|O_CREAT|O_TRUNC, 0644)) < 0 ) {
            printf("could not create disk image\n");
            exit(1);
        }
        lseek(fd, DISK_SIZE * SECTOR_SIZE - 1, SEEK_SET);
        write(fd, "", 1);
        
        // init master block
        Disc_init();
        buf[0] = (DISK_SIZE >> 8); buf[1] = (DISK_SIZE & 0xff);
        Disc_write(1, buf); // write master block to sector 1
        
        // create the root node
        Geom.first = (DISK_SIZE >> 8) + 2;
        root = Alloc_block();
        memset(nodbuf, 0, SECTOR_SIZE);
        strcpy(dnode->Name, "*");
        Get_map( FLUSH );
        Disc_write(root, nodbuf);
        close( dsk );
    }
    close( fd );
}

/*
 * TEST CODE
 */

int
main(int argc, char *argv[])
{
    int i;
    struct Mem_node *n;
    struct FCB *fcb;

    Check_image();

    // read master block
    Disc_init();
    Disc_read(1, buf);
    Geom.size = (buf[1] & 0xff) | ((buf[0] & 0xff) << 8);
    Geom.first = (Geom.size >> 8) + 2;

    // set current dir to root node
    Root_node = Get_node( Geom.first );
    Cur_node = Root_node;
    Cur_node->refcount++;

    // Open root node as file
    fcb = Open("*", "w");
    for(i=0; i<513; ++i)
        Put(i);
    Close( fcb );

    i = Make_node("/kernel");
    printf("Make 'kernel' = %d\n", i);
    i = Make_node("/kernel/lib");
    printf("Make 'kernel/lib' = %d\n", i);    
    n = Seek_node("/kernel", NULL);
    printf("Result seek = %d\n", n->self);
    Put_node( n, CLOSE );
    n = Seek_node( argv[1], NULL );
    printf("Result path = %d\n", n->self);
    Put_node( n, CLOSE );
    i = Remove_node( argv[1] );
    printf("Remove = %d\n", i);
    Cur_node = Seek_node( "*/kernel", NULL );
    i = Make_node("/lib");
    printf("Make '@/lib' = %d\n", i);  
    Put_node( Cur_node, CLOSE );
    Cur_node = Root_node;

    fcb = Open("*", "a");
    for(i=0; i<500; ++i)
        Put(i+65);
    Close( fcb );
    return( 0 );
}
