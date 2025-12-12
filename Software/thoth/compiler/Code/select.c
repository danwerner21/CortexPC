
#include <stdio.h>
#include <stdlib.h>

#include "code.h"
#include "load.h"
#include "ti_opcodes.h"
#include "../Parse/il_opc.h"

#define JUMPTABLE_LIMIT         8
#define STRAIGHTTEST_LIMIT      20
#define MINUS_INFINITY         -32768
#define PLUS_INFINITY           32767

struct selstmt {
    int number_of_cases;
    int first_case;
    int min_case;
    int max_case;
    int interval_case;
    struct Symbol *sel_label;
    struct Symbol *def_label;
    struct Symbol *end_label;
} Selects[10], *Select_ptr;

int Sel_index;

struct selcase {
    int minval;
    int maxval;
    struct Symbol *jmp_label;
} Cases[100], *Index[100];

int Case_index;

void Jumptable_gen(void);
void Straighttest_gen(void);

void
Pairtest_gen(void)
{
}

void
Binarytest_gen(void)
{
}

// Choose which code to generate for word select.
void
Wdselect_gen2(void)
{
    int min, max, ncases, size;
    
    ncases = Select_ptr->number_of_cases;
    min = Select_ptr->min_case;
    max = Select_ptr->max_case;
    size = max - min;
    
    if( min >= MINUS_INFINITY &&
        max <= PLUS_INFINITY &&
        size >= 0 && size < 2 * ncases )
    {
        if( ncases >= JUMPTABLE_LIMIT ||
            Select_ptr->interval_case )
        {
            Jumptable_gen();
        } else {
            Straighttest_gen();
        }
    }
    else if( Select_ptr->interval_case )
        Pairtest_gen();
    else if( ncases < STRAIGHTTEST_LIMIT )
        Straighttest_gen();
    else
        Binarytest_gen();
}

void
Start_select()
{
    struct selstmt *Sel;
    
    if( Sel_index == 10 ) {
        fprintf(stderr, "select statement nested too deeply\n");
        exit(1);
    }
    Select_ptr = Sel = &Selects[Sel_index++];

    Sel->end_label       = Int_label();
    Sel->sel_label       = Int_label();
    Sel->def_label       = Int_label();
    Sel->first_case      = Case_index;
    Sel->number_of_cases = 0;
    Sel->min_case        = PLUS_INFINITY;
    Sel->max_case        = MINUS_INFINITY;

    Jump_gen( Select_ptr->sel_label );
}

void
Do_case( int min, int max )
{
    struct selcase *Case;
    
    if( Case_index == 100 ) {
        fprintf(stderr, "too many case clauses\n");
        exit(1);
    }

    if( Select_ptr->number_of_cases > 0 || Select_ptr->def_label ) {
        Jump_gen( Select_ptr->end_label );  
    }
    Select_ptr->number_of_cases++;
    Select_ptr->interval_case = (min != max );
    if( min < Select_ptr->min_case ) Select_ptr->min_case = min;
    if( max > Select_ptr->max_case ) Select_ptr->max_case = max;

    Case = &Cases[Case_index++];
    Case->minval    = min;
    Case->maxval    = max;
    Backplug_gen( Case->jmp_label = Int_label() );
}

void
Def_case()
{
    if( Select_ptr->number_of_cases > 0 ) {
        Jump_gen( Select_ptr->end_label );  
    }
    if( Select_ptr->def_label->type & SYMBOL_DEFINED ) {
        fprintf(stderr, "duplicate default clause\n");
        return;
    }
    Backplug_gen( Select_ptr->def_label );
}

#define B_TYP  EXTRN_TYPE | (ADDR_RELDESC<<8) | SYMBOL_DEFINED
struct Symbol Sym_selstr = { NULL, 0, 0,  INDEXED_MODE, B_TYP | 254, 0, 0, NULL, NULL };
struct Symbol Sym_selbin = { NULL, 0, 0,  INDEXED_MODE, B_TYP | 253, 0, 0, NULL, NULL };
struct Symbol Sym_seltab = { NULL, 0, 0,  INDEXED_MODE, B_TYP | 252, 0, 0, NULL, NULL };

void
Str_select(void)
{
    struct Symbol sym;
    struct selcase *Case;
    int i;
    
    // terminate final case and set empty default if needed
    if( !(Select_ptr->def_label->type & SYMBOL_DEFINED) ) {
        Backplug_gen( Select_ptr->def_label );
    }
    Jump_gen( Select_ptr->end_label );

    Backplug_gen( Select_ptr->sel_label );
    // li r3, jmp table
    Load_word( 0x0203 );
    Rload_word(ADDR_RELDESC, WSD_DATA, Counter[WSD_DATA-WSD_CODE]);
    Branch_gen( &Sym_selstr );
    Put_load_ref( 'g', 254, "..selstr" );
    
    // generate case jump table
    Set_loc( WSD_DATA );
    Case = &Cases[Select_ptr->first_case];
    for( i = Select_ptr->first_case; i < Case_index; i++) {
        Process_string( Case->minval, &sym, '"' );
        Rload_word(ADDR_RELDESC, WSD_CODE, Case->jmp_label->value );
        Rload_word(ADDR_RELDESC, WSD_STRING, sym.value );
        Case++;
    }
    Rload_word(ADDR_RELDESC, WSD_CODE, Select_ptr->def_label->value );
    Load_word( 0 );
    Set_loc( WSD_CODE );

    Backplug_gen( Select_ptr->end_label );
    Case_index = Select_ptr->first_case;
    Select_ptr = &Selects[--Sel_index];
}

