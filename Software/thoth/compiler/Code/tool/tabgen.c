
/* Symbol table routines */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "../../Parse/il_opc.h"

#define TRUE  1
#define FALSE 0

#define INSERT 0
#define IL_OPC 1
#define TI_OPC 2
#define TEST   3
#define OPER   4
#define ACTION 5
#define ANY    6

#define D 0x10
#define L 0x20

char Ctab[128] = {
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      L,      0,
    D|0,    D|1,    D|2,    D|3,    D|4,    D|5,    D|6,    D|7,
    D|8,    D|9,    0,      0,      0,      0,      0,      0,
    0,      D|L|10, D|L|11, D|L|12, D|L|13, D|L|14, D|L|15, L,
    L,      L,      L,      L,      L,      L,      L,      L,
    L,      L,      L,      L,      L,      L,      L,      L,
    L,      L,      L,      0,      0,      0,      0,      L,
    0,      D|L|10, D|L|11, D|L|12, D|L|13, D|L|14, D|L|15, L,
    L,      L,      L,      L,      L,      L,      L,      L,
    L,      L,      L,      L,      L,      L,      L,      L,
    L,      L,      L,      0,      0,      0,      0,      0
};

/* lib.c */
int  Length(char *s);
int  Equal(char *s1, char *s2);
void Copy(char *src, char *dst);

char Name[33];
int IFile;

int In_pushed_back;
int Cache;
int Line_no = 1;

int
Get_ind(void)
{
    char ch;
    int skip = FALSE;

    if( In_pushed_back ) {
        In_pushed_back = 0;
            return Cache;
    }

    do {
        if( read(IFile, &ch, 1) <= 0 )
            return (Cache = 0);
        if( ch == '#' ) skip = TRUE;
        if( ch == '\n' ) { Line_no++; skip = FALSE; }
    } while( skip || ch==' ' || ch== '\t' || ch=='\n' );

    return (Cache = ch);
}

void
Skip( int ch )
{
    int c;

    if( (c = Get_ind()) != ch ) {
        printf("'%c' not '%c' expected after '%s' on line %d\n", ch, c, Name, Line_no);
        exit(1);
    }
}

int
Test( int ch )
{
    if( Get_ind() != ch ) {
        In_pushed_back = TRUE;
        return( 0 );
    }
    return( 1 );
}

int
Get_ident(void)
{
    int i, ch;
    
    ch = Get_ind();
    In_pushed_back = TRUE;
    if( (Ctab[ch] & L) == 0 )
        return 0;

    memset(Name, 0, 32);
    for(i=0; i<=32; ++i) {
        ch = Get_ind();
        if( Ctab[ch] & (L|D) )
            Name[i] = ch;
        else {
            In_pushed_back = TRUE;
            break;
        }
    }
    return( ch!=0 && i!=0 );
}

struct Symbol {
    struct Symbol *link;
    int type;
    int value;
    char name[1];
};

struct Symbol*
Sym_alloc(char *name)
{
    struct Symbol *sym;
    int size = sizeof(struct Symbol) + Length(name);
    
    sym = (struct Symbol*)malloc( size);
    memset(sym, 0, size);
    Copy(name, sym->name);
    sym->type = -1;
    return sym;
}

/* 64-bucket hash code, section 3.2, pg 12. The hash algorithm is not
    * specified; follow the V5/V6 DMR C compiler */
int
Hash(char *name)
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

struct Symbol*
Sym_lookup(char *name, int type)
{
    struct Symbol *sym, **prevptr;
    int idx;
    
    idx = Hash(name);
    prevptr = &SymHash[idx];
    sym = *prevptr;
    
    while( sym ) {
        if( name[0] == sym->name[0] && name[1] == sym->name[1]
            && Equal(name, sym->name) )
        {
            if( sym->type != type && type != ANY ) {
                printf("identifier '%s' is wrong type on line %d\n", Name, Line_no);
                exit(1);
            }
            return sym;
        }
        prevptr = &sym->link;
        sym = sym->link;
    }
    if( type == INSERT ) {
        sym = Sym_alloc(name);
        sym->link = SymHash[idx];
        SymHash[idx] =  sym;
        return sym;
    }
    if( type != ANY ) {
        printf("identifier not found '%s' on line %d\n", Name, Line_no);
        exit(1);
    }
    return NULL;
}

void Sym_dump(void)
{
    int i;
    struct Symbol *sym;
    
    for( i = 0; i < 64; ++i ) {
    fprintf(stderr, "Chain %d:\n", i);
        sym = SymHash[i];
        while( sym ) {
            fprintf(stderr, "sym '%32s', %d, %04x\n", sym->name, sym->type, sym->value);
            sym = sym->link;
        }
    }
}

struct Symbol*
insert(char *name, int val, int type)
{
    struct Symbol *sym;

    sym = Sym_lookup(name, INSERT);
    sym->value = val;
    sym->type = type;
    return sym;
}

