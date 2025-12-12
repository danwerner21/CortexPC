
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "parse.h"
#include "il_opc.h"

int IFile, DFile, SFile;

int Line_no;
int Path_id;
int Level;

int
Get(int fd)
{
    char ch;
    
    if( read(fd, &ch, 1) <= 0 ) {
        return 0;
    }
    return (ch & 0xff);
}

void
print_zstr( void )
{
    int ch;

    lseek(SFile, Path_id, SEEK_SET);
    do {
        ch = Get(SFile);
        fprintf(stderr, "%c", ch);
    } while (ch != 0);
}

void
yyerror(char *s)
{ 
    fprintf( stderr, "%s on line %d ", s, Line_no );
    if( Level > 1 ) {
        fprintf( stderr, "in file " );
        print_zstr();
    }
    fprintf( stderr, "\n" );
}

# define ASSIGNOP 22
# define LOGOR 23
# define LOGAND 24
# define RELATIONALOP 25
# define DIVOP 26
# define BITWISEOP 27
# define PREUNIOP 28
# define POSTUNIOP 29
# define COLCOLN 30

struct tok {
    char term;
    char op;
} toktab[] = {
 /*  0     */      { 0,            0          },
 /*  1 =   */      { '=',          0          },
 /*  2 /=  */      { ASSIGNOP,     IL_ASDIV   },
 /*  3 *=  */      { ASSIGNOP,     IL_ASMUL   },
 /*  4 %=  */      { ASSIGNOP,     IL_ASMOD   },
 /*  5 +=  */      { ASSIGNOP,     IL_ASADD   },
 /*  6 -=  */      { ASSIGNOP,     IL_ASSUB   },
 /*  7 <<= */      { ASSIGNOP,     IL_ASSHL   },
 /*  8 >>= */      { ASSIGNOP,     IL_ASSHR   },
 /*  9 &=  */      { ASSIGNOP,     IL_ASAND   },
 /* 10 |=  */      { ASSIGNOP,     IL_ASOR    },
 /* 11 ^=  */      { ASSIGNOP,     IL_ASXOR   },
 /* 12 ==  */      { RELATIONALOP, IL_EQU     },
 /* 13 !=  */      { RELATIONALOP, IL_NEQU    },
 /* 14 >   */      { RELATIONALOP, IL_GT      },
 /* 15 >=  */      { RELATIONALOP, IL_GTE     },
 /* 16 <=  */      { RELATIONALOP, IL_LTE     },
 /* 17 <   */      { RELATIONALOP, IL_LT      },
 /* 18 >>  */      { BITWISEOP,    IL_SHR     },
 /* 19 <<  */      { BITWISEOP,    IL_SHL     },
 /* 20 |   */      { BITWISEOP,    IL_BITOR   },
 /* 21 ^   */      { BITWISEOP,    IL_BITXOR  },
 /* 22 /   */      { DIVOP,        IL_DIV     },
 /* 23 %   */      { DIVOP,        IL_MOD     },
 /* 24 ++  */      { POSTUNIOP,    IL_POSTINC },
 /* 25 --  */      { POSTUNIOP,    IL_POSTDEC },
 /* 26 ~   */      { PREUNIOP,     IL_BITNOT  },
 /* 27 !   */      { PREUNIOP,     IL_NOT     },
 /* 28 (   */      { '(',          0          },
 /* 29 )   */      { ')',          0          },
 /* 30 [   */      { '[',          0          },
 /* 31 ]   */      { ']',          0          },
 /* 32 {   */      { '{',          0          },
 /* 33 }   */      { '}',          0          },
 /* 34 ,   */      { ',',          0          },
 /* 35 ;   */      { ';',          0          },
 /* 36 ?   */      { '?',          0          },
 /* 37 ||  */      { LOGOR,        0          },
 /* 38 &&  */      { LOGAND,       0          },
 /* 39 +   */      { '+',          0          },
 /* 40 -   */      { '-',          0          },
 /* 41 *   */      { '*',          0          },
 /* 42 &   */      { '&',          0          },
 /* 43     */      { 0,            0          },
 /* 44 ::  */      { COLCOLN,      0          },
 /* 45 :   */      { ':',          0          },
 /* 46 #   */      { '#',          0          },
 /* 47     */      { 0,            0          },
 /* 48 auto    */  { 4,            0          },
 /* 49 extern  */  { 5,            0          },
 /* 50         */  { 0,            0          },
 /* 51         */  { 0,            0          },
 /* 52         */  { 0,            0          },
 /* 53 if      */  { 7,            0          },
 /* 54 else    */  { 6,            0          },
 /* 55 for     */  { 8,            0          },
 /* 56 repeat  */  { 9,            0          },
 /* 57 while   */  { 11,           0          },
 /* 58 select  */  { 10,           0          },
 /* 59 break   */  { 12,           0          },
 /* 60 goto    */  { 13,           0          },
 /* 61 next    */  { 14,           0          },
 /* 61 return  */  { 15,           0          },
 /* 63 case    */  { 16,           0          },
 /* 64 default */  { 17,           0          },
 /* 65 enable  */  { 18,           0          },
 /* 66 disable */  { 19,           0          },
 /* 67 twit    */  { 20,           0          },
 /* 68 active  */  { 21,           0          }
};

