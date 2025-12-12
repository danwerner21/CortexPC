
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "code.h"
#include "load.h"
#include "ti_opcodes.h"

#define TRUE  1
#define FALSE 0

int IFile;
int SFile;

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
GetR(void)
{
    int val;
    
    val = Get();
    if( val & 0x80 ) {
        val = (val & 0x7f) + (Get() << 7);
    }
    return val;
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

void
Error(char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

void
print_lstr(int str_id)
{
    char ch;
    int len, i;

    lseek(SFile, str_id, SEEK_SET);
    read(SFile, &ch, 1);
    len = ch;
    read(SFile, &ch, 1);
    len = (len << 8) + ch;
    printf("\"");
    for( i = 0; i < len - 1; ++i ) {
        read(SFile, &ch, 1);
        if( ch == 0 )
            printf("*$00");  
        else if( ch == 10 )
            printf("*n");  
        else
            printf("%c", ch);  
    }
    printf("\"");
}
    
/*====================*/

int Auto_top;
int Autovec_top;
int Temp_top;
int Framesizelocn;
int Arg_count;
extern int *Counter;

extern struct Symbol *R[];

#define WSD_AUTO 255
#define WSD_VEC  254
#define WSD_TEMP 253
#define ALIGN_EXTRN 2

struct Symbol*
Next_auto( int ref )
{
    struct Symbol *p;

    p = Sym_find( ref, TRUE );
    if( p->ref == ref )
        Error("Reference redefined");

    Arg_count++;
    Auto_top -= 2;

    p->ref = ref;
    if( Auto_top == 6 ) Auto_top = -2;
    p->value   = Auto_top;
    p->type    = AUTO_TYPE | WSD_AUTO;
    p->addrmod = AUTO_ADDRMOD;
    return( p );
}

struct Symbol *
Next_temp()
{
    struct Symbol *sym;
    
    if( Auto_top > 8 ) {
        Auto_top -= 2;
        return( &Sym_pseudo[Auto_top >> 1] );
    }
    
    Temp_top -= 2;
    sym = Sym_alloc();
    sym->value = Temp_top;
    sym->type = AUTO_TYPE | WSD_TEMP;
    sym->addrmod = AUTO_ADDRMOD;
    return( sym );
}

void
Next_autovec( struct Symbol *V, int size, int byte )
{
    struct Symbol *p;
    
    ++size; // 0 to n inclusive => n + 1 cells
    if( !byte ) size *= 2;
    if( size & 1 ) ++size; // round up to even
    Code( _MOV_, R[12], R[11] );
    Load_word( 0x22c ); // ai r12, xxxx
    Rload_word( ADDR_RELDESC, WSD_VEC, Autovec_top );
    Code( _MOV_, V, R[12] );
    Autovec_top += size;
}

void
Def_const( int ref, int const_val )
{
    struct Symbol *p;

    p = Sym_find( ref, TRUE );
    if( p->ref == ref )
        Error("Reference redefined");
    p->ref  = ref;
    p->type = CONST_TYPE;
    p->addrmod = INDEXED_MODE;
    p->const_value = (short)const_val;
}

void
Process_string( int str_id, struct Symbol *p, int type )
{
    unsigned char ch;
    int i, len, save;
    
    p->type = STRING_TYPE | (PTR_RELDESC<<8) | WSD_STRING;
    p->addrmod = INDEXED_MODE;
    p->str_id = str_id;

    // Only load regular strings, not twit strings
    if( type == '"' ) {
        lseek(SFile, str_id, SEEK_SET);
        read(SFile, &ch, 1);
        len = ch;
        read(SFile, &ch, 1);
        len = (len << 8) + ch;
        save = Set_loc( WSD_STRING );
        p->value = *Counter;
        for( i = 0; i < len; ++i ) {
            read(SFile, &ch, 1);
            Load_byte( ch );
        }
        if( *Counter & 1 ) *Counter += 1;
        Set_loc( save );
    }
}

void
Def_string( int ref, int str_id, int type )
{
    struct Symbol *p;

    p = Sym_find( ref, TRUE );
    if( p->ref == ref )
        Error("Reference redefined");
    p->ref  = ref;
    Process_string( str_id, p, type );
}

void
Def_extrn( int ref, int wsd )
{
    struct Symbol *p;

    p = Sym_find( ref, TRUE );
    if( p->ref == ref )
        Error("Reference redefined");
    p->ref  = ref;
    p->type = EXTRN_TYPE | (ADDR_RELDESC<<8) | wsd;
    p->addrmod = INDEXED_MODE;
}

void
Enable_gen(void)
{
    Code( _LIMI_, Constant(9), NULL );
}

void
Disable_gen(void)
{
    Code( _LIMI_, Constant(0), NULL );
}

int Wsd_next;

int
Wsd_index()
{
    return Wsd_next++;
}

int Prolog_done;
int Args_unbounded;
struct Symbol *R13_save_lcn = &Sym_pseudo[28];

/* Change all arguments to use R13 as base instead
 * of R11 (the frame base). Used for functions with
 * an unbounded argument count (i.e. using '?')
 */
void
Adjust_args(void)
{
    int i;
    struct Symbol *arg;
    
    for( i = 1; i <= Arg_count; ++i )
    {
        arg = Sym_find( i<<1, FALSE );
        if( !arg ) Error("argument not found");
        arg->addrmod = ARG_ADDRMOD;
    }
}

/* Determine which function prolog subroutine to call
 * based on whether the function contains an unbounded 
 * number of arguments (ie. 'questionable').
 * Generate the code to call the subroutine.
 * Save the code segment location at which to stuff the
 * stack increment via Epilog_gen.
 */
void
Prolog_gen(int nargs)
{
    /* Set up R11, new frame base. The argument
     * to "AI R11, ..." gets plugged by Epilog_gen.
     */
    Code( _STWP_, R[11], NULL );
    Load_word( _AI_ + 11 );      /* AI R11, ... */
    Framesizelocn = *Counter;
    Load_word( 0 );
    Args_unbounded = FALSE;

    /* If the function is 'questionable', save
     * R13 and replace by current WS pointer.
     * Adjust the arguments to use R13 as base.
     */
    if( nargs == 0377 ) {
        Code( _MOV_, R13_save_lcn, R[15] );
        Code( _STWP_, R[15], NULL );
        Adjust_args();
        Auto_top = 0;
        Args_unbounded = TRUE;
    }
}

/* Define 1oca1 symbo1s "..auto", "..vec", and "..temp".
 * Dump the remainder of the constant/address pool
 * Backplug the framesize location used in the prolog.
 */
void
Epilog_gen()
{
    int next_frame, frame_size;
    
    //Dump_pool();

    Auto_top = ( Auto_top > 0 ) ? 0 : -Auto_top;
//fprintf(stderr, "auto %d, vec %d, temp %d\n", Auto_top, Autovec_top, -Temp_top);
    frame_size = Auto_top + Autovec_top - Temp_top + 32 + 2;
    if( Args_unbounded ) frame_size += 2;
    next_frame = frame_size;
    Flush_load_bufs();
    
    Set_load_L_addr( WSD_CODE, Framesizelocn );
    Load_word( -next_frame ); /* stuff stack increment */
    *Counter -= 2; /* repair proper size of module; */
    
    Put_load_def( 'T', ABSOLUTE_RB, next_frame, "..auto" );
    next_frame -= Auto_top + Autovec_top;
    Put_load_def( 'T', ABSOLUTE_RB, next_frame, "..temp" );
    Put_load_def( 'T', ABSOLUTE_RB, next_frame, "..vec" );
    
    //Set_frame_size( frame_size ); /* save for stack bounding */
    Flush_load_bufs();
}

void
Return_gen()
{
    if( Args_unbounded ) {
        Code( _MOV_, R[15], R13_save_lcn );
    }
    Code( _RTWP_, NULL, NULL );
}

extern int Last_nargs;

void
Clean_tifunc(void)
{
    Auto_top = 22;
    Autovec_top = 0;
    Temp_top = 0;
    Arg_count = 0;
    Last_nargs = -1;
}

char Module_name[32];
int Code_rb;
int Data_rb;
int Strings_rb;
int In_twit;

#include "../Parse/il_opc.h"

void
Func_gen(void)
{
    int token, val, ref, i;
    int minarg, maxarg, segs, retval;
    char name[32];
    struct Symbol *sym;
    struct Expr *expr;
    static char E_rec[6] = "E\4\0\0\0\0";

    Prolog_done = FALSE;
    Set_loc( WSD_CODE );
    Flush_load_bufs();
    while( (token = Get()) != 0 ) {
        switch( token ) {
        
        /* ==== function header and declarations ==== */

        case 'a':  /* argument */
            sym = Next_auto( GetR() );
            break;

        case 'P':  /* prolog min, max, seg */
            Put_M_dir( EXTRN_RB, ALIGN_EXTRN, Module_name );
            Put_load_def( 'G', EXTRN_RB, 0, Module_name );
            minarg = Get(); maxarg = Get(); segs = Get();
            retval = FALSE;
//fprintf(stderr, "min %d, max %d, seg %d\n", minarg, maxarg, segs);
            Code_rb =  segs       & 0xf; if( Code_rb == 15 ) Code_rb = CODE_RB;
            Data_rb = (segs >> 4) & 0xf; if( Data_rb == 15 ) Data_rb = DATA_RB;
            Put_load_def( 'T', Data_rb,   0, "..data" );
            Put_load_ref( 't', WSD_DATA,   "..data" );
            Put_load_def( 'T', Code_rb,   0, "..code" );
            Put_load_ref( 't', WSD_CODE,   "..code" );
            Put_load_def( 'T', STRING_RB, 0, "..string" );
            Put_load_ref( 't', WSD_STRING, "..string" );
            Put_load_ref( 't', WSD_AUTO, "..auto" );
            Put_load_ref( 't', WSD_VEC,  "..vec"  );
            Put_load_ref( 't', WSD_TEMP, "..temp" );
            Set_loc( WSD_DATA );
            Rload_word( PTR_RELDESC, WSD_CODE, 0 );
            Set_loc( WSD_CODE );
            Prolog_gen( maxarg );
            Prolog_done = TRUE;
            break;

        case 'A':  /* auto */
            sym = Next_auto( GetR() );
            break;
        
        case 'V':  /* auto vector */
        case 'v':  /* auto string */
            sym = Next_auto( GetR() );
            val = GetW();
            Next_autovec(sym, val, token == 'v');
            break;

        case 'g':  /* external name */
            val = Wsd_index();
            Def_extrn( GetR(), val );
            Get_name( GetW(), name );
            Put_load_ref( 'g', val, name );
            break;
        
        case 'c':  /* declare constant REF */
            ref = GetR();
            val = GetW();
            Def_const( ref, val );
            break;

        case '\'': /* declare twit string */
        case '"':  /* declare string REF */
            ref = GetR();
            val = GetW();
            Def_string( ref, val, token );
            break;

        case 'H':  /* start of executable body */
            break;

        /* ===== statements ====== */

        case 'E': /* expression */
            expr = Build_tree();
            //Expr_print( expr );
            expr = Modify( expr );
            //Expr_print( expr );
            expr = Allocate_registers( expr, NULL );
            //Expr_print( expr );
            Arith_expr_gen( expr );
            break;
        
        case 'R': /* return */
            expr = Build_tree();
            if( expr ) {
                retval = TRUE;
                expr = Modify( expr );
                expr = Allocate_registers( expr, R[12] );
                Arith_expr_gen( expr );
            }
            Return_gen();
            break;
        
        case 'l':  /* label */
            sym = Sym_label( GetR() );
            Backplug_gen( sym );
            Last_nargs = -1;
            break;
            
        case 'J': /* goto label */
            sym = Sym_label( GetR() );
            Jump_gen( sym );
            Flush_load_bufs();
            break;

        case 'T': /* if  expr goto label */
        case 'F': /* if !expr goto label */
            sym = Sym_label( GetR() );
            expr = Build_tree();
            expr = Modify( expr );
            //Expr_print( expr );
            expr = Allocate_registers( expr, R[12] );
            //Expr_print( expr );
            Logical_expr_gen( expr, sym, (token == 'T') );
            break;
            
        case 'W': /* select word */        
        case 'S': /* select string */
            expr = Build_tree();
            expr = Modify( expr );
            expr = Allocate_registers( expr, R[12] );
            Arith_expr_gen( expr );
            Start_select();
            break;
        
        case 'r': /* case <const>::<const> */
            val = GetW();
            ref = GetW();
            Do_case( val, ref );
            break;
        
        case 'i': /* case <const> */
        case 'b': /* case <str> */
            val = GetW();
            Do_case( val, val );
            break;
                    
        case 'D': /* default */
            Def_case();
            break;
        
        case 'w': /* end select word */
            Wdselect_gen();
            break;

        case 's': /* end select string */
            Str_select();
            break;
            
        case 'e':
            Enable_gen();
            break;
            
        case 'd':
            Disable_gen();
            break;
                    
        case 't':
            In_twit = TRUE;
            expr = Build_tree();
            In_twit = FALSE;
            Twit_gen( expr );
            break;

        default: 
            fprintf( stderr, "unrecognised IL statement: '%c'\n", token);
        }
    }
    Set_loc( 0 );
    for( i=1; i<4; ++i) {
        ++Counter;
        Put_I_dir(i, *Counter);
    }
    Epilog_gen();
    E_rec[2] = minarg;  // minimum args
    E_rec[3] = maxarg;  // maximum args
    E_rec[4] = TRUE;    // needs arg count
    E_rec[5] = retval;  // returns value
    Put_load_dir( E_rec  );
}

void
process_cleanil(void)
{
    int token, val, ref;
    static int once = 0;

    while( (token = Get()) != 0 )
    {
        Clean();
        Sym_clean();
        Clean_tifunc();
        Wsd_next = 5;

        switch( token ) {

        case '(':  /* function def */
            Get_name( GetW(), Module_name );
            Func_gen();
            break;

        default: {
            fprintf( stderr, "unrecognised cleanil statement: '%c'\n", token);
            }
        }
    }
}

int
main()
{
    if( (IFile = open("functions", O_RDONLY)) < 0 ) {
        fprintf( stderr, "cannot open parsil functions file\n");
        exit(1);
    }
    if( (SFile = open("strings", O_RDONLY)) < 0 ) {
        fprintf( stderr, "cannot open strings file\n");
        exit(1);
    }
    OFile = 1;

    //Gen_start();
    process_cleanil();
    return(0);
}
