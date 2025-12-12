#include "stdio.h"
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>

#include "tla.h"
#include "tokens.h"
#include "mani.h"
#include "load.h"

#define ALIGN       1
#define DC1         2
#define DC2         3
#define DC3         4
#define DC4         5
#define DS          6
#define END         7
#define ENT         8
#define EXT         9
#define FORMAL     10
#define FUNCTION   11
#define LOC        12
#define NARGS      13
#define REL        14
#define PTR        15
#define STACK      16
#define STR        17

struct Pseudo {
    char *name;
    int   val;
} cmds[] = {
    { ".align",    ALIGN },
    { ".dc1",      DC1 },
    { ".dc2",      DC2 },
    { ".dc3",      DC3 },
    { ".dc4",      DC4 },
    { ".ds",       DS },
    { ".ent",      ENT },
    { ".ext",      EXT },
    { ".end",      END },
    { ".formal",   FORMAL },
    { ".function", FUNCTION },
    { ".loc",      LOC },
    { ".nargs",    NARGS },
    { ".rel",      REL },
    { ".ptr",      PTR },
    { ".stack",    STACK },
    { ".str",      STR },
    NULL, 0
};

extern char Name[];

struct Module *M, *MList;

struct Module *
Mod_alloc()
{
    struct Module *m;
    int size = sizeof(struct Module);
    
    m = (struct Module*) malloc( size );
    memset(m, 0, size);
    m->next = M;
    MList = m;
    return m;
}

struct Module *
Mod_lookup( struct Symbol *entry )
{
    struct Module *m = MList;
    
    while( m && m->entry != entry ) m = m->next;
    return m;
}

void
Init_pseudo( void )
{
    struct Symbol *sym;
    struct Pseudo *ptr;
    
    M = Mod_alloc();
    for( ptr = cmds; ptr->name; ptr++ ) {
        Copy( ptr->name, Name );
        sym = Sym_lookup(TRUE);
        sym->type = PSEUDO;
        sym->val  = ptr->val;
    }
}

void
Eat_line( void )
{
    while( token && token != END_LINE ) Lex();
}

jmp_buf Err_jmp;

void
Asm_error( char *msg, int val )
{
    fprintf(stderr, msg, val);
    fprintf(stderr, ", line %d\n", (token == END_LINE ) ? Line_no - 1 : Line_no );
    
    Err_count++;
    Eat_line();

    longjmp( Err_jmp, 1 );
}

struct seg_wsd {
    char *name;
} stab[] = {
    { "..wsd0"   },
    { "..data"   },
    { "..code"   },
    { "..string" },
    { "..const"  },
    { "..wsd5"   },
    { "..wsd6"   },
    { "..wsd7"   }
};

