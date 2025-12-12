#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "code.h"
#include "../Parse/il_opc.h"
#include "ti_opcodes.h"

extern int SFile;

static struct Symbol *argv[3];
static int argc;

int
Arg_check( struct Expr *expr )
{
    int i, argc, ref;

    argc = 0;
    argv[0] = argv[1] = argv[2] = NULL;

    while( expr && expr->op != IL_MARK ) {
        if( expr->op == IL_COMMA ) {
            if( argc == 3 ) {
                fprintf(stderr, "More than 3 args to twit\n");
                return FALSE;
            }
            ref = expr->right->op;
            if( ref & 1) {
                fprintf(stderr, "argument to twit not a symbol\n");
                return FALSE;
            }
            argv[2 - argc] = (struct Symbol*)(expr->right);
            
            // Examine next argument
            ++argc;
            expr = expr->left;
        }
    }
    if( argc < 3 ) {
        argv[0] = argv[1]; argv[1] = argv[2]; argv[2] = NULL;
    }
    if( argc < 2 ) {
        argv[0] = argv[1]; argv[1] = NULL;
    }
    if( argc == 0 ) {
        fprintf(stderr, "Missing opcode in twit\n");
        return FALSE;
    }
    return TRUE;
}

void
Process_twit_string( int i )
{
    char ch, str[11];
    int j, val;

    lseek(SFile, argv[i]->str_id + 2, SEEK_SET);    
    for( j = 0; j < 10; j++ ) {
        if( read(SFile, &ch, 1) < 0 )
            ch = 0;
        if( (str[j] = ch) == 0 ) break;
    }
    str[10] = 0;

    argv[i]->type      = AUTO_TYPE;
    argv[i]->value     = 0;
    if( sscanf( str, "r%d", &val )==1 && val >= 0 && val <= 15 )
        argv[i]->addrmod  = val;
    else if ( sscanf( str, "ri%d", &val )==1 && val >= 0 && val <= 15 )
        argv[i]->addrmod  = val | 0x10;
    else if ( sscanf( str, "ria%d", &val )==1 && val >= 0 && val <= 15 )
        argv[i]->addrmod  = val | 0x30;
    else {
        argv[i]->addrmod  = val;
        fprintf(stderr, "Unrecognised twit string\n");
    }
}

void
Twit_gen( struct Expr *expr )
{
    int opc;

    if( !Arg_check( expr ) ) return;
    if( (argv[0]->type & TYPE_FIELD) != CONST_TYPE ) {
        fprintf(stderr, "Twit opcode not a constant number\n");
        return;
    }
    opc = (int)argv[0]->const_value & 0xffff;

    if( argv[1] && (argv[1]->type & TYPE_FIELD) == STRING_TYPE ) {
        Process_twit_string( 1 );
    }

    if( argv[2] && (argv[2]->type & TYPE_FIELD) == STRING_TYPE ) {
        Process_twit_string( 2 );
    }

    // TO DO: consider B, BL and BLWP
    if( opc >= _JMP_ && opc <= _JOP_ && !(argv[1]->type & SYMBOL_DEFINED) )
        Defer_gen( opc, argv[1], 2, 240 );
    else
        Code( opc, argv[1], argv[2] );
}
