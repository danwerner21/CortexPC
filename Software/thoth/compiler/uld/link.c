
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "uld.h"

unsigned char buf[40];

int Rd[8];
int Rbr[8];

unsigned char Wr[32];

struct Symbol *Wsd[256];
struct Symbol *Module;

void
Pass_one( void )
{
    int check, i, cmd, len, m;
    struct Symbol *sym;

    while( 1 )
    {
        /* Read record */
        buf[0] = cmd = Get();
        buf[1] = len = Get();
        for( i = 0; i < len; ++i)
            buf[2+i] = Get();
        buf[len+2] = Get();
        
        check = 0;
        for( i = 0; i < len + 3; ++i ) {
            check ^= buf[i];
        }
        if( check != 0 ) {
            fprintf(stderr, "Bad checksum %x\n", check);
            exit(1);
        }
        buf[len+2] = 0;
        
        /* Process record */
        switch( cmd ) {

        case 'M':  /* Start module */
            m = buf[3] - 1;
            Rbr[buf[2]] = (Rbr[buf[2]] + m) & ~m;
            Module = Global_lookup( &buf[4], TRUE );
            break;

        case 'G':  /* Define global symbol */
            sym = Global_lookup( &buf[5], TRUE );
            if( sym->Rbr & DEFINED) {
                fprintf(stderr, "global symbol redefined %s\n", &buf[5]);
                break;
            }
            sym->value = Rbr[buf[2]] + (buf[3]<<8) + buf[4];
            sym->Rbr = buf[2] | DEFINED;
            break;

        case 'g':  /* Reference global symbol */
            Global_lookup( &buf[3], TRUE );
            break;

        case 'T':  /* Define local symbol */
            sym = Local_lookup( &buf[5] );
            if( sym->Rbr & DEFINED) {
                fprintf(stderr, "local symbol redefined %s\n", &buf[5]);
                break;
            }
            sym->value = Rbr[buf[2]] + (buf[3]<<8) + buf[4];
            sym->Rbr = buf[2] | DEFINED;
            break;

        case 't':  /* Reference local symbol */
            Local_lookup( &buf[3] );
            break;
        
        case 'L':  /* Data */
            break;

        case 'O':  /* Relocation */
            break;

        case 'A':  /* Align base */
            m = buf[3] - 1;
            Rbr[buf[2]] = (Rbr[buf[2]] + m) & ~m;
            break;

        case 'I':  /* Increment base */
            Rbr[buf[2]] += (buf[3]<<8)+buf[4];
            break;
            
        case 'E':  /* End module */
            Module = NULL;
            break;
        
        case 0:
            return;

        default: {
            fprintf(stderr, "unrecognised load statement: '%c'\n", cmd);
            exit(1);
            }
        }
    }
}

/* Quick hack to relocate for TI990 */
void
Reloc( int index, int Rd, int ref )
{
    int val;
    
    //fprintf(stderr, "reloc byte %d, symbol '%s', base %04x\n", index, Wsd[ref]->name, Wsd[ref]->value);
    val  = (Wr[index-1] << 8) + Wr[index];
    val += Wsd[ref]->value;
    if( Rd == 3 ) val >>= 1; /* 3 == PTR_RELDESC */
    Wr[index-1] = val >> 8;
    Wr[index]   = val & 0xff;
}

void Output()
{
    int addr, len, i;
    unsigned char *mem;
    
    addr = (Wr[0] << 8) + Wr[1];
    len  = Wr[2] + 3;
    mem  = &Memory[addr];
    
    for( i = 3; i < len; ++i ) {
        *mem++ = Wr[i];
    }
}

void
Pass_two( void )
{
    int i, cmd, len, m;
    struct Symbol *sym;

    while( 1 )
    {
        /* Read record, trust checksum */
        buf[0] = cmd = Get();
        buf[1] = len = Get();
        for( i = 0; i < len; ++i)
            buf[2+i] = Get();
        buf[len+2] = Get();
        buf[len+2] = 0;
        
        /* Process record */
        switch( cmd ) {

        case 'M':  /* Start module */
            Module = Global_lookup( &buf[4], TRUE );
            memset(Wsd, 0, sizeof(Wsd) );
            Wsd[0] = Module;
            break;

        case 'G':  /* Define symbol */
        case 'T':
            break;
        
        case 'g':  /* Reference global symbol */
            sym = Global_lookup( &buf[3], TRUE );
            Wsd[buf[2]] = sym;
            break;

        case 't': /* Reference local symbol */
            sym = Local_lookup( &buf[3] );
            Wsd[buf[2]] = sym;
            break;
        
        case 'L':  /* Load data */
            len = buf[1] + 2;
            if( len > 34 ) {
                fprintf(stderr, "L directive length too large\n");
                exit(1);
            }
            for( i = 2; i < len; ++i ) Wr[i-2] = buf[i];
            break;

        case 'O':  /* Relocate and output data */
            len = buf[1] + 2;
            for( i = 2; i < len; i += 2 ) {
                Reloc( buf[i]>>3, buf[i]&0x7, buf[i+1] );
            }
            Output();
            break;

        case 'A':  /* Align base */
            break;

        case 'I':  /* Increment base */
            break;
            
        case 'E':  /* End module */
            break;
                
        case 0:
            return;
                
        default: {
            fprintf(stderr, "unrecognised load statement: '%c'\n", cmd);
            }
        }
    }
}

void
Rbr_rebase( void )
{
    /* place Rbr's sequentially in memory */
    Sym_rebase( 1, 0x0100 ); Rbr[1] += 0x0100;
    Sym_rebase( 2, Rbr[1] ); Rbr[2] += Rbr[1];
    Sym_rebase( 3, Rbr[2] ); Rbr[3] += Rbr[2];
    Sym_rebase( 4, Rbr[3] ); Rbr[4] += Rbr[3];
    Sym_rebase( 5, Rbr[4] ); Rbr[5] += Rbr[4];
    Sym_rebase( 6, Rbr[5] ); Rbr[6] += Rbr[5];
    Sym_rebase( 7, Rbr[6] ); Rbr[7] += Rbr[6];
}

