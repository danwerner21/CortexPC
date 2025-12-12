
/* Routines to manage (forward) jumps and backplugging the
 * destination labels when they become defined.
 */

#include <stdio.h>
 
#include <stdlib.h>
#include <string.h>
 
#include "code.h"
#include "load.h"
#include "ti_opcodes.h"
#include "../Parse/il_opc.h"

struct Defer {
    int defer_inst;
    struct Symbol *defer_symbol;
    int defer_loc;
    int defer_disp;
    struct Defer *defer_link;
    struct Defer *defer_qfwd;
    struct Defer *defer_qbck;
};

struct Defer Defer_anchor = {
    0, 0, 0, 0, NULL,
    &Defer_anchor, &Defer_anchor
};

struct Defer *Defer_queue_empty = &Defer_anchor;
#define Defer_queue_head (Defer_anchor.defer_qbck)
#define Defer_queue_tail (Defer_anchor.defer_qfwd)

/* Defer generation of the instruction 
 * until the symbol becomes defined.
 */
struct Defer *
Defer_gen( int instruction, struct Symbol *symbol, int length, int displacement )
{
    struct Defer *limit, *p, *i;
    int loc;
    
    p = (struct Defer *)malloc( sizeof(struct Defer) );
    p->defer_inst   = instruction;
    p->defer_symbol = symbol;
    p->defer_loc    = *Counter; /* remember where to backplug it */
    *Counter += length;         /* leave a gap for backplugging */
    Flush_load_bufs();
    
    p->defer_link = symbol->dlist; /* link it into the list of deferred */
    symbol->dlist = p;             /* instructions for this symbol */
    loc = p->defer_disp = displacement + p->defer_loc;

    limit = Defer_queue_empty; /* sort by displaced locations */
    for( i=Defer_queue_head; i != limit; i = i->defer_qbck )
                    if( i->defer_disp >= loc ) break;

    p->defer_qbck = i; /* link it in */
    p->defer_qfwd = i->defer_qfwd;
    i->defer_qfwd->defer_qbck = p;
    i->defer_qfwd = p;
    return( p );
}

int Backplugging;

/*  Backplug instructions which depend on the symbol just defined
 */
void
Backplug_gen( struct Symbol *symbol )
{
    struct Defer *dlist, *p;
    int *save;
    
    dlist = symbol->dlist; /* list of deferred instructions */
    symbol->value = *Counter; // /MACHADDR_FACTOR; PNR: seems wrong ??
    symbol->type |= SYMBOL_DEFINED;
    save = Counter;
    Backplugging = TRUE;
    
    while( (p=dlist) )
    {
        if( p->defer_qfwd ) /* still in queue, so remove it */
        {
            p->defer_qfwd->defer_qbck = p->defer_qbck;
            p->defer_qbck->defer_qfwd = p->defer_qfwd;
        }
        Counter = &p->defer_loc;
        Flush_load_bufs();
        Code( p->defer_inst, symbol, NULL );
        dlist = p->defer_link;
        free( p );
    }
    Backplugging = FALSE;
    Counter = save;
    Flush_load_bufs();
}


/* If the first entry in the defer queue needs no 
 * backplugging exit. Otherwise, backplug the
 * instruction and any others whose target symbol
 * is a non label and which falls within the
 * specified range. If skip == 1 or is not given
 * generate a jump around any code which might be
 * generated.
 */
void
Check_addressability( int skip, int range )
{
    struct Defer *defered, *previous;
    struct Symbol *symbol, *label;
    
    defered = Defer_queue_head;
    if( defered != Defer_queue_empty &&
        *Counter >= defered->defer_disp && !Backplugging )
    {
        if( skip ) Jump_gen( (label = Int_label()) );
        Backplug_symbol( defered->defer_symbol );
        previous = Defer_queue_head->defer_qbck;
        while( (defered = previous->defer_qfwd) != Defer_queue_empty )
        {
            if( *Counter < defered->defer_disp - range ) break;
            symbol = defered->defer_symbol;
            if( (symbol->type & TYPE_FIELD) == LABEL_TYPE &&
                *Counter < defered->defer_disp )
            {
                previous = defered;
                continue;
            }
            Backplug_symbol( symbol );  
        }
        if( skip ) Backplug_gen( label );
    }
}

/* Machine specific: for the TI 990
 * Define trampoline label to backplug before
 * a range violation occurs and defer a full
 * range branch at for the original target.
 */
void
Backplug_symbol( struct Symbol *symbol )
{
    struct Symbol *label = Int_label();
    label->dlist = symbol->dlist; /* copy defer queue pointer */
    symbol->dlist = NULL;
    Backplug_gen( label );
    Branch_gen( symbol );
}

/* Machine specific: for the TI 990
 * Emit a branch instruction if label defined.
 * Otherwise defer it with address always in range.
 */
void
Branch_gen( struct Symbol *label )
{
    struct Defer *p;
    
    if( !(label->type & SYMBOL_DEFINED) )
    {
        p = Defer_gen( _B_, label, 4, 240 );
        /* Now disconnect it so Check_addressability
         * will not find it in the range queue.
         */
        p->defer_qfwd->defer_qbck = p->defer_qbck;
        p->defer_qbck->defer_qfwd = p->defer_qfwd;
        p->defer_qfwd = NULL;
    }
    else Code( _B_, label, NULL );
} 

/* Machine specific: for the TI 990
 * Emit a jump instruction if label defined.
 * Otherwise defer it with max address range.
 */
void
Jump_gen( struct Symbol *label )
{
    struct Symbol *p;
    
    if( !(label->type & SYMBOL_DEFINED) )
    {
        Defer_gen( _JMP_, label, 2, 240 );
    }
    else if( (*Counter - label->value) < 240 )
        Code( _JMP_, label, NULL );
    else
        Code( _B_, label, NULL );
}


/* Machine specific: for the TI 990 not all
 * 6 jumps are defined; this is fixed by using a
 * reverse jump over the actual jump.
 * Emit a jump instruction if label defined.
 * Otherwise defer it with max address range.
 */
int Jmp_op[] = {
/*   ==      !=    >      >=     <=     <    */
    _JEQ_, _JNE_, _JGT_, _JLT_, _JGT_, _JLT_
};

void
Hop_gen( int op, struct Symbol *label )
{
    struct Symbol *p;
    int idx, far;
    
    far = ((label->type & SYMBOL_DEFINED) &&
           ((*Counter - label->value) > 240)) ? 1 : 0;

    if( op < IL_LAST ) {
        idx = (op - IL_EQU) >> 1;
        if( idx < 0 || idx > 5  )
            Error("bad hop gen instruction");
        op = Jmp_op[ idx ];
        if( idx == 3 || idx == 4 ) { // reverse jump over (far) jump
            Load_word( op | (1 + far));
            op = far ? _B_ : _JMP_;
        }
    }
    
    if( !(label->type & SYMBOL_DEFINED) )
    {
        Defer_gen( op, label, 2, 240 );
    }
    else {
        // far backward conditional jumps require special handling
        if( far && idx != 3 && idx != 4 ) {
            if( idx < 2) {
                op = idx ? _JEQ_ : _JNE_;
                Load_word( op | 2 );
            }
            else { // idx == 2 || idx == 5
                Load_word( op | 1 );
                Load_word( _JMP_ | 2) ;
            }
            op = _B_;
        }
        Code( op, label, NULL );
    }
}

