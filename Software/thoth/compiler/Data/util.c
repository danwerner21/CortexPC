
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "load.h"
#include "mani.h"

int IFile;
int SFile;

int
Get(void)
{
    char byte;
    
    if( read(IFile, &byte, 1) < 0 )
        Error("could not read input file");
    return(byte&0xff);
}

int
Get_id(void)
{
    int str_id;
    str_id = (Get() << 8) | Get();
    return str_id;
}

void
Get_name(int id, char *name)
{
    char ch;
    int i;

    lseek(SFile, id, SEEK_SET);
    for( i = 0; i < 32; ++i ) {
        if( read(SFile, &ch, 1) < 0 )
            Error("cannot read strings file");
        if( ch == 0 ) break;
        *name++ = ch;
    }
    if( i < 32 ) *name = 0;
}

int
Get_constant(int c)
{
    int len, i, val;

    val = 0;
    len = c & 0x7f;
    for( i = 0; i < len; ++i ) {
        val = (val << 8) + Get();
    }
    return(val);
}

extern int Data_rb;

void
Copy_string(int min_length, int str_id)
{
    char ch;
    int len, i;
    
    lseek(SFile, str_id, SEEK_SET);
    read(SFile, &ch, 1);
    len = ch & 0xff;
    read(SFile, &ch, 1);
    len = (len << 8) + ch & 0xff;

    Push_loc();
    for( i = 0; i < len; ++i ) {
        read(SFile, &ch, 1);
        Load_byte( ch & 0xff );
    }
    if( *Counter < min_length ) *Counter = min_length;
    Pop_loc();
    Flush_load_bufs();
}

void
Error(char *msg)
{
    fprintf(stderr, "error: %s\n", msg);
    exit(1);
}

void
Initialize(int argc, char **argv)
{
    IFile = 0;
    
    if( argc > 1 ) {
        IFile = open(argv[1], O_RDONLY);
        if( IFile < 0 ) {
            Error("could not open input file");
        }
    }
    SFile = open("strings", O_RDONLY);
    if( SFile < 0 )
        Error("could not open strings file");
        
    OFile = 1;
}

int id_tab[256];
int refno;

int
Search_wsd(int id, char* name)
{
    int i;

    for( i = refno; i < 256; ++i) {
        if( id_tab[i] == id ) return i;
    }
    
    if( refno <= Counter_level + 1 )
        Error("Wsd number overflow");
    id_tab[--refno] = id;
    Put_load_ref( 'g', refno, name );
    return refno;
}

char *
Create_name(int wsd)
{
    static char name[33];
    sprintf(name, "X%d", wsd);
    return name;
}

