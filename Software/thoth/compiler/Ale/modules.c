
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "ale.h"

struct Module Modules[MAXMOD];
short References[MAXREFS];
int Next_module;
int Next_ref;

short *
Refs_base(int count)
{
    short *ret;

    ret = &References[ Next_ref ];
    Next_ref += count;
    return( ret );
}

void
Process_out_file( void )
{
    int cmd;
    struct Symbol *sym;
    struct Module *mod;

    /* Scan the compiler output file and add to library index
     * and copy each module to the library directory.
     */
    OFile = -1;
    while( 1 ) {        
        switch( cmd = Get_command() ) {
        
        case 'M':
            /* Add module to index */
            sym = Sym_lookup( &Buf[4] );
            if( sym->module != NULL ) {
                mod = sym->module;
                if( Mode == 'a' ) {
                    fprintf(stderr, "module %s redefined\n", sym->name );
                    exit(1);
                }
            }
            else {
                mod = &Modules[ Next_module++ ];
                mod->symindex = Sym_index( sym );
                sym->module = mod;
                sym->count++;
            }
            mod->refcount = 0;
            mod->refs = &References[Next_ref];
            
            /* Initiate copy to module file */
            Copy( (char *)&Buf[4], Ptr);
            if( (OFile = open(Path, O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0 ) {
                fprintf(stderr, "cannot create module file %s\n", Path);
            }
            break;
        
        case 'g':
            /* Add reference to index */
            sym = Sym_lookup( &Buf[3] );
            References[ Next_ref++ ] = Sym_index( sym );
            mod->refcount++;
            break;

        case 'E':
            /* Add function particulars to index */
            mod->data[0] = Buf[2];
            mod->data[1] = Buf[3];
            mod->data[2] = Buf[4];
            mod->data[3] = Buf[5];
            break;
        
        case 'G': case 'T': case 't': case 'L': case 'O': case 'A': case 'I':
            break;
        
        case 0:
            return;
            
        default:
            fprintf(stderr, "bad format in compiler output file\n");

        }
        
        /* Copy command to library module file and close after 'E' */
        if( OFile >= 0 ) {
            Put_command( Buf[0], Buf[1] );
            if( Buf[0] == 'E' ) {
                write(OFile, "", 1);
                close(OFile);
                OFile = -1;
            }
        }
    }
}
