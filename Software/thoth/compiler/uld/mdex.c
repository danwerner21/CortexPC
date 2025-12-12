
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "uld.h"

int OFile;

void
Gen_listing( void )
{
    int i;

    printf("Linked image:\n");

    for( i=0x100; i < Rbr[3]; i += 2 )
    {
        char buf[40];
        int c;

        /* print data */
        printf("%04x %04x ", i, (Memory[i]<<8)+Memory[i+1] );
        
        /* if code section, disassemble */
        if( i >=  Rbr[1] && i < Rbr[2] ) {
            i = dasm_one(buf, i ) - 2;
            printf("%s\n", buf);
        }
        /* if string section, include text */
        else if( i >=  Rbr[2] && i < Rbr[3] ) {
            printf("\"");
            while( (c = Memory[i]) ) {
                if( c == '\n' )
                    printf("*n");
                else
                    printf("%c", c);
                i++;
            }
            printf("\"\n");
            if( (i & 1) ) i--;
        }
        else
            printf("\n");
    }
}

void
Gen_MDEX( void )
{
    struct Symbol *sym;
    int i, entry;

    /* Lookup entry point */
    Module = Global_lookup( (unsigned char *)"..start", FALSE );
    if( Module ) {
        sym = Local_lookup( (unsigned char *)"..code" );
        entry = sym ? sym->value : 0;
    }
    
    /* Build MDEX / NOS executable header at address 0x80 */
    Memory[128] = 255;    Memory[129] = 255;      /* magic FFFF  */
    Memory[134] = 1;      Memory[135] = 0;        /* load  0100  */
    i = Rbr[3] - 0x80;
    Memory[136] = i >> 8; Memory[137] = i & 0xff; /* len   xxxx  */
    Memory[138] = 240;    Memory[139] = 0;        /* WP    F000  */
    i = entry;
    Memory[140] = i >> 8; Memory[141] = i & 0xff; /* PC    start */
    
    /* Open image file and write file */
    if( (OFile = creat("eh.bin", 0644)) < 0 ) {
        fprintf(stderr, "cannot open 'eh.bin' file\n");
        exit(1);
    }
    write( OFile, &Memory[0x80], Rbr[3] - 0x80);
    close( OFile );
}

