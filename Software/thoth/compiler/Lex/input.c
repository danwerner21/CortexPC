
/* Preprocessor routines */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"
#include "lexil.h"

#define BUFLEN 256
char Path[BUFLEN];

int IFile;
int OFile = 1;
int MFile, CFile;

int Line_no;

int Cache;
int In_pushed_back;
int In_string;

int Cond_level;
int Cond_ign;

struct Frame {
    struct Frame *link;
    int   manifest;
    int   pos;
    int   line_no;
    char *path;
    int   path_id;
    int   ifile;
    char *at_dir;
} Frames[10], *Frame = Frames;

int Level;

int Get_next(void);

void
Put(int token)
{
    char byte = token;
    write(OFile, &byte, 1);
}

void
PutM(int ch)
{
    char byte = ch;
    write(MFile, &byte, 1);
}

void
PutW(int word)
{
    char byte;
    
    byte = (word >> 8) & 0xff;
    write(OFile, &byte, 1);
    byte = word & 0xff;
    write(OFile, &byte, 1);
}

void
Error(char *error)
{
    fprintf(stderr, "Error %s, on line %d", error, Line_no );
    if( Level > 1 ) {
        fprintf(stderr, " in file %s\n", Frame->path);
    }
    else
        fprintf(stderr, "\n");
    exit(1);
}

void
Enter_level( char *path, int manifest )
{

    if( Level >= 10 )
        Error( "%includes nested too deeply" );

    if( Level ) {
        Frame->line_no = Line_no;
        Frame->pos     = lseek(IFile, 0, SEEK_CUR) - In_pushed_back;
        Frame->at_dir  = getcwd(NULL, 0);
    }

    ++Level;
    ++Frame;
    Frame->manifest = manifest ? TRUE : FALSE;
    if( manifest ) {
        IFile = MFile;
    }
    else if( (IFile = open( path, O_RDONLY )) < 0 )
        Error("cannot open file");
    lseek( IFile, manifest, SEEK_SET );
    Frame->ifile  = IFile;
    Frame->path_id = Intern_path( path );
    Frame->path    = strdup( path );

    if( !manifest ) {
        Line_no = 0;
        Put( NEW_FILE ); PutW( Frame->path_id ); PutW( Line_no ); Put( 255 );
    }
    Cache = '\n';
    In_pushed_back = !manifest;
}

void
Leave_level( void )
{
    int manifest;

    if( !Frame->manifest )
        close( IFile );
    free( Frame->path );
    manifest = Frame->manifest;

    --Level;
    --Frame;
    IFile = Frame->ifile;
    Line_no  = Frame->line_no;
    lseek(IFile, Frame->pos, SEEK_SET);
    chdir( Frame->at_dir );
    free( Frame->at_dir );

    In_pushed_back = FALSE;
    if( !manifest ) {
        Put( NEW_FILE ); PutW( Frame->path_id ); PutW( Line_no ); Put( 255 );
    }
}

void
Get_eol(void)
{
    while( Get_next() != '\n' ) ;
    In_pushed_back = TRUE;
}

void
Get_white(void)
{
    char ch;
    while( (ch = Get_next()) == ' ' || ch == '\t' ) ;
    In_pushed_back = TRUE;
}

void
Get_path(void)
{
    char ch;
    int i = 0;

    Get_white();
    if( Cache != '/' ) {
        getcwd( Path, BUFLEN );
        i = strlen( Path );
        Path[i++] = '/';
    }
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

int
Intern_manifest( void )
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
Check_redef( int loc )
{
    int sav;
    char c1, c2;
    
    sav = Intern_manifest();
    lseek( MFile, sav, SEEK_SET );
    lseek( CFile, loc, SEEK_SET );
    while( read( MFile, &c1, 1 ) == 1 && c1 != 0 ) {
        if( read( CFile, &c2, 1 ) == 1 && c1 != c2 )
            Error( "manifest redefined" );
    }
    if( read(IFile, &c2, 1) != 1 && c2 != 0 )
        Error( "manifest redefined" );
    ftruncate( MFile, sav );
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
        sym->loc = Intern_manifest();
        break;
        
    case MANIFEST:
        Check_redef( sym->loc );
        break;
        
    default:
        Error("manifest redefined");
        Get_eol();
    }
    Cache = '\n';
}

void
In_expand(int loc)
{
    Enter_level( "manifests", loc );
}

void
Set_path(void)
{
    Get_white();
    Get_path();
    if( chdir( Path ) < 0 )
        Error("cannot set base directory");
}

void
Include_file(void)
{
    Get_path();
    Get_eol();
    Enter_level( Path, 0 );
}

void
Process_ifdef(void)
{
    struct Symbol *sym;
    int found;

    Get_path();
    if( Equal( Path, "ifdef") ) {
        ++Cond_level;
        Get_white();
        Get_ident();
        found = (sym = Sym_lookup(FALSE)) != NULL && sym->type == MANIFEST;
        if( found == 0 && Cond_ign == 0)
            ++Cond_ign;
    }
    else if( Equal( Path, "else") ) {
        if( Cond_level == 0)
            Error("?else without ?ifdef");
        else
            Cond_ign = (Cond_ign < 2) ? (1 - Cond_ign) : Cond_ign;
    }
    else if( Equal( Path, "endif") ) {
        if( Cond_level > 0)
            --Cond_level;
        else
            Error("?endif without ?ifdef");
        if( Cond_ign > 0)
            --Cond_ign;
    }
    else {
        Error("bad ?condition command");
    }
    Get_eol();
}

int
Get_next(void)
{
    char ch;

    while( 1 ) {
        if( In_pushed_back ) {
            In_pushed_back = 0;
            return Cache;
        }

        if( read(IFile, &ch, 1) <= 0 || ch == 0 ) {
            if( Cond_level ) {
                Error( "?ifdef without ?end" );
            }
            if( Level > 1) {
                Leave_level();
                continue;
            }
            return( Cache = 0 );
        }

        if( !In_string && Cache == '\n' ){
            switch(ch) {
            case '#': Define_manifest(); continue;
            case '@': Set_path();        continue;
            case '%': Include_file();    continue;
            case '?': Process_ifdef();   continue;
            /* Note: Zed adds '$' to set relocation segments */
            }
        }
        Cache = ch;
        return ch;
    }
}

int
Get_ind(void)
{
    char ch;
    
    do {
        if( (ch = Get_next()) == '\n' && Cond_ign ) {
            ++Line_no;
            Put( END_LINE );
        }
    } while( Cond_ign );
    return( ch );
}

int
main(int argc, char *argv[] )
{
    int fd, id;
    char *file;

    Intern_init();
    MFile = open( "manifests", O_RDWR|O_CREAT|O_TRUNC, 0644 );
    CFile = open( "manifests", O_RDONLY );
    PutM( 0 );
    
    file = argc < 2 ? "test.b" : argv[1];
    if( file[0] != '/' ) {
        getcwd( Path, BUFLEN );
        strcat( Path, "/" );
        strcat( Path, file );
    }

    Enter_level( Path, 0 );
    
    Lex();
    //Sym_dump();

    close( MFile );
    close( CFile );
}