void
Gen_pseudo( void )
{
    static char E_rec[6] = "E\4\0\0\0\0";
    int i, seg, sav, sw;

    switch( sw = Sym->val ) {

    case DC1: case DC2:
    case DC3: case DC4:
        Gen_DCn( Sym->val - DC1 );
        break;

    case ALIGN:
    case DS:
        Lex();
        i = Abs_expr();
        if( sw == ALIGN ) {
            sav = (*Counter % i);
            *Counter += sav ? i - sav : 0;
        } else {
            *Counter += 2 * i;
        }
        Flush_load_bufs();
        break;
        
    case ENT:
        if( Pass == 1 && M->active ) {
            Asm_error( "start of new module without ending previous module", 0);
        }
        i = 0;
        do {
            Lex();
            if( Pass == 1 ) {
                if( token != ID ) {
                    Asm_error( "Identifier expected", 0 );
                }
                if( Sym->type != UNDEFINED) {
                    fprintf(stderr, "'%s': ", Sym->name);
                    Asm_error( "duplicate definition", 0);
                }
                Sym->type = GLOBAL;
                Sym->val  = *Counter;
                Sym->seg  = Counter_level;
                if( M->entry == NULL ) {
                    M->entry = Sym;
                }
                M->seg_used |= (1 << Counter_level);
            }
            else {
                if( i == 0 ) {
                    M = Mod_lookup( Sym );
                    Put_M_dir( Sym->seg, ALIGN_EXTRN, Sym->name );
                }
                Put_load_def( 'G', Sym->seg, Sym->val, Sym->name );
                for( seg = 0; seg < 8; seg++ ) {
                    if( M->seg_used & (1 << seg) ) {
                        Put_load_def( 'T', seg, 0, stab[seg].name );
                        Put_load_ref( 't', seg,    stab[seg].name );
                    }
                }
            }
            Lex();
            i++;
        } while( token == COMMA );
        M->active = TRUE;
        break;

    case EXT:
        do {
            M->no_global = TRUE;
            Lex();
            M->no_global = FALSE;
            if( Pass == 1 ) {
                if( !M->active ) {
                    Asm_error( ".ext used outside module", 0 );
                }
                if( token != ID ) {
                    Asm_error( "Identifier expected", 0 );
                }
                if( Sym->type != UNDEFINED) {
                    fprintf(stderr, "'%s': ", Sym->name);
                    Asm_error( "duplicate definition", 0);
                }
                Sym->type = EXTERNAL;
                Sym->val  = 0;
                Sym->seg  = Next_Wsd();
            }
            else {
                Put_load_ref( 'g', Sym->seg, Sym->name );
            }
            Lex();
        } while( token == COMMA );
        break;
        
    case END:
        M->active = FALSE;
        if( Pass == 1 ) {
            M = Mod_alloc(); // prep for new module
        }
        else {
            Flush_load_bufs();
            sav = Set_loc( 0 );
            for( i = 0, seg = 1; i < 8; i++ ) {
                if( M->seg_used & seg  ) {
                    Put_I_dir(i, *Counter);
                }
                seg <<= 1;
                Counter++;
            }
            Set_loc( sav );
            E_rec[2] = M->min;
            E_rec[3] = M->max;
            E_rec[4] = M->nargs;
            E_rec[5] = M->function;
            Put_load_dir( E_rec  );
        }
        Clean();
        Lex();
        break;

    case FORMAL:
        Lex();
        if( Pass == 1 )
            Eat_line();
        else {
            M->min = Abs_expr();
            Lex(); // skip separator
            M->max = Abs_expr();
        }
        break;

    case FUNCTION:
        Lex();
        if( Pass == 2 ) M->function = TRUE;
        break;
        
    case NARGS:
        Lex();
        if( Pass == 2 ) M->nargs = TRUE;
        break;

    case PTR:
        if( Pass == 1 ) {
            Eat_line();
            *Counter += EHADDR_FACTOR;
        }
        else {
            Lex();
            Expr();
            Rload_word( PTR_RELDESC, Result.seg, Result.val );
        }
        break;

    case LOC:
        Lex();
        Expr();
        Set_loc( Result.val );
        *Counter = Result.seg;
        break;

    case REL:
        Lex();
        Expr();
        if( Result.val < 1 || Result.val > 7 || Result.seg != RB_ABS) {
            Asm_error( "Relocation segment must be between 1 and 7", 0 );
        }
        Set_loc( Result.val );
        if( Pass == 1) M->seg_used |= (1 << Result.val);
        break;
        
    case STACK:
        Lex();
        Eat_line();
        break;
        
    case STR:
        Lex();
        if( Pass == 1 ) {
            if( token != STRING )
                Asm_error("String expected\n", 0 );
        }
        else {
            for( i = 0, sav = 1; sav & 0xff ; ++i ) {
                sav = String[i] << 8;
                if( sav ) sav |= String[++i];
                Load_word( sav );
            }
        }
        Lex();
        break;
        
    default:
        Asm_error("interal error: bad pseudo token %d", sw);
        Eat_line();
    }
}

void
Parse_line( void )
{
    if( token != ID ) {
        fprintf(stderr, "res %d %d\n", Result.val, Result.seg);
        Asm_error( "Label or (pseudo-)opcode expected %d", token );
        Eat_line();
        return;
    }
    switch( Sym->type ) {

    case LABEL:
    case GLOBAL:
    case UNDEFINED:
        Lex();
        if( token != COLON ) {
            Asm_error( "':' expected after label definition", 0 );
            Eat_line();
            return;
        }
        if( Pass == 1 ) {
            if( Sym->type != LABEL ) {
                Sym->type = LABEL;
                Sym->val  = *Counter;
                Sym->seg  = Counter_level;
            }
            else {
                Asm_error( "Label redefined", 0 );
            }
        }
        Lex();
        break;

    case PSEUDO:
        Gen_pseudo();
        break;

    case OPCODE:
        Gen_opcode();
        break;

    default:
        Asm_error( "Bad identifier type %d", Sym->type );
        Eat_line();
    }
}

void
Parse( void )
{
    Lex();
    while( token ) {
        if( token == END_LINE ) {
            Lex();
            continue;
        }
        if( setjmp( Err_jmp ) ) continue;
        Parse_line();
    }
    //if( Pass == 1) Sym_dump();
}

