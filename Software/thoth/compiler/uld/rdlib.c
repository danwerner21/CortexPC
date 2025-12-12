
/* Read library index */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "uld.h"

unsigned char Buf[512];
char Path[256];
char *Ptr;

#define MAXSYM 512

char  Names[MAXSYM * 8 ];
char *Name[MAXSYM];
char *Next_name;

/* Read a loader command from the input stream. Replace
 * checksum byte with a terminating zero.
 */
int
Get_command(void)
{
    int check, i, cmd, len;

    if( (cmd = Get()) != 0 )
    {
        Buf[0] = cmd;
        Buf[1] = len = Get();
        for( i = 0; i < len; ++i)
            Buf[2+i] = Get();
        Buf[len+2] = Get();
        
        check = 0;
        for( i = 0; i < len + 3; ++i ) {
            check ^= Buf[i];
        }
        if( check != 0 ) {
            fprintf(stderr, "Bad checksum %x\n", check);
            exit(1);
        }
        Buf[len+2] = 0;
    }
    return( cmd );
}

/* Read a 16-bit word from the command buffer at position i */
int
Fetch_word(int i)
{
    return( (Buf[i]<<8) + Buf[i+1] );
}

/* Read a library index file from the input stream.
 */
int
Get_lib_index( struct Files *file )
{
    struct Symbol *sym;
    char *name;
    int cmd, n, na, w, len, i;
    int reload, been_there;

    w = n = -1;
    reload = FALSE;
    been_there = (Next_name != Names);
    while( 1 ) {        
        switch( cmd = Get_command() ) {

        case 'F':  /* Start of library index */
            n = Fetch_word( 2 );
            w = Fetch_word( 4 );
            if( n > MAXSYM ) {
                fprintf(stderr, "error: library too large\n");
                exit(1);
            }
            na = 0;
            break;

        case 'N':  /* Read a symbol name definition */
            if( !been_there ) {
                Name[na++] = Next_name;
                Next_name = Copy((char*)&Buf[4], Next_name) + 1;
                if( (Next_name - Names) >= MAXSYM * 8 ) {
                    fprintf(stderr, "error: lib name space full\n");
                    exit(1);
                }
            }
            break;

        case 'W':  /* Check a module for relevance */
        next_module:
            w--;
            name = Name[ Fetch_word( 2 ) ];
            //fprintf(stderr, "reading module %s ... ", name);
            sym = Global_lookup( name, FALSE );
            if( !sym || (sym->Rbr & (DEFINED|PENDING)) ) {
                // not needed or already defined: skip this module
                //fprintf(stderr, "skipped\n");
                while( (cmd = Get_command()) && cmd != 'W' )
                    if( cmd == 0 || cmd == 'Z' ) return reload;
                goto next_module;
            }
            if( sym ) {
                // in graph, but undefined: add module to load list
                //fprintf(stderr, "loaded\n");
                sym->Rbr |= PENDING;
                sym = Sym_alloc( name );
                sym->link = file->modules;
                file->modules = sym;
            }
            break;

        case 'X':  /* Put module references in graph */
            len = Buf[1] / 2;
            for( i = 0; i < len; i++ ) {
                name = Name[ Fetch_word( 2 + 2*i ) ];
                sym = Global_lookup( name, FALSE );
                if( !sym ) {
                    //fprintf(stderr, "  needs %s\n", name);
                    reload = TRUE;
                    sym = Global_lookup( name, TRUE );
                }
            }
            break;

        case 'Y':  /* Put module entry points in graph */
            len = Buf[1] / 2;
            for( i = 0; i < len; i++ ) {
                name = Name[ Fetch_word( 2 + 2*i ) ];
                sym = Global_lookup( name, TRUE );
                if( sym && !(sym->Rbr & DEFINED) ) {
                    sym->Rbr |= PENDING;
                }
            }
            break;
            
        case 'Z':  /* End library index */
        case 0:
            if( n != na )
                fprintf(stderr, "warning: name list has wrong length\n");
            if( w != 0 )
                fprintf(stderr, "warning: module list has wrong length\n");
            return reload;

        default:
            fprintf(stderr, "unrecognised library record: '%c' (%d)\n", cmd, cmd);
            exit(1);
        }
    }
}

void
Scan_lib( struct Files *file )
{
    Ptr = Copy( file->path, Path );
    if( Ptr[-1] != '/' )
    Ptr = Copy( "/", Ptr );
    Copy( "INDEX", Ptr );

    //fprintf(stderr, "reading library %s\n", Path);
    if( (IFile = open(Path, O_RDONLY)) < 0 ) {
        fprintf(stderr, "cannot open library index %s\n", Path);
        exit(1);
    }

    Next_name = Names;
    while( Get_lib_index( file ) )
        lseek( IFile, 0, SEEK_SET);
        
    close( IFile );
}

