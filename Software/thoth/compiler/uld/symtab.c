
/* Symbol table routines */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "uld.h"

int
Length(char *s)
{
    int i = 0;

    while(s[i] != 0 ) ++i;
    return i;
}

int
Equal(char *s1, char *s2)
{
    int i = 0, ch;
    while( (ch = s1[i]) != 0 && ch == s2[i] ) ++i;
    return s1[i] == s2[i];
}

char *
Copy(char *src, char *dst)
{
    int i;
    for( i = 0; src[i] != 0; ++i ) dst[i] = src[i];
    dst[i] = 0;
    return( dst+i );
}

struct Symbol*
Sym_alloc( char *name )
{
    struct Symbol *sym;
    int size = sizeof(struct Symbol) + Length(name);
    
    sym = (struct Symbol*)malloc( size);
    memset(sym, 0, size);
    Copy(name, sym->name);
    return sym;
}

/* 64-bucket hash code, section 3.2, pg 12. The hash algorithm is not
    * specified; follow the V5/V6 DMR C compiler */
int
Hash( char* name )
{
    char *sp = name;
    int i = 0;
    
    while( *sp ) {
        i += *sp++;
    }
    return (i & 0x3f);
}

/* Symbol table lookup as per section 3.2, page 12+13 */
struct Symbol *SymHash[64];
struct Symbol **prevptr;
    
struct Symbol*
Sym_lookup( char *name, int insert )
{
    struct Symbol *sym;
    int idx;

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
    if( insert )
    sym = *prevptr = Sym_alloc( name );
    return sym;
}

struct Symbol*
Global_lookup( void *nm, int insert )
{
    char *name = (char*)nm;

    prevptr = &SymHash[Hash( name )];
    return( Sym_lookup( name, insert ) );
}

struct Symbol*
Local_lookup( void *nm )
{
    char *name = (char*)nm;

    prevptr = &(Module->local);
    return( Sym_lookup( name, TRUE ) );
}

void
Sym_rebase( int rbr, int base )
{
    int i;
    struct Symbol *sym, *lcl;
    
    for( i = 0; i < 64; ++i ) {
        sym = SymHash[i];
        while( sym ) {
            if( (sym->Rbr & 0x7) == rbr ) sym->value += base;
            if( (lcl = sym->local) ) {
                while( lcl ) {
                    if( (lcl->Rbr & 0x7) == rbr ) lcl->value += base;
                    lcl = lcl->link;
                }
            }
            sym = sym->link;
        }
    }
}

void
Sym_dump(void)
{
    int i;
    struct Symbol *sym, *lcl;
    
    if( flagv ) printf("Symbol table:\n");    
    for( i = 0; i < 64; ++i ) {
    //fprintf(stderr, "Chain %d:\n", i);
        sym = SymHash[i];
        while( sym ) {
            if( flagv ) fprintf(stderr, "sym '%s' %c, %d, %04x\n", sym->name, sym->Rbr&DEFINED?' ':'*', sym->Rbr&0x7, sym->value);
            if( !(sym->Rbr&DEFINED) ) {
                fprintf(stderr,  "warning: symbol %s undefined\n", sym->name );
            }
            if( (lcl = sym->local) ) {
                while( lcl ) {
                    if( flagv ) fprintf(stderr, "lcl '%s' %c, %d, %04x\n", lcl->name,  lcl->Rbr&DEFINED?' ':'*', lcl->Rbr&0x7, lcl->value);
                    if( !(lcl->Rbr&DEFINED) ) {
                        fprintf(stderr,  "warning: local %s undefined\n", lcl->name );
                    }
                    lcl = lcl->link;
                }
            }
            sym = sym->link;
        }
    }
}