int
GetW(int fd)
{
    return (Get(fd) << 8) + Get(fd);
}

int
Get_const(int token)
{
    int len, val, i;
    
    val = 0;
    len = token & 0x7f;
    for( i = 0; i < len; ++i ) {
        val = (val << 8) + Get(IFile);
    }
    return val;
}

int
yylex()
{
    int  token, lex, val;

again:
    token = Get(IFile) & 0xff;

    /* operator or keyword */
    if( token >= 0 && token <= 68) {
        lex = toktab[token].term;
        yylval = toktab[token].op;
        return lex;
    }
    /* identifier */
    if( token == 124 ) {
        val = GetW(IFile);
        yylval = val;
        return 1;
    }
    /* numeric / character constant */
    if( token >= 128 && token < 253 ) {
        val = Get_const( token );
        yylval = val;
        return 2;
    }
    /* string constant */
    if( token == 125 ) {
        val = GetW(IFile);
        yylval = val;
        return 3;
    }
    /* new line */
    if( token == 126 ) {
        Line_no++;
        goto again;
    }
    /* new file */
    if( token == 127 ) {
        Path_id = GetW(IFile); /* eat path id */
        Line_no = GetW(IFile);
        Get(IFile); /* eat segment info */
        if( Line_no == 0 ) ++Level; else --Level;
        goto again;
    }
    yyerror("bad lexil token");
    return 0;
}

int OFile;

int yyparse(void);

int
main(){
    IFile = 0;
    Line_no = 1;
    if( (IFile = open("lexil", O_RDONLY) ) < 0 ) {
        printf("cannot open lexil input file\n");
        exit(1);
    }
    if( (SFile = open("strings", O_RDONLY) ) < 0 ) {
        printf("cannot open strings file\n");
        exit(1);
    }
    if( (OFile = open("functions", O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0 ) {
        printf("cannot open parsil functions file\n");
        exit(1);
    }
    if( (DFile = open("externals", O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0 ) {
        printf("cannot open parsil externals file\n");
        exit(1);
    }
    Ref_clear();
    yyparse();
    Put(0); DPut(0);
    //Ref_print();
    return 0;
}

void
Put(int token)
{
    char byte = token;
    write(OFile, &byte, 1);
}

void
PutR(int val)
{
    char byte;

    if( val <= 127 ) {
        byte = val & 0x7f;
        write(OFile, &byte, 1);
    }
    else {
        byte = (val & 0x7f) | 0x80;
        write(OFile, &byte, 1);
        byte = (val >> 7) & 0xff;
        write(OFile, &byte, 1);
    }
}

void
PutW(int word)
{
    char byte;
    
    byte = (word >> 8) & 0xff;
    write(OFile, &byte, 1);
    byte = word & 0xff;
    write(OFile, &byte, 1);
}

void
PutC(int val)
{
    unsigned char con[8];
    int i;
    
    for( i = 0; i < 8; i++) {
        con[i] = val & 0xff;
        val >>= 8;
    }
    i--;
    while( i-- > 0 ) {
        if( con[i] != 0xff && con[i] != 0x00 ) {
            break;
        }
    }
    Put( ++i | 0x80 );
    while( --i >= 0 ) Put(con[i]);
}

void
DPut(int token)
{
    char byte = token;
    write(DFile, &byte, 1);
}

void
DPutW(int word)
{
    char byte;
    
    byte = (word >> 8) & 0xff;
    write(DFile, &byte, 1);
    byte = word & 0xff;
    write(DFile, &byte, 1);
}

void
DPutC(int val)
{
    unsigned char con[8];
    int i;
    
    for( i = 0; i < 8; i++) {
        con[i] = val & 0xff;
        val >>= 8;
    }
    while( i-- > 0 ) {
        if( con[i] != 0xff && con[i] != 0x00 ) {
            break;
        }
    }
    ++i;
    DPut( ++i | 0x80 );
    while( --i >= 0 ) DPut(con[i]);
}