/* Sort cases in ascending order. Uses heapsort, which seems overkill
 * but it is what Bonkowski section 4.5 specifies.
 */
void
Case_sort(void)
{
    struct selcase t, *Case;
    int start, end, root, child, i;
    
    Case = &Cases[ Select_ptr->first_case ];
    end = Select_ptr->number_of_cases;
    start = end / 2;

    while( end > 1 ) {
        if( start > 0 ) {
            start = start - 1;
        } else {
            end = end - 1;
            t = Case[end]; Case[end] = Case[0]; Case[0] = t;
        }

        root = start;
        while( root * 2 + 1 < end )
        {
            child = root * 2 + 1;
            if( child + 1 < end && (Case[child].minval < Case[child + 1].minval) ) {
                child = child + 1;
            }
            if( Case[root].minval < Case[child].minval ) {
                t = Case[root]; Case[root] = Case[child]; Case[child] = t;
                root = child;
            } else {
                break;
            }
        }
    }
    end = Select_ptr->number_of_cases - 1;
    for( i = 0; i < end; i++ ) {
        if( Case[0].maxval >= Case[1].minval ) {
            fprintf(stderr, "Duplicate case value %d\n", Case[1].minval);
        }
        Case++;
    }
}

void
Straighttest_gen( void )
{
    struct selcase *Case;
    int i, last;

    // terminate final case and set empty default if needed
    if( !(Select_ptr->def_label->type & SYMBOL_DEFINED) ) {
        Backplug_gen( Select_ptr->def_label );
    }
    Jump_gen( Select_ptr->end_label );

    Backplug_gen( Select_ptr->sel_label );
    Case = &Cases[ Select_ptr->first_case ];
    last = Select_ptr->number_of_cases;
    for( i = 0; i < last; i++ ) {
        Code( _CI_, R[12], Constant( Case->minval ) );
        Hop_gen( IL_EQU, Case->jmp_label );
        Case++;
    }
    Jump_gen( Select_ptr->def_label );

    Backplug_gen( Select_ptr->end_label );
    Case_index = Select_ptr->first_case;
    Select_ptr = &Selects[--Sel_index];
}

void
Jumptable_gen(void)
{
    struct selcase *Case;
    struct Symbol *lbl;
    int i, min, max, tab;

    // terminate final case and set empty default if needed
    if( !(Select_ptr->def_label->type & SYMBOL_DEFINED) ) {
        Backplug_gen( Select_ptr->def_label );
    }
    Jump_gen( Select_ptr->end_label );

    // Generate the direct jump table
    min = Select_ptr->min_case;
    max = Select_ptr->max_case;
    Case = &Cases[ Select_ptr->first_case ];
    tab = *Counter;
    for( i = min; i <= max; i++ ) {
        lbl = Select_ptr->def_label;
        if( Case->minval == i ) {
            lbl = Case->jmp_label;
            Case++;
        }
        Rload_word(ADDR_RELDESC, WSD_CODE, lbl->value );
    }
    
    // Generate the jump selection code
    Backplug_gen( Select_ptr->sel_label );
    switch( Select_ptr->min_case ) {
        case -2: Code( _INCT_, R[12], 0 ); break;
        case -1: Code( _INC_,  R[12], 0 ); break;
        case  0: break;
        case  1: Code( _DEC_,  R[12], 0 ); break;
        case  2: Code( _DECT_, R[12], 0 ); break;
        default: Code( _AI_, R[12], Constant( -min ) );
    }
    Code( _CI_, R[12], Constant( max - min ) );
    Hop_gen( _JH_, Select_ptr->def_label );
    Code( _SLA_, Constant( 1 ), R[12] );
    Load_word( 0xc02c ); // MOV tab(R12),R0
    Rload_word(ADDR_RELDESC, WSD_CODE, tab);
    Load_word( 0x0450 ); // B *R0
    
    Backplug_gen( Select_ptr->end_label );
    Case_index = Select_ptr->first_case;
    Select_ptr = &Selects[--Sel_index];
}

void
Wdselect_gen( void )
{
    struct selcase *Case;
    int i;
    
    Case_sort();
    Straighttest_gen();
    //Jumptable_gen();
    
/*    Case = &Cases[ Select_ptr->first_case ];
    for( i = 0; i < Select_ptr->number_of_cases; i++ ) {
        fprintf(stderr, "case %d is for value %d\n", i, Case->minval);
        Case++;
    }
   
    Case_sort();
    
    Case = &Cases[ Select_ptr->first_case ];
    for( i = 0; i < Select_ptr->number_of_cases; i++ ) {
        fprintf(stderr, "case %d is for value %d\n", i, Case->minval);
        Case++;
    }
*/
}
