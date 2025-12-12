
#include <stdlib.h>
#include <string.h>

#include "parse.h"

struct Ref {
    int link;
    int refno;
    int key;
    int type;
};

int Reference;

#define MAXREF 200

struct Ref Reftab[MAXREF];

int RefHash[64]; /* identifier hash chain heads*/
int RefConst;    /* constants chain head */

int
Ref_alloc(int type)
{
    int refno;

    if( Reference == MAXREF ) {
        yyerror("ref table overflow");
        exit(1);
    }
    refno = Reference;
    Reftab[refno>>1].link  = 0;
    Reftab[refno>>1].refno = refno;
    Reftab[refno>>1].type  = type;
    Reference += 2;
    return (refno >> 1);
}

int *Prev_link;

int Ref_seek(int key)
{
    int idx;
    
    idx = *Prev_link;
    while( idx ) {
        if( Reftab[idx].key == key ) {
            return(idx);
        }
        else if( Reftab[idx].key > key ) {
            break;
        }
        Prev_link = &Reftab[idx].link;
        idx = Reftab[idx].link;
    }
    return(0);
}

int
Ref_iseek(int str_id, int type, int seek)
{
    int idx;
    
    idx = str_id & 0x3f;
    Prev_link = &RefHash[idx];
    idx = Ref_seek(str_id);
    if( idx == 0 ) {
        if( seek == MUST ) yyerror("identifier undefined");

        idx = Ref_alloc( type );
        Reftab[idx].key  = str_id;
        Reftab[idx].link = *Prev_link;
        *Prev_link = idx;
        idx = -idx;
    }
    else {
        if( seek == NEW ) yyerror("identifier redefined");
    }
    return ( idx << 1 );
}

int
Ref_cseek(int value)
{
    int idx;
    
    Prev_link = &RefConst;
    idx = Ref_seek(value);
    if( idx == 0 ) {
        idx = Ref_alloc( 'c' );
        Reftab[idx].key  = value;
        Reftab[idx].link = *Prev_link;
        *Prev_link = idx;
        idx = -idx;
    }
    return (idx << 1);
}

void
Ref_clear(void)
{
    struct Ref *ref, *link;
    int i;
    
    for( i = 0; i < 64; ++i ) {
        RefHash[i] = 0;
    }
    RefConst = 0;
    Reference = 2;
}

int
Ref_const(int val)
{
    int ref;

    ref = Ref_cseek(val);
    if( ref < 0 ) {
        ref = -ref;
        Put('c');
        PutR(ref);
        PutW(val);
    }
    return ref;
}

int
Ref_string(int str_id)
{
    int ref, type;
    extern int In_twit; // currently processing a twit param list
    
    type = In_twit ? '\'' : '"';
    ref = Ref_alloc(type) << 1;
    Put(type);
    PutR(ref);
    PutW(str_id);
    return ref;
}

int
Ref_label(void)
{
    int idx;
    
    idx = Ref_alloc('l') << 1;
    return idx;
}
#include <stdio.h>

int
Ref_ident(int name_id, int type, int seek)
{
    int ref;
    extern int In_twit; // currently processing a twit param list

    if( In_twit && seek == MUST ) {
        seek = MAY;
        type = 'l';
    }
    ref = Ref_iseek(name_id, type, seek);
    if( ref < 0 ) {
        ref = -ref;
        if( type != 'l' ) {
            Put(type);
            PutR(ref);
            if( type == 'g' ) PutW(name_id);
        }
    }    
    return ref;
}

#include <stdio.h>

void
Ref_print(void)
{
    struct Ref *ref;
    int i;
    
    printf("\nReference table: %d\n", Reference);
    ref = &Reftab[1];
    for( i = 1; i < Reference/2; i++) {
        printf("%c %3d ", ref->refno < 0 ? '*' : ' ', ref->refno);
        printf("'%c' %d\n", ref->type, ref->key);
        ref++;
    }
}
