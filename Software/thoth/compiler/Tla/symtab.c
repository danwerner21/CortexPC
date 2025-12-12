
/* Symbol table routines */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "tla.h"

char Name[33];

void
Get_ident(void)
{
    int i, ch;
    
    memset(Name, 0, 32);
    for(i=0; i<=32; ++i) {
        ch = Get_ind();
        if( Ctab[ch] & (L|D) )
            Name[i] = ch;
        else
            break;
    }
    if( i == 0 )  Error("identifier missing");
    if( i == 33 ) Error("name too long");
    In_pushed_back = TRUE;
}

struct Symbol*
Sym_alloc(char *name)
{
    struct Symbol *sym;
    int size = sizeof(struct Symbol) + Length(name);
    
    sym = (struct Symbol*)malloc( size);
    memset(sym, 0, size);
    Copy(name, sym->name);
    sym->type = UNDEFINED;
    return sym;
}

/* 64-bucket hash code, section 3.2, pg 12. The hash algorithm is not
    * specified; follow the V5/V6 DMR C compiler */
int
Hash(void)
{
    char *sp = Name;
    int i = 0;
    
    while( *sp ) {
        i += *sp++;
    }
    return (i & 0x3f);
}

/* Symbol table lookup as per section 3.2, page 12+13 */
struct Symbol *SymHash[64];

struct Symbol*
Sym_lookup(int insert)
{
    struct Symbol *sym, **prevptr;
    int idx;
    
    if( M->active ) {
        for( sym = M->local; sym; sym = sym->link ) {
            if( Equal( Name, sym->name ) ) return sym;
        }
        // .ext definitions must be local
        if( M->no_global && insert ) {
            sym = Sym_alloc(Name);
            sym->link = M->local;
            M->local = sym;
            return sym;
        }
    }
    
    idx = Hash();
    prevptr = &SymHash[idx];
    sym = *prevptr;
    while( sym ) {
        if( Name[0] == sym->name[0] && Name[1] == sym->name[1]
            && Equal(Name, sym->name) )
        {
            *prevptr = sym->link;
            sym->link = SymHash[idx];
            SymHash[idx] = sym;
            return sym;
        }
        prevptr = &sym->link;
        sym = sym->link;
    }

    if( insert ) {
        sym = Sym_alloc(Name);
        if( M->active ) {
            sym->link = M->local;
            M->local = sym;
        } else {
            sym->link = SymHash[idx];
            SymHash[idx] =  sym;
        }
    }
    return sym;
}

void Sym_dump(void)
{
    int i;
    struct Symbol *sym;
    
    for( i = 0; i < 64; ++i ) {
    //fprintf(stderr, "Chain %d:\n", i);
        sym = SymHash[i];
        while( sym ) {
        if( (sym->type == LABEL || sym->type == EXTERNAL || sym->type == GLOBAL ) && sym->seg != RB_ABS )
            fprintf(stderr, "sym '%s', %d, %d, %d\n", sym->name, sym->type, sym->val, sym->seg);
            sym = sym->link;
        }
    }
    fprintf(stderr, "---\n");
}

