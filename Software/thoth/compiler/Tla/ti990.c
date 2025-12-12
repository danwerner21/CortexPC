#include "stdio.h"

#include "tla.h"
#include "tokens.h"
#include "load.h"
#include "mani.h"

struct Opcodes {
    char   *name;	
    int    opcode;
    int    opclass;
};

/*
 * Opcodes and pseudo-ops
 */
struct Opcodes opc[] = {

    /* TI990 machine ops */
    { "a",      0xa000, 1 },
    { "ab",     0xb000, 1 },
    { "abs",    0x0740, 3 },
    { "ai",     0x0220, 5 },
    { "andi",   0x0240, 5 },
    { "b",      0x0440, 3 },
    { "bl",     0x0680, 3 },
    { "blwp",   0x0400, 3 },
    { "c",      0x8000, 1 },
    { "cb",     0x9000, 1 },
    { "ci",     0x0280, 5 },
    { "ckof",   0x03c0, 9 },
    { "ckon",   0x03a0, 9 },
    { "clr",    0x04c0, 3 },
    { "coc",    0x2000, 2 },
    { "czc",    0x2400, 2 },
    { "dec",    0x0600, 3 },
    { "dect",   0x0640, 3 },
    { "div",    0x3c00, 2 },
    { "divs",   0x0180, 3 },
    { "idle",   0x0340, 9 },
    { "inc",    0x0580, 3 },
    { "inct",   0x05c0, 3 },
    { "inv",    0x0540, 3 },
    { "jeq",    0x1300, 6 },
    { "jgt",    0x1500, 6 },
    { "jh",     0x1b00, 6 },
    { "jhe",    0x1400, 6 },
    { "jl",     0x1a00, 6 },
    { "jle",    0x1200, 6 },
    { "jlt",    0x1100, 6 },
    { "jmp",    0x1000, 6 },
    { "jnc",    0x1700, 6 },
    { "jne",    0x1600, 6 },
    { "jno",    0x1900, 6 },
    { "joc",    0x1800, 6 },
    { "jop",    0x1c00, 6 },
    { "ldcr",   0x3000, 2 },
    { "ldd",    0x07c0, 3 },
    { "lds",    0x0780, 3 },
    { "li",     0x0200, 5 },
    { "limi",   0x0300, 7 },
    { "lrex",   0x03e0, 9 },
    { "lst",    0x0080, 4 },
    { "lwp",    0x0090, 4 },
    { "lwpi",   0x02e0, 7 },
    { "mov",    0xc000, 1 },
    { "movb",   0xd000, 1 },
    { "mpy",    0x3800, 2 },
    { "mpys",   0x01e0, 3 },
    { "neg",    0x0500, 3 },
    { "nop",    0x1000, 9 },
    { "ori",    0x0260, 5 },
    { "rset",   0x0360, 9 },
    { "rtwp",   0x0380, 9 },
    { "s",      0x6000, 1 },
    { "sb",     0x7000, 1 },
    { "sbo",    0x1d00, 10 },
    { "sbz",    0x1e00, 10 },
    { "seto",   0x0700, 3 },
    { "sla",    0x0a00, 8 },
    { "soc",    0xe000, 1 },
    { "socb",   0xf000, 1 },
    { "sra",    0x0800, 8 },
    { "src",    0x0b00, 8 },
    { "srl",    0x0900, 8 },
    { "stcr",   0x3400, 2 },
    { "stst",   0x02c0, 4 },
    { "stwp",   0x02a0, 4 },
    { "swpb",   0x06c0, 3 },
    { "szc",    0x4000, 1 },
    { "szcb",   0x5000, 1 },
    { "tb",     0x1f00, 10 },
    { "x",      0x0480, 3 },
    { "xop",    0x2c00, 2 },
    { "xor",    0x2800, 2 },
    { NULL, 0 }
};

extern char Name[];

void
Init_opcodes( void )
{
    struct Symbol  *sym;
    struct Opcodes *ptr;
    int i;
    
    /* Load the opcodes proper */
    for( ptr = opc; ptr->name; ptr++ ) {
        Copy( ptr->name, Name );
        sym = Sym_lookup(TRUE);
        sym->type = OPCODE;
        sym->val  = ptr->opcode;
        sym->seg  = ptr->opclass;
    }
    
    /* Load the register names */
    for( i = 0; i < 16; i++ ) {
        sprintf(Name, "r%d", i);
        sym = Sym_lookup(TRUE);
        sym->type = LABEL;
        sym->val  = i;
        sym->seg  = RB_ABS;
    }
}

static void
Skip_token( int need )
{
    if( token != need ) {
        Asm_error( "Bad operand syntax", 0 );
    }
    else
        Lex();
}

static int
Count_At( void )
{
    int count = 0;

    while( token != END_LINE ) {
        if( token == AT_SIGN ) count += 2;
        Lex();
    }
    return( count );
}

/* Evaluate an expression which is expected to return
   a 4-bit absolute value */
static int
Regexpr( void )
{
    int ret, r;

    Expr();
    if( Result.seg != RB_ABS || (Result.val & ~0xf) ) {
        Asm_error( "Register out of range", 0 );
    }
    return( Result.val );
}

#define WRI	    0x10   /* register indirect: [R] */
#define WRIA 	0x30   /* register indirect, autoincrement: [R]+ */
#define MD	    0x20   /* memory direct: @M or indexed @M[R] */
#define MASK	0x30   /* mask for mode bits */

