
/* Thoth tool chain library editor */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "ale.h"

int IFile, OFile, Mode;
unsigned char Buf[512];
char Path[256];
char *Ptr;

int
Get(void)
{
    char ch;
    
    if( read(IFile, &ch, 1) <= 0 ) {
        return 0;
    }
    return (ch & 0xff);
}

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

/* Write a command to the output file, including
 * the chacksum byte.
 */
void
Put_command(int cmd, int len)
{
    int i, check;
    
    Buf[0] = cmd;
    Buf[1] = len;
    check = 0;
    for( i = 0; i < len + 2; ++i) {
        check ^= Buf[i];
    }
    Buf[len + 2] = check;
    if( write(OFile, Buf, len + 3) < 0 ) {
        fprintf(stderr, "error writing output file [%c]\n", Buf[0]);
        exit(1);
    }
}

/* Read a 16-bit word from the command buffer at position i */
int
Fetch_word(int i)
{
    return( (Buf[i]<<8) + Buf[i+1] );
}

/* Write a 16-bit word to the command buffer at position i */
void
Store_word(int i, int val)
{
    Buf[i]   = val >> 8;
    Buf[i+1] = val;
}

/* Read a library index file from the input stream.
 */
void
Get_lib_index(void)
{
    int cmd, n, w, len, i;
    struct Symbol *sym;
    struct Module *mod;

    w = n = 0;
    while( 1 ) {        
        switch( cmd = Get_command() ) {

        case 'F':  /* Start of library index */
            n = Fetch_word( 2 );
            w = Fetch_word( 4 );
            if( n > MAXSYM || w > MAXMOD ) {
                fprintf(stderr, "error: library too large\n");
                exit(1);
            }
            break;

        case 'N':  /* Read a symbol name definition */
            sym = Sym_lookup( &Buf[4] );
            sym->count = Fetch_word( 2 );
            n--;
            break;

        case 'W':  /* Read a module definition */
            if( n != 0 )
                fprintf(stderr, "warning: name list has wrong length\n");
            sym = Sym_byindex( Fetch_word( 2 ) );
            if( sym->module != NULL ) {
                fprintf(stderr, "error: module %s redefined in index\n", sym->name );
                exit( 1 );
            }
            mod = &Modules[ Next_module++ ];

            sym->module = mod;
            mod->symindex = Fetch_word( 2 );
            mod->data[0] = Buf[6]; /* min arg */
            mod->data[1] = Buf[7]; /* max arg */
            mod->data[2] = Buf[8]; /* nargs   */
            mod->data[3] = Buf[9]; /* retval  */
            w--;
            break;

        case 'X':  /* Read module references */
            len = Buf[1] / 2;
            mod->refs = Refs_base( len );
            for( i = 0; i < len; i++ ) {
                mod->refs[i] = Fetch_word( 2 + 2*i );
            }
            mod->refcount = len;
            break;

        case 'Y':  /* Entry points */
            /* Ignore for now, as the compiler does not produce this */
            break;
            
        case 'Z':  /* End library index */
        case 0:
            if( w != 0 )
                fprintf(stderr, "warning: module list has wrong length\n");
            return;

        default:
            fprintf(stderr, "unrecognised library record: '%c' (%d)\n", cmd, cmd);
            exit(1);
        }
    }
}

void
Put_lib_index(void)
{
    struct Symbol *sym;
    struct Module *mod;
    int i, j, len;

    /* Write library header */
    Store_word(2, Next_symbol);
    Store_word(4, Next_module);
    Put_command('F', 4);
    
    /* Write symbols */
    for( i = 0; i < Next_symbol; i++ ) {
        sym = Sym_byindex( i );
        len = Length( sym->name );
        Store_word( 2, sym->count );
        for( j = 0; j < len; j++ )
            Buf[4 + j] = sym->name[j];
        Put_command('N', len + 2 );
    }
    
    /* Write modules incl. references */
    for( i = 0; i < Next_module; i++ ) {
        mod = &Modules[ i ];
        Store_word( 2, mod->symindex );
        Store_word( 4, 0 );
        Buf[6] = mod->data[0];
        Buf[7] = mod->data[1];
        Buf[8] = mod->data[2];
        Buf[9] = mod->data[3];
        Put_command('W', 8);
        
        for( j = 0; j < mod->refcount; j++ ) {
            Store_word( 2 + 2*j, mod->refs[j] );
        }
        Put_command('X', 2*j);
    }

    /* Write library footer */
    Put_command('Z', 0);
}

int
main( int argc, char **argv )
{
    int i, save;
    
    if( argc == 1 ) {
        fprintf(stderr, "Usage: %s <lib path> [-u|-a] <.out file>]\n", argv[0] );
        exit( 1 );
    }

    /* Read index file */
    Ptr = Copy( argv[1], Path );
    if( Ptr[-1] != '/' )
        Ptr = Copy( "/", Ptr );
    Copy( "INDEX", Ptr );
    
	if( (IFile = open(Path, O_RDWR|O_CREAT, 0644)) < 0 ) {
        fprintf(stderr, "cannot open library index %s\n", Path);
        exit(1);
    }
    Get_lib_index();
    save = IFile;

    /* Add or update new modules as needed */
    if( argc == 4 ) {
        Mode = argv[2][1]; /* 'u' (update) or 'a' (add) */
        if( (IFile = open(argv[3], O_RDONLY)) < 0 ) {
            fprintf(stderr, "cannot open compiler output file %s\n", argv[3]);
            exit(1);
        }
        Process_out_file();
    }
    
    /* Write new index file */
    OFile = save;
    lseek(OFile, 0, SEEK_SET);
    Put_lib_index();

    return( 0 );
}
