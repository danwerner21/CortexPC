
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "code.h"

/* pseudo symbols */
struct Symbol Sym_pseudo[] = {
    /*  0, R0  */ { NULL, 0, 0,  0, AUTO_TYPE, 0, 0, NULL, NULL },
    /*  1, R1  */ { NULL, 0, 0,  1, AUTO_TYPE, 0, 0, NULL, NULL },
    /*  2, R2  */ { NULL, 0, 0,  2, AUTO_TYPE, 0, 0, NULL, NULL },
    /*  3, R3  */ { NULL, 0, 0,  3, AUTO_TYPE, 0, 0, NULL, NULL },
    /*  4, R4  */ { NULL, 0, 0,  4, AUTO_TYPE, 0, 0, NULL, NULL },
    /*  5, R5  */ { NULL, 0, 0,  5, AUTO_TYPE, 0, 0, NULL, NULL },
    /*  6, R6  */ { NULL, 0, 0,  6, AUTO_TYPE, 0, 0, NULL, NULL },
    /*  7, R7  */ { NULL, 0, 0,  7, AUTO_TYPE, 0, 0, NULL, NULL },
    /*  8, R8  */ { NULL, 0, 0,  8, AUTO_TYPE, 0, 0, NULL, NULL },
    /*  9, R9  */ { NULL, 0, 0,  9, AUTO_TYPE, 0, 0, NULL, NULL },
    /* 10, R10 */ { NULL, 0, 0, 10, AUTO_TYPE, 0, 0, NULL, NULL },
    /* 11, R11 */ { NULL, 0, 0, 11, AUTO_TYPE, 0, 0, NULL, NULL },
    /* 12, R12 */ { NULL, 0, 0, 12, AUTO_TYPE, 0, 0, NULL, NULL },
    /* 13, R13 */ { NULL, 0, 0, 13, AUTO_TYPE, 0, 0, NULL, NULL },
    /* 14, R14 */ { NULL, 0, 0, 14, AUTO_TYPE, 0, 0, NULL, NULL },
    /* 15, R15 */ { NULL, 0, 0, 15, AUTO_TYPE, 0, 0, NULL, NULL },
    /* 16,     */ { NULL, 0, 32, 0x2b, AUTO_TYPE, 0, 0, NULL, NULL }, /* Nargs_locn */
    /* 17,     */ { NULL, 0, 24, 0x2b, AUTO_TYPE, 0, 0, NULL, NULL }, /* Retval_locn */
    /* 18, *R0 */ { NULL, 0, 0,  0x10, AUTO_TYPE, 0, 0, NULL, NULL },
    /* 19, *R1 */ { NULL, 0, 0,  0x11, AUTO_TYPE, 0, 0, NULL, NULL },
    /* 20, *R2 */ { NULL, 0, 0,  0x12, AUTO_TYPE, 0, 0, NULL, NULL },
    /* 21, *R3 */ { NULL, 0, 0,  0x13, AUTO_TYPE, 0, 0, NULL, NULL },
    /* 22, *R3 */ { NULL, 0, 0,  0x13, AUTO_TYPE, 0, 0, NULL, NULL },
    /* 23, 1   */ { NULL, 0, 0,  0x20, CONST_TYPE, 0, 1, NULL, NULL },
    /* 24, 2   */ { NULL, 0, 0,  0x20, CONST_TYPE, 0, 2, NULL, NULL },
    /* 25, 4   */ { NULL, 0, 0,  0x20, CONST_TYPE, 0, 4, NULL, NULL },
    /* 26, 8   */ { NULL, 0, 0,  0x20, CONST_TYPE, 0, 8, NULL, NULL },
    /* 27, 16  */ { NULL, 0, 0,  0x20, CONST_TYPE, 0, 16, NULL, NULL },
    /* 28,     */ { NULL, 0, 34, 0x2b, AUTO_TYPE, 0, 0, NULL, NULL }, /* R13_save_lcn */
    /* 29,     */ { NULL, 0, 32, 0x2d, AUTO_TYPE, 0, 0, NULL, NULL }, /* Nargs_locn in parent */
    /* 30,     */ { NULL, 0, 24, 0x2d, AUTO_TYPE, 0, 0, NULL, NULL }, /* Retval_locn in parent */
    /* end     */ { NULL, 0, 0,  0, 0, 0, 0, NULL, NULL }  
};

struct Symbol *Sym_hash[64];
struct Symbol *Constants;
struct Symbol *Sym_labels;

struct Symbol *
Sym_alloc( void )
{
    struct Symbol *p;

    p = (struct Symbol *)malloc(sizeof(struct Symbol));
    memset(p, 0, sizeof(struct Symbol));
    return(p);
}

struct Symbol *
Sym_find( int ref, int insert )
{
    struct Symbol *p, **prev;

    prev = &Sym_hash[ref>>1];
    p = *prev;
    while( p ) {
        if( p->ref == ref )
            return( p );
        prev = &p->link;
        p = p->link;
    }
    if( insert )
        *prev = p = Sym_alloc();
    return( p );
}

struct Symbol *
Constant( int cval )
{
    struct Symbol *p;
    
    p = Constants;
    while( p && p->const_value != cval )
        p = p->clink;
    if( p )
        return( p );

    p = Sym_alloc();
    p->const_value  = cval;
    p->type  = CONST_TYPE;
    p->addrmod = INDEXED_MODE;
    p->clink = Constants;
    Constants = p;
    return( p );
}

struct Symbol *
Sym_label( int ref )
{
    struct Symbol *p;
    
    p = Sym_find( ref, TRUE );
    if( p->ref == 0 )
    {
        p->ref = ref;
        p->type = LABEL_TYPE;
        p->addrmod = INDEXED_MODE;
    }
    return( p );
}

struct Symbol *
Int_label( void )
{
    struct Symbol *p;

    p = Sym_alloc();
    p->type = LABEL_TYPE;
    p->addrmod = INDEXED_MODE;
    p->link = Sym_labels;
    Sym_labels = p;
    return( p );
}

void
Sym_free(struct Symbol *p)
{
    struct Symbol *next;
    
    while( p )
    {
        next = p->link;
        if( !(p->clink) ) free( p );
        p = next;
    }
}

void
Con_free(struct Symbol *p)
{
    struct Symbol *next;
    
    while( p )
    {
        next = p->clink;
        free( p );
        p = next;
    }
}

void
Sym_clean(void)
{
    int i;
    
    for( i=0; i<64; ++i )
    {
        Sym_free( Sym_hash[i] );
        Sym_hash[i] = NULL;
    }
    Sym_free( Sym_labels ); Sym_labels = NULL;
    Con_free( Constants ); Constants = NULL;
}