/* R, [R], [R]+, @expr, @expr[R] */
static int
Full_reg( void )
{
    struct Value save;
    int bits;

    if( token == OPEN_BRACKET ) {
        Lex();
        bits = Regexpr();
        Skip_token( CLOSE_BRACKET );
        if( token == PLUS ) {
            bits |= WRIA;
            Lex();
        }
        else
            bits |= WRI;
        return( bits );
    }
    
    if( token == AT_SIGN ) {
        Lex();
        Expr();
        bits = MD;
        if( token == OPEN_BRACKET ) {
            Lex();
            save = Result;
            bits |= Regexpr();
            Result = save;
            Skip_token( CLOSE_BRACKET );
        }
        return( bits );
    }
    
    bits = Regexpr();
    return( bits );
}

static void
Load_oper( struct Value addr )
{
    if( addr.seg == RB_ABS )
        Load_word( addr.val );
    else
        Rload_word( ADDR_RELDESC, addr.seg, addr.val );
}

/* format 1: Ts, S, Td, D */
void
Opcode_1( int opc )
{
    struct Value first;
    
    if( Pass == 1 ) {
        *Counter += 2 + Count_At();
    }
    else {
        opc |= Full_reg();
        first = Result;
        Skip_token( COMMA );
        opc |= Full_reg() << 6;

        Load_word( opc );
        if( (opc & MASK) == MD )
            Load_oper( first );
        if( (opc & (MASK<<6)) == (MD<<6) )
            Load_oper( Result );
    }
}

/* format 1: Ts, S, Td, D */
void
Opcode_2( int opc )
{
    struct Value first;
    
    if( Pass == 1 ) {
        *Counter += 2 + Count_At();
    }
    else {
        opc |= Full_reg();
        first = Result;
        Skip_token( COMMA );
        opc |= Regexpr() << 6;

        Load_word( opc );
        if( (opc & MASK) == MD )
            Load_oper( first );
    }
}

/* format 3: Td, D */
void
Opcode_3( int opc )
{
    if( Pass == 1 ) {
        *Counter +=  2 + Count_At();
    }
    else { 
        opc |= Full_reg();
        Load_word( opc );
        if( (opc & MASK) == MD )
            Load_oper( Result );
    }
}

/* format 4: R */
void
Opcode_4( int opc )
{
    if( Pass == 1 ) {
        *Counter += 2;
        Eat_line();
    }
    else { 
        opc |= Regexpr();
        Load_word( opc );
    }
}

/* format 5: R, immediate */
void
Opcode_5( int opc )
{
    if( Pass == 1 ) {
        *Counter += 4;
        Eat_line();
    }
    else { 
        opc |= Regexpr();
        Skip_token( COMMA );
        Expr();
        Load_word( opc );
        Load_oper( Result );
    }
}

/* format 6: relational jumps */
void
Opcode_6( int opc )
{
    int offs, r;

    if( Pass == 1 ) {
        *Counter += 2;
        Eat_line();
    }
    else {
        Expr();
        if( Result.seg != Counter_level ) {
            Asm_error( "Jump target must be local", 0 );
        }
        offs = ( Result.val - (*Counter + 2) ) >> 1;
        if (offs < -128 || offs > 127) {
            Asm_error( "Jump target out of range", 0 );
        }
        opc |= (offs & 0xff);
        Load_word( opc );
    }
}

/* format 7: immediate */
void
Opcode_7( int opc )
{
    if( Pass == 1 ) {
        *Counter += 4;
        Eat_line();
    }
    else { 
        Expr();
        Load_word( opc );
        Load_oper( Result );
    }
}

/* format 8: count, R */
void
Opcode_8( int opc )
{
    if( Pass == 1 ) {
        *Counter += 2;
        Eat_line();
    }
    else {
        opc |= Regexpr();
        Skip_token( COMMA );
        opc |= Regexpr() << 4;
        Load_word( opc );
    }
}

/* format 9: none */
void
Opcode_9( int opc )
{
    if( Pass == 1 ) {
        *Counter += 2;
        Eat_line();
    }
    else
        Load_word( opc );
}

/* format 10: bit CRU */
void
Opcode_10( int opc )
{
    int bit;

    if( Pass == 1 ) {
        *Counter += 2;
        Eat_line();
    }
    else {
        Expr();
        if( Result.seg != RB_ABS || (Result.val & ~0xff) ) {
            Asm_error( "CRU bit must be absolute value 0 - 255", 0 );
        }
        opc |= (Result.val & 0xff);
        Load_word( opc );
    }
}

void
Gen_opcode( void )
{
    int opcode = Sym->val;
    int optype = Sym->seg;

    //if( Pass == 2 ) fprintf(stderr, "%1d %04x %s\n", Counter_level, *Counter, Sym->name );
    Lex();
    switch( optype ) {
    case  1: Opcode_1(  opcode ); return;
    case  2: Opcode_2(  opcode ); return;
    case  3: Opcode_3(  opcode ); return;
    case  4: Opcode_4(  opcode ); return;
    case  5: Opcode_5(  opcode ); return;
    case  6: Opcode_6(  opcode ); return;
    case  7: Opcode_7(  opcode ); return;
    case  8: Opcode_8(  opcode ); return;
    case  9: Opcode_9(  opcode ); return;
    case 10: Opcode_10( opcode ); return;

    default:
        if( Pass == 1) {
            *Counter += 2;
            fprintf(stderr, "token %5d, %10s, %3d, 0x%04x, %2d\n", token, Sym->name, Sym->type, Sym->val, Sym->seg );
        }
        else
            Load_word( opcode );
        Eat_line();
    }
}

void
Gen_DCn( int n )
{
    if( Pass == 1 ) {
        if( n > 0 )
            Asm_error( "Only DC1 supported on this architecture", 0 );
        *Counter += 2;
        Eat_line();
    }
    else {
        switch( n ) {

        case 0:
            Lex();
            Expr();
            Load_oper( Result );
            break;

        case 1:
        case 2:
        case 3:
            Eat_line();
            break;
        }

    }
}
