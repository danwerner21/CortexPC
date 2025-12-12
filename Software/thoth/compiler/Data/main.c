
/* Eh compiler Data module, as per chapter 3 of Bonkowski. */

#include <stdio.h>
#include <stdlib.h>

#include "mani.h"
#include "load.h"

char Module_name[33];
int  Externals_rb = 1;
int  Data_rb = 1;

int  Error_out;
char Load_E_dir[] = "E\0";

void
Get_rbs(void)
{
    int rbs;
    
    rbs = Get();
    Data_rb = Externals_rb = 1; //rbs & 0x7;
}


#define CONSTANT & 0x80

void
Process_simple()
{
    int c, wsd, id;
    char name[32];
    
    if( (c=Get()) CONSTANT ) Load_word( Get_constant(c) );
    else if( c=='g' )
    {
        Get_name( id=Get_id(), name );
        Put_load_ref( 'g', 255, name );
        Rload_word( PTR_RELDESC, 255, 0 );
    }
    else Error( "invalid character in simple external\n" );
    
    Flush_load_bufs();
    Put_I_dir( Externals_rb, EHADDR_FACTOR );
    Put_load_dir( Load_E_dir );
}

void
Process_string()
{
    int amt;
    
    Put_load_ref( 't', Last_data_ref=1, "X1" );
    Rload_word( PTR_RELDESC, 1, Counter[1]=0 );
    Copy_string( Get_constant( Get() ), Get_id() );
    Flush_load_bufs();
    
    Put_I_dir( Externals_rb, EHADDR_FACTOR );
#ifdef ALIGN_EXTERNAL_DATA
    Put_A_dir( Data_rb, ALIGN_EXTERNAL_DATA );
#endif
    Put_load_def( 'T', Data_rb, 0, "X1" );
    if( (amt = Counter[1]/MACHADDR_FACTOR) )
        Put_I_dir( Data_rb, amt );
    Put_A_dir( Data_rb, 2 );
    Put_load_dir( Load_E_dir );
}

void
Process_vector()
{
    int c, wsd, id, min_length;
    char name[32];
    
    min_length = TARGET_BPW * (Get_constant(Get()) + 1);
    
    Pointer_gen();
    Push_loc();
    
    while( 1 ) {
        c = Get();
        if( c CONSTANT ) {
            Load_word( Get_constant(c) );
            continue;
        }
        switch( c ) {
        
        case 'g': /* external symbol reference */
          {
            Get_name( id=Get_id(), name );
            wsd = Search_wsd( id, name );
            Rload_word( PTR_RELDESC, wsd, 0 );
            Flush_load_bufs();
            continue;
          }
        case '"': /* string initialization */
          {
            Pointer_gen(); /* to next data segment */
            Copy_string( 0, Get_id() );
            continue;
          }
        case '[': /* new level */
          {
            Pointer_gen(); /* to next data segment */
            Push_loc();
            continue;
          }
        case ']': /* to previous  level */
          {
            Pop_loc();
            continue;
          }
        default: ;
        }
        break;
    }
    if( c )
        Error("bad character in vector external");

    if( *Counter < min_length ) *Counter = min_length;
    Pop_loc();
    Flush_load_bufs();
    
    Put_I_dir( Externals_rb, EHADDR_FACTOR );
    for( wsd=1; wsd<=Last_data_ref; ++wsd) {
        ++Counter;
#ifdef ALIGN_EXTERNAL_DATA
        Put_A_dir( Data_rb, ALIGN_EXTERNAL_DATA );
#endif
        Put_load_def( 'T', Data_rb, 0, Create_name(wsd) );
        if( (min_length = *Counter/MACHADDR_FACTOR) )
            Put_I_dir( Data_rb, min_length );
    }
    Put_load_dir( Load_E_dir );
}

int main(int argc, char **argv)
{
    int c;
    
    //fprintf(stderr, ", Data");
    
    Initialize(argc, argv);
    while(1) {
        if( (c=Get()) == 0 ) exit(0);
        if( c!=':' && c!='"' && c!='[' )
            Error( "invalid input in Main\n" );
        
        Clean(); /* initializations for each module */
        
        Get_name( Get_id(), Module_name );
        Get_rbs();
        Put_M_dir( Externals_rb, ALIGN_EXTRN, Module_name );
        Put_load_def( 'G', Externals_rb, 0, Module_name );
        
        switch( c ) /* process the module */
        {
            case ':': Process_simple(); break;
            case '"': Process_string(); break;
            case '[': Process_vector(); break;
        }
    }
}

