
/* This file implements Stafford function checking (chapter 5, phase 1-3) */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "../Parse/il_opc.h"

#define TRUE  1
#define FALSE 0

/* Phase 1: build lists of function definitions and function calls.
 */

/* Build a list of functions in the program, both defined and called.
 */

#define MAXMOD 100

struct module {
    short   str_id;
    char    min;
    char    max;
    char    nargs;
    char    val;
} M[MAXMOD];

int Mod_index;

struct module *
Seek_function_def(int id)
{
    struct module *Mod;
    int i;
    
    Mod = &M[0];
    for( i = 0; i < Mod_index; i++ ) {
        if( Mod->str_id == id ) return( Mod );
        Mod++;
    }
    return( NULL );
}

int Called_Nargs;

void Function_def(int mod_id, int min, int max)
{
    struct module *Mod;
    
    Mod = Seek_function_def(mod_id);
    if( Mod == NULL ) {
        Mod = &M[Mod_index++];
        Mod->str_id = mod_id;
    }
    else {
        if( Mod->min != 1 || Mod->max != 0 ) {
            fprintf(stderr, "Warning: redefinition of module\n");
            return;
        }
    }
    Mod->min = min;
    Mod->max = max;
    Called_Nargs = FALSE;
}

void Function_ref( int mod_id )
{
    struct module *Mod;
    
    Mod = Seek_function_def(mod_id);
    if( Mod == NULL ) {
        Mod = &M[Mod_index++];
        Mod->str_id = mod_id;
        Mod->min = 1; Mod->max = 0; // impossible combo
    }
}

void Data_def( int mod_id )
{
    struct module *Mod;
    
    Mod = Seek_function_def(mod_id);
    if( Mod == NULL ) {
        Mod = &M[Mod_index++];
        Mod->str_id = mod_id;
    }
    else {
        fprintf(stderr, "Warning: redefinition of module\n");
        return;    
    }
}

void Function_fin( int mod_id, int val )
{
    struct module *Mod;
    
    Mod = Seek_function_def(mod_id);
    if( Mod == NULL ) {
        fprintf(stderr, "internal error - fin without def\n");
        return;
    }
    Mod->val = val;
    Mod->nargs = Called_Nargs;
}

/* Make a log of function calls. For now implement in memory, Stafford
 * uses a temp file for this.
 */

struct calls {
    short where;
    short who;
    char num;
    char val;
    short line;
    short file;
} Call_log[100];

int call_idx;
int Check_extrn(int ref);

int Log_calls(char *text, int idx, int offs)
{
    int ref, prev, argc, str_id;
    struct calls *call_rec;
    
    prev = argc = 0;
    while( (ref = (text[idx++] & 0xff)) != 0 ) {
        if( ref & 0x80 ) {
            idx++;
            ref = (ref & 0x7f) | (text[idx++]<< 7);
        }
        else if( ref == IL_MARK ) {
            idx = Log_calls(text, idx, offs);
        }
        else if( ref == IL_COMMA ) {
            argc++;
        }
        else if( ref == IL_NFCTN_CALL ) {
            if( (str_id = Check_extrn(prev)) ) {
                call_rec = &Call_log[call_idx++];
                call_rec->where = offs + idx - 1;
                call_rec->who   = str_id;
                call_rec->num   = argc;
                call_rec->val   = text[idx] != IL_RQD;
                call_rec->line  = 0;
                call_rec->file  = 0;
                if( str_id == 3 ) Called_Nargs = 1;
                Function_ref( str_id );
                //fprintf(stderr, "CALL at loc %d, argc = %d, name %d, ret %d\n", offs+idx-1, argc, str_id, call_rec->val );
            }
            return( idx );
        }
        prev = ref;
    }
    return( idx );
}

/* Phase 2: seek libraries to find missing function definitions */

#include <fcntl.h>

struct lookup {
    int  str_id;
    int  lib_id;
    int  lib_idx;
    char name[33];
} Lookup_list[100], *Item;

int  lookup_index;
int  SFile;

void
Add_lookup( int str_id )
{
    char *p;

    Item = &Lookup_list[lookup_index++];
    Item->str_id = str_id;
    Item->lib_id = 0;
    p = Item->name;
    lseek(SFile, str_id, SEEK_SET);
    do {
        read(SFile, p, 1);
    } while( *p++ != 0 );
    //fprintf(stderr, "Define '%s' (%d)\n", Lookup_list[lookup_index-1].name, str_id );
}

void
Fetch_names()
{
    struct module *Mod;
    int i;
    
    SFile = open("strings", O_RDONLY);
    Mod = &M[0];
    for( i = 0; i < Mod_index; i++ ) {
        if( Mod->min == 1 && Mod->max == 0 ) {
            Add_lookup( Mod->str_id );
            //fprintf(stderr, "Function %s undefined\n", Lookup_list[lookup_index-1].name );
        }
        Mod++;
    }
    close( SFile );
}

