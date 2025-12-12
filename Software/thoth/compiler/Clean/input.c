
/* Start of module 'Clean' (Stafford chapter 5 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define TRUE  1
#define FALSE 0

int IFile;
int OFile;
char *Extra_lib_dir;

char buffer[256];
char *ptr;
int no_opt;

int
Get(void)
{
    char ch;
    
    if( read(IFile, &ch, 1) <= 0 ) {
        exit(1);
    }
    return (ch & 0xff);
}

int
GetW(void)
{
    return (Get() << 8) + Get();
}

int
BufW(void)
{
    *ptr++ = Get();
    *ptr++ = Get();
    return( ((ptr[-2]<<8) & 0xff00) + (ptr[-1] & 0xff) );
}

int
BufR(void)
{
    int byte = Get();
    
    if( (*ptr++ = byte) & 0x80 ) {
        *ptr++ = Get();
        return ((ptr[-1]&0xff)<<7) + (byte & 0x7f) ;
    }
    return( byte );
}

void
Buf_expr(void)
{
    while( BufR() != 0 ) ;
    if( buffer[0]=='X' )
        for( char *p = buffer; p < ptr; ++p )
            fprintf(stderr, "%02x-", *p&0xff);
}

void
Put_buffer(void)
{
    //fprintf(stderr, "%c ", buffer[0]);
    write(OFile, buffer, ptr - buffer);
    ptr = buffer;
}

#define MAXCONST 100

struct Const_tab {
    int ref;
    int val;
} Const_tab[MAXCONST];

static int Const_index;

void
Add_constant(int ref, int val)
{
    Const_tab[Const_index].ref = ref;
    Const_tab[Const_index].val = val;
    //fprintf( stderr, "ref %d is const %d\n", ref, Const_index );
    ++Const_index;
}

int
Check_const(int ref, int *d)
{
    int i;
    
    for( i = 0; i < Const_index; ++i )
    {
        if( Const_tab[i].ref == ref )
        {
            *d = -1;
            //fprintf( stderr, "for ref %d d set to -1 (%d)\n", ref, i );
            return( Const_tab[i].val );
        }
    }
    *d = 0;
    return( 0 );
}

/* Maybe merge the below code with code for constants above, but constants
 * can be larger than 2 bytes when implemented as described in Stafford.
 */

#define MAXEXTRN 100

struct Extrn_tab {
    int ref;
    int str_id;
} Extrn_tab[MAXEXTRN];

static int Extrn_index;

void
Add_extrn(int ref, int str_id)
{
    Extrn_tab[Extrn_index].ref    = ref;
    Extrn_tab[Extrn_index].str_id = str_id;
    //fprintf( stderr, "ref %d is str_id %d\n", ref, str_id );
    ++Extrn_index;
}

int
Check_extrn(int ref)
{
    int i;
    
    for( i = 0; i < Extrn_index; ++i ) {
        if( Extrn_tab[i].ref == ref ) {
            return( Extrn_tab[i].str_id );
        }
    }
    return( 0 );
}

/* ---- */

void Add_stmt(void);
void Clean_stmts(void);
void Put_stmts(void);

int  Expr_read( char *refptr );
void Expr_print( int node );
int  Expr_write(int expr, char* start);
int  Expr_optim(int node);

void
Put(int c)
{
    char byte = c;
    write(OFile, &byte, 1);
}

int Mod_name; // string id of current module definition
int Mod_val;  // boolean, function returns value;

void Function_def( int mod_id, int min, int max );
void Function_fin( int mod_id, int val );

