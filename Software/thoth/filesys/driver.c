
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "disc.h"

/*
 * DISK OPERATIONS
 */

int dsk;
char buf[SECTOR_SIZE];

void
Disc_init( void )
{
    if( (dsk = open("thoth.dsk", O_RDWR)) < 0 ) {
        printf("could not open disk image\n");
        exit(1);
    }
}

int
Disc_read(int block, char* buffer)
{
    lseek(dsk, block * SECTOR_SIZE, SEEK_SET);
    return (read(dsk, buffer, SECTOR_SIZE));
}

int
Disc_write(int block, char* buffer)
{
    lseek(dsk, block * SECTOR_SIZE, SEEK_SET);
    return (write(dsk, buffer, SECTOR_SIZE));
}
