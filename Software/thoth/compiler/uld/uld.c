
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "uld.h"

int IFile;
unsigned char *Memory;

int
Get(void)
{
    char ch;
    
    if( read(IFile, &ch, 1) <= 0 ) {
        return 0;
    }
    return (ch & 0xff);
}

#define UNIT 1
#define LIB  2

struct Files *First_file, *Cur_file;

struct Files *
File_alloc( void )
{
    struct Files *file;
    
    file = (struct Files *) malloc( sizeof(struct Files) );
    if( !file ) {
        fprintf(stderr, "out of memory\n" );
        exit(1);
    }
    memset( file, 0, sizeof( struct Files ) );
    return( file );
}

void
Load_unit( char *path, int pass )
{
	if( (IFile = open(path, O_RDONLY)) < 0 ) {
        fprintf(stderr, "cannot open '%s' file\n", path );
        exit(1);
    }
    if( pass == PASS_ONE ) Pass_one(); else Pass_two();
    close( IFile );
}

void
Load_file( char *path )
{
    struct Files *file;
    struct Symbol *module;
    struct stat status;
    char here[256];

    if( stat(path, &status) <0 ) {
        fprintf(stderr, "cannot stat path '%s'\n", path );
        exit(1);
    }
    
    file = File_alloc();
    if( !First_file )
        Cur_file = First_file = file;
    else {
        Cur_file->link = file;
        Cur_file = file;
    }
    file->path = path;
    
    if( S_ISREG(status.st_mode) ) {
        file->type = UNIT;
        Load_unit( path, PASS_ONE );
    }
    else {
        file->type = LIB;
        Scan_lib( file );
        getcwd( here, 256 );
        chdir( file->path );
        for( module = file->modules; module; module = module->link ) {
            Load_unit( module->name, PASS_ONE );
        }
        chdir( here );
    }
}

int flagv, flagl;

void
Usage( char *name )
{
    fprintf(stderr, "Usage: %s [-v] [-l] file...\n", name);
    exit(1);
}

int
main(int argc, char *argv[] )
{
    struct Files *file;
    struct Symbol *module;
    char *path;
    char here[256];

    path = argv[0];
    argv++; argc--;
    while( argc && argv[0][0] == '-' ) {
        switch( argv[0][1] ) {
        case 'v': flagv++; break;
        case 'l': flagl++; break;
        default:  Usage( path );
        }
        argv++; argc--;
    }
    if( !argc ) Usage( path );

    Memory = (unsigned char *) malloc( 65536 );
    if( !Memory ) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    /* Process files */
    Load_file( "../libb/..start" );
    while( argc ) {
        Load_file( argv[0] );
        argv++; argc--;
    }
    Load_file( "../libb" );
    
    Rbr_rebase();
    
    /* Rescan all files for pass two */
    for( file = First_file; file; file = file->link ) {
        if( file->type == UNIT )
            Load_unit( file->path, PASS_TWO );
        else {
            getcwd( here, 256 );
            chdir( file->path );
            for( module = file->modules; module; module = module->link ) {
                Load_unit( module->name, PASS_TWO );
            }
            chdir( here );
        }
    }

    /* Output */
    if( flagl )
        Gen_listing();
    Sym_dump();
    Gen_MDEX();
}