void
read_func(void)
{
    int stmt, val, ref, expr;
    char *p;

    no_opt = FALSE;
    while( (stmt = Get()) != 0 ) {

        ptr = buffer;
        *ptr++ = stmt;
        
        switch( stmt ) {
        
        /* ==== function header and declarations ==== */

        case 'P':  /* prolog XXX */
            *ptr++ = Get(); /* min */
            *ptr++ = Get(); /* max */
            *ptr++ = Get(); /* seg */
            Mod_val = 0;
            Function_def(Mod_name, ptr[-3], ptr[-2]);
            break;

        case 'a':  /* argument */
        case 'A':  /* auto */
            BufR();
            break;
        
        case 'V':  /* auto vector */
        case 'v':  /* auto string */
        case '"':  /* declare string REF */
        case '\'': /* declare twit string */
            BufR();
            BufW();
            break;

        case 'g':  /* external name */
            ref = BufR();
            val = BufW();
            Add_extrn(ref, val);
            break;
        
        case 'c':  /* declare constant REF */
            ref = BufR();
            val = BufW();
            Add_constant( ref, val );
            break;
        
        /* ===== statements ====== */

        case 'H': /* start of executable body */
        case 'w': /* end select word */
        case 's': /* end select string */
        case 'D': /* default */
        case 'e': /* enable */
        case 'd': /* disable */
            Add_stmt();
            continue;

        case 't': /* twit stmt */
            no_opt = TRUE;
        case 'E': /* expression */
        case 'R': /* return */
        case 'W': /* select word */
        case 'S': /* select string */
            Buf_expr();
            expr = Expr_read( &buffer[1] );
            //fprintf( stderr, "\n%c:\n", stmt );
            //Expr_print( expr );
            expr = Expr_optim( expr );
            //Expr_print( expr );
            ptr = buffer + 1 + Expr_write( expr, &buffer[1] );
            Add_stmt();
            if( stmt == 'R' && buffer[1] != 0 ) {
                Mod_val = TRUE;
            }
            continue;
        
        case 'l': /* label */
        case 'J': /* goto label */
            BufR();
            Add_stmt();
            continue;

        case 'F': /* if !expr goto label */
            ref = BufR();
            val = (ref>127) ? 3 : 2;
            Buf_expr();
            expr = Expr_read( &buffer[val] );
            //fprintf( stderr, "\n%c:\n", stmt );
            //Expr_print( expr );
            expr = Expr_optim( expr );
            //Expr_print( expr );
            ptr = buffer + val + Expr_write( expr, &buffer[val] );
            Add_stmt();
            continue;
        
        case 'i': /* case <const> */
        case 'b': /* case <str> */
            BufW();
            Add_stmt();
            continue;
        
        case 'r': /* case <const>::<const> */
            BufW();
            BufW();
            Add_stmt();
            continue;

        default: 
            printf("unrecognised IL statement: '%c'\n", stmt);
        }
        Put_buffer();
    }
    if( !no_opt ) Clean_stmts();
    Put_stmts();
    Put(0);
    Function_fin( Mod_name, Mod_val );
}

void Data_def( int mod_id );

void
Read_parsil(void)
{
    int token, val, ref;

    while( (token = Get()) != 0 )
    {
        switch( token ) {

        case ':':  /* external def */
            val = GetW();
            Data_def( val );
            break;

        case '(':  /* function def */
            ptr = buffer;
            *ptr++ = '(';
            Mod_name = BufW();
            Put_buffer();
            Const_index = 0;
            Extrn_index = 0;
            read_func();
            break;

        default: {
            printf("unrecognised parsil statement: '%c'\n", token);
            }
        }
    }
}

void Fetch_libs(void);
void Check_calls(void);

int
main( int argc, char *argv[] )
{
    Extra_lib_dir = (argc > 1) ? argv[1] : NULL;
        
    if( (IFile = open("functions", O_RDONLY)) < 0 ) {
        printf("cannot open parsil functions file\n");
        exit(1);
    }
    //close(IFile); IFile = 0;

    if( (OFile = open("cleanil", O_RDWR|O_CREAT|O_TRUNC, 0644)) < 0 ) {
        printf("cannot open output file cleanil\n" );
        exit(1);
    }
    // OFile = 1;

    Read_parsil(); /* phase 1 */
    Fetch_libs();  /* phase 2 */
    Check_calls(); /* phase 3 */
}
