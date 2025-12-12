
/* Preprocessor routines */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "tla.h"
#include "tokens.h"
#include "load.h"

int Line_no;
int Path_ID;
int IFile;
int OFile = 1;
int MFile;
int Err_count;

int Cache;
int In_pushed_back;
int In_string;

int Cond_level;
int Cond_ign;

struct Input {
    struct Input *link;
    int pos;
    int line_no;
    char *path;
} *Input;

int Level;

void
Error(char *error)
{
    fprintf(stderr, "Error %s, on line %d", error, Line_no );
    if( Level > 1 ) {
        fprintf(stderr, " in file %s\n", Input->path);
    }
    else
        fprintf(stderr, "\n");
    exit(1);
}

struct Input *
Input_alloc( void )
{
    struct Input *inp;
    
    inp = (struct Input *) malloc( sizeof(struct Input) );
    inp->link = Input;
    Input = inp;
    return( inp );
}

void
Input_free( struct Input *inp )
{
    free( inp->path );
    free( inp );
}

void
Enter_level( char *path, int offs )
{
    if( Input ) {
        Input->line_no = Line_no;
        Input->pos = lseek(IFile, 0, SEEK_CUR) - In_pushed_back;
    }

    close(IFile);
    if( (IFile = open( path, O_RDONLY )) < 0 )
        Error("cannot open file");
    Input = Input_alloc();
    Input->path = strdup( path );
    lseek( IFile, offs, SEEK_SET );

    if( !offs ) Line_no = 1;
    Cache = '\n';
    In_pushed_back = FALSE;
    ++Level;
}

void
Leave_level( void )
{
    struct Input *inp;

    inp = Input;
    Input = inp->link;
    Input_free( inp );

    close(IFile);
    if( Input ) {
        if( (IFile = open( Input->path, O_RDONLY )) < 0 )
            Error("cannot open file");
        lseek(IFile, Input->pos, SEEK_SET);
        Line_no  = Input->line_no;
    }
    In_pushed_back = FALSE;
    --Level;
}
/*
void
Get_eol(void)
{
    char ch;
    if( In_pushed_back && Cache == '\n' )
        return;

    do {
        if( read(IFile, &ch, 1) <= 0 ) break;
    } while( ch != '\n' );

    Cache = '\n';
    In_pushed_back = TRUE;
}
*/
void
Get_eol(void)
{
    while( Get_ind() != '\n' ) ;
    In_pushed_back = TRUE;
}
/*
void
Get_white(void)
{
    char ch;

    if( In_pushed_back && Cache != ' ' && Cache != '\t' )
        return;

    do {
        if( read(IFile, &ch, 1) <= 0 ) {
            ch = 0;
        }
    } while( ch == ' ' || ch == '\t' );

    Cache = ch;
    In_pushed_back = TRUE;
}
*/
void
Get_white(void)
{
    char ch;
    while( (ch = Get_ind()) == ' ' || ch == '\t' ) ;
    In_pushed_back = TRUE;
}

#define BUFLEN 256
char Path[BUFLEN];

void
Get_path(void)
{
    char ch;
    int i = 0;

    Get_white();
    Path[i++] = Cache;
    In_pushed_back = FALSE;
    do {
        if( read(IFile, &ch, 1) <= 0 ) {
            ch = 0;
        }
        if( ch == '\n' ) ch = 0;
        Path[i++] = ch;
        if( ch != 0 && i == BUFLEN-1 ) {
            Error("path too long");
        }
    } while( ch != ' ' && ch != '\t' && ch != 0 );
    Path[i-1] = 0;
    if( ch == 0 ) {
        Cache = '\n';
        In_pushed_back = TRUE;
    }
}

void
PutM(int ch)
{
    char byte = ch;
    write(MFile, &byte, 1);
}

int
Intern_manifest(void)
{
    char ch;
    int loc;

    loc = lseek(MFile, 0, SEEK_END);

    do {
        if( read(IFile, &ch, 1) <= 0 ) {
            ch = 0;
        }
        if( ch == '\\' ) {
            Get_eol();
            ch = 0;
        }
        else if( ch == '\n' ) {
            ch = 0;
        }
        if( ch == 0 ) {
            PutM(' ');
        }
        PutM(ch);
    } while( ch != 0 );
    Cache = '\n';
    In_pushed_back = TRUE;
    return loc;
}

void
In_expand(int loc)
{
    Enter_level( "manifests", loc );
}

void
Define_manifest()
{
    struct Symbol *sym;

    Get_white();
    Get_ident();
    Get_white();
    if( Get_ind() != '=' ) {
        Error("missing '=' in manifest definition");
        Get_eol();
        return;
    }
    sym = Sym_lookup(TRUE);
    switch( sym->type ) {
    
    case UNDEFINED:
        sym->type = MANIFEST;
        sym->val = Intern_manifest();
        break;
        
    case MANIFEST:
        if( Pass == 2 ) {
            Get_eol();
            break;
        }
        // maybe allow redef ?
        /* fall through */
    default:
        Error("identifier redefined");
        Get_eol();
    }
    Cache = '\n';
}

void
Include_file(void)
{
    Get_path();
    Get_eol();
    Enter_level( Path, 0 );
}

int
Get_ind(void)
{
    char ch;

again:
    if( In_pushed_back ) {
        In_pushed_back = 0;
        return Cache;
    }

    if( read(IFile, &ch, 1) <= 0 || ch == 0 ) {
        if( Level > 1) {
            Leave_level();
            goto again;
        }
        return( Cache = 0 );
    }

    if( !In_string && Cache == '\n' ){
        switch(ch) {
        case '#': Define_manifest(); goto again;
        case '%': Include_file();    goto again;
        }
    }
    Cache = ch;
    return ch;
}

int Pass;

int
main( int argc, char **argv )
{

    if( argc < 2 ) {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
        return 0;
    }

    Init_pseudo();
    Init_opcodes();

    OFile = open( "eh.out", O_WRONLY|O_CREAT|O_TRUNC, 0644 );
    if( OFile < 0 ) Error("cannot open 'eh.out' file");

    MFile = open( "manifests", O_WRONLY|O_CREAT|O_TRUNC, 0644 );
    if( MFile < 0 ) Error("cannot open 'manifests' file");
    PutM(' ');

    for( Pass = 1; Pass < 3; Pass++ ) {
    
        Clean();
        Enter_level( argv[1], 0 );
        Parse();
        Flush_load_bufs();
        
        if( Err_count > 0 ) {
            break;
        }
    }

    close( IFile );
    close( OFile );
    close( MFile );
    unlink("manifests");
    return 0;
}