int
Get_num(void)
{
    int val, ch, is_num = FALSE;
    
    val = 0;
    while( 1 ) {
        ch = Get_ind();
        if( ch >= '0' && ch <= '9' ) {
            val = val * 10 + Ctab[ch] & 0xf;
            is_num = TRUE;
        }
        else {
            In_pushed_back = TRUE;
            break;
        }
    }
    if( !is_num )
        printf("bad argument on line %d\n", Line_no);
    return( val );
}

int
Get_arg(void)
{
    struct Symbol *sym = NULL;

    if( Get_ident() != 0 )
        sym = Sym_lookup(Name, ANY);
    if( !sym )
        return( Get_num() );
    if( sym->type != TI_OPC && sym->type != IL_OPC && sym->type != OPER )
        printf("bad argument on line %d\n", Line_no);
    return( sym->value );
}

#include "init.inc"

short Op_list[150];
short Conditions[500];
short Actions[1000];
int   Next_cond = 0;
int   Next_action = 0;
int   Next_IL = IL_LAST;

#define CONTINUE 0x8000
#define OP3      0x8000

void
Print_table(short *table, int len)
{
    int i;

    while( 1 ) {
        printf("    ");
        for( i=0; i<8; ++i) {
            printf("0x%04x", *table++ & 0xffff);
            --len;
            if( len )
                printf(", ");
            else
                break;
        }
        printf("\n");
        if( !len ) break;
    }
}

int
main()
{
    struct Symbol *sym;
    int ch, idx, n, cbase, abase;
    int action, op1, op2, op3;
    int ccnt, acnt;

    IFile = 0;
    //if( (IFile = open("../B/table.mac", O_RDONLY), 0644) < 0 ) {
    //    printf("cannot open B/table.mac input file\n");
    //    exit(1);
    //}
    Initialize();
    
    ccnt = acnt = 0;
    while( Get_ident() != 0 )
    {
        /* process a new definition header */
        sym = Sym_lookup(Name, ANY);
        if( sym == NULL ) {
            sym = insert(Name, (Next_IL += 2), IL_OPC );
        }
        if( sym->type != IL_OPC ) {
            printf("'%s' is not a IL operation or subroutine on line %d\n", Name, Line_no);
            exit(1);
        }
        Skip( '=' );
        idx = sym->value >> 1;
        if( Op_list[idx] != 0 )
            printf("IL operator '%s' double define\n", Name);
        Op_list[idx] = Next_cond + 1;
        Skip( '{' );

        do
        {
            /* process a set of conditions */
            cbase = Next_cond + 1;
            while( 1 ) {
                ++Next_cond; ++ccnt;
                if( Get_ident() == 0 )
                    printf("missing test operand on line %d\n", Line_no);
                sym = Sym_lookup(Name, OPER);
                Conditions[ Next_cond ] = sym->value  << 5;
                if( sym->value != 0 ) { /* only if not "always" */
                    Skip( ',' );
                    if( Get_ident() == 0 )
                        printf("missing test on line %d\n", Line_no);
                    sym = Sym_lookup(Name, TEST);
                    Conditions[ Next_cond ] |= sym->value;
                    if( Test('&') ) {
                        Conditions[ Next_cond ] |= CONTINUE;
                        continue;
                    }
                }
                if( Test(':') ) break;
                Skip(':'); /* error out */
            }

            /* process a set of actions */
            abase = Next_action; n = 0;
            Skip( '{' );
            while( Get_ident() != 0 )
            {
                op1 = op2 = op3 = 0;
                sym = Sym_lookup(Name, ACTION);
                action = sym->value << 10;
                Skip( '(' );
                op1 = Get_arg();
                if( Test(',') ) {
                    op2 = Get_arg();
                    if( Test(',') ) {
                        op3 = op1; op1 = op2;
                        op2 = Get_arg();
                        action |= OP3;
                    }
                }
                action |= (op1 & 0x1f) << 5;
                action |=  op2 & 0x1f;
                Skip( ')' ); Skip( ';' );
                Actions[ Next_action++ ] = action;
                if( action & OP3 )
                    Actions[ Next_action++ ] = op3;
                n++;
            }
            Skip( '}' );
            if( n > 0 ) {
                Conditions[ cbase ] |= (n << 10);
                Conditions[ ++Next_cond ] = abase;
                acnt += n;
            }
        }
        while( Test('}') == FALSE );
    }
    ch = Get_ind();
    if( ch != 0 ) {
        printf("unexpected character '%c' in line %d\n", ch, Line_no);
        exit( 1 );
    }
    
    idx = (Next_IL >> 1) + 1;
    printf("\n/* %d IL opcodes, of which %d subroutines */\n", idx, idx - IL_LAST/2 - 1);
    printf("unsigned short Op_list[] = {\n"); Print_table(Op_list, idx); printf("};\n\n");
    printf("/* %d conditions */\n", ccnt);
    printf("unsigned short Conditions[] = {\n"); Print_table(Conditions, Next_cond+1); printf("};\n\n");
    printf("/* %d actions */\n", acnt);
    printf("unsigned short Actions[] = {\n"); Print_table(Actions, Next_action); printf("};\n\n");
    
    //Sym_dump();
}