unsigned char Buf[512];
int LFile;

int
LGet(void)
{
    char ch;
    
    if( read(LFile, &ch, 1) <= 0 ) {
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

    if( (cmd = LGet()) != 0 )
    {
        Buf[0] = cmd;
        Buf[1] = len = LGet();
        for( i = 0; i < len; ++i)
            Buf[2+i] = LGet();
        Buf[len+2] = LGet();
        
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
void
Scan_lib(void)
{
    int cmd, n, w, i, id;
    struct module *Mod;
    static int idx;

    idx++;
    w = n = -1;
    while( TRUE ) {
        switch( cmd = Get_command() ) {

        case 'F':  /* Start of library index */
            n = Fetch_word( 2 );
            w = Fetch_word( 4 );
            id = 0;
            break;

        case 'N':  /* Map names to lib IDs */
            Item = &Lookup_list[0];
            for( i = 0; i < lookup_index; i++, Item++ ) {
                if( strcmp( (char *)&Buf[4], Item->name) ) continue;
                Item->lib_id = id + 1;
                Item->lib_idx = idx;
                break;
            }
            id++; n--;
            break;

        case 'W':  /* Update mapped modules */
            if( n != 0 )
                fprintf(stderr, "warning: name list has wrong length\n");
            id = Fetch_word( 2 ) + 1;
            Item = &Lookup_list[0];
            for( i = 0; i < lookup_index; i++, Item++ ) {
                if( id != Item->lib_id || idx != Item->lib_idx ) continue;
                Mod = Seek_function_def( Item->str_id );
                if( !Mod ) {
                    fprintf(stderr, "internal error - module not found\n");
                }
                else {
                    Mod->min   = Buf[6];
                    Mod->max   = Buf[7];
                    Mod->nargs = Buf[8];
                    Mod->val   = Buf[9];
                    Item->name[0] = 0;      // matched - don't match further
                }
                break;
            }
            w--;
            if( w == 0 ) return;
            break;
            
        // Ignore X and Y records
        case 'X':
        case 'Y':
            break;

        default:
            fprintf(stderr, "unrecognised library record: '%c' (%d)\n", cmd, cmd);
            exit(1);
        }
    }
}

void
Load_lib(char *lib_name)
{
    LFile = open( lib_name, O_RDONLY );
    if( LFile < 0 ) {
        fprintf(stderr, "Could not load %s\n", lib_name );
    }
    Scan_lib();
    close( LFile );
}

void
Fetch_libs()
{
    int i;
    char path[256];
    extern char* Extra_lib_dir;

    Fetch_names();
    if( Extra_lib_dir ) {
        strcpy( path, Extra_lib_dir );
        strcat( path, "/INDEX" );
        Load_lib(path);
    }
    Load_lib("../libb/INDEX");

    Item = &Lookup_list[0];
    for( i = 0; i < lookup_index; i++, Item++ ) {
        if( Item->lib_id == 0 ) {
            fprintf(stderr, "Function %s not found in source or libs\n", Item->name );
        }
    }
}

/* Phase 3: check the calls and patch the output file */

extern int OFile;

void
Check_calls()
{
    struct module *Mod;
    struct calls *Call;
    int i;
    char name[33], *p;
    static char test, opc = IL_FCTN_CALL;
    
    SFile = open("strings", O_RDONLY);
    Call = &Call_log[0];
    for( i = 0; i < call_idx; i++ ) {
        // lookup function definition
        Mod = Seek_function_def( Call->who );
        if( Mod == NULL ) {
            fprintf(stderr, "internal error - no definition for %d\n", Call->who);
        }
        else if( Mod->min != 1 || Mod->max != 0 ) {
            // get name
            lseek(SFile, Mod->str_id, SEEK_SET);
            p = name;
            do {
                read(SFile, p, 1);
            } while( *p++ != 0 );
            // check function call parameters & return value
            if( Mod->min > 0 && Call->num < Mod->min ) fprintf(stderr, "Too few parameters for '%s' on line %d\n", name, Call->line);
            if( Mod->max > 0 && Call->num > Mod->max ) fprintf(stderr, "Too many parameters for '%s' on line %d\n", name, Call->line);
            if( Call->val == 1 && Mod->val == 0 ) fprintf(stderr, "Function '%s' does not return a value on line %d\n", name, Call->line);
            
            // patch output file as needed
            if( Mod->nargs == FALSE ) {
                lseek( OFile, Call->where, SEEK_SET );
                //read( OFile, &test, 1);
                //fprintf(stderr, "Patch loc %d (%d)\n", Call->where, test);
                write( OFile, &opc, 1);
            }
        }
        Call++;
    }
    close( SFile );
    lseek( OFile, 0, SEEK_END );
}

