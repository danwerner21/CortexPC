
/* Symbol table routines */

#include <stdio.h>

#include "ale.h"

/* Keep the Symbols in a linear array, overlayed
 * with a 64-bucket hash index
 */
struct Symbol  Symbols[MAXSYM];
struct Symbol *SymHash[64];

int   Next_symbol;
char  Names[MAXSYM * 8];
char *Next_name = Names;

struct Symbol*
Sym_alloc(char *name)
{
    struct Symbol *sym;
    
    sym = &Symbols[Next_symbol++];
    sym->link = NULL;
    sym->module = NULL;
    sym->name = Next_name;
    Next_name = Copy(name, Next_name) + 1;
    return sym;
}

/* Use the same hash setup as for the other tool chain
 * components: 64 hash buckets, additive hash.
 */
int
Hash(char* name)
{
    char *sp = name;
    int i = 0;
    
    while( *sp ) {
        i += *sp++;
    }
    return (i & 0x3f);
}

/* Look up a symbo by name
 */
struct Symbol*
Sym_lookup( unsigned char *nm )
{
    struct Symbol *sym, **prevptr;
    int idx;
    char *name = (char *)nm;

    prevptr = &SymHash[ Hash( name ) ];
    sym = *prevptr;

    while( sym ) {
        if( name[0] == sym->name[0] && name[1] == sym->name[1]
            && Equal(name, sym->name) )
        {
            return sym;
        }
        prevptr = &sym->link;
        sym = sym->link;
    }
    sym = *prevptr = Sym_alloc( name );
    return sym;
}

/* Look up a symbol by index
 */
struct Symbol*
Sym_byindex( int index )
{
    return( &Symbols[ index ] );
}

/* Find the index of a symbol
 */
int
Sym_index( struct Symbol *sym )
{
    return( sym - Symbols );
}

