
#include "tla.h"
#include "tokens.h"

#include <stdio.h>

/* This is the 128 entry table described in Stafford section 3.5.1 and
 * 3.5.2, pp 27-29
 */

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

int
Escaped_char(int delim)
{
    int ch, och, val, cnt;

    ch = Get_ind();
rescan:
    if( ch == delim )
        return(-1);

    switch(ch) {
    case '\n':
    case '\0':
        Error("nonterminated string");
        return(-1);

    case '*':
        och = ch;
        switch (ch = Get_ind()) {

        case 't':   return('\t');
    case 'n':   return('\n');
    case 'f':   return('\f');
    case 'b':   return('\b');
    case 'r':   return('\r');
    case '#':   {
        val = 0;
        cnt = 0;
        while( ++cnt <= 3 ) {
            ch = Ctab[Get_ind()];
            if( (ch & D)==0 || (ch & 0x0f) > 7 ) {
                In_pushed_back = TRUE;
                break;
            }
            val <<= 3;
            val += ch & 0x0f;
        }
        return(val & 0xff);
        }
    case '$':   {
        val = 0;
        cnt = 0;
        while( ++cnt <= 2 ) {
            ch = Ctab[Get_ind()];
            if( (ch & D)==0 )  {
                In_pushed_back = TRUE;
                break;
            }
            val <<= 4;
            val += ch & 0x0f;
        }
        return(val);
        }
    case '\n':  {
        ch = Get_ind();
        goto rescan;
        }
    }
    In_pushed_back = TRUE;
    ch = och;
    }
    return(ch);
}

void
Process_const(int base)
{
    char ch;
    int val;

    val = 0;
    while(TRUE) {
        ch=Ctab[Get_ind() & 0x7f];
        if( (ch & D) == 0 || (ch & 0x0f) >= base) break;
        val *= base;
        val += ch & 0x0f ;
    }
    In_pushed_back = TRUE;
    Oper.val = val;
    Oper.seg = RB_ABS;
}

void
Process_char()
{
    int ch;

    In_string = TRUE;
    if( (ch = Escaped_char('\'')) >= 0 ) {
        Get_ind();
        Oper.val = ch;
        Oper.seg = RB_ABS;
    } else
        Error("empty character constant");
    In_string = FALSE;
}

/* Process a string */
#define MAXSTR 256
char String[MAXSTR];

void
Process_string()
{
    int i = 0;
    char ch;
    
    In_string = TRUE;
    while( (ch = Escaped_char('"')) >= 0 ) {
        String[i++] = ch;
        if( i >= MAXSTR - 1 ) break;
    }
    String[i] = 0;
    In_string = FALSE;
}

extern char Name[];
struct Symbol *Sym;

/* Process a name */
void
Process_identifier()
{
    int kw;
    struct Symbol *sym;
    
    Get_ident();
    
    sym = Sym_lookup(TRUE);
    //fprintf(stderr, "lookup '%s', type = %d\n", Name, sym->type);
    switch( sym->type ) {
    case UNDEFINED:
        sym->val  = 0;
        sym->seg  = UNDEFINED;
        /* fall through */
    default:
        Sym = sym;
        token = ID;
        return;

    case MANIFEST:
        In_expand(sym->val);
        token = 0;
        return;
    }
}

int token;

void
Lex(void)
{
    int ch;

again:
    switch( ch=Get_ind() )
    {
        case ' ' :		//	ignore blanks
        case '\t' :		//	and tabs
        case '\f' :		//	and form feeds
            goto again;
            
        case 0 : //	end of input, finished.
            token = 0;
            break;

        case '\n'	:
        {
            token = END_LINE;
            ++Line_no;
            break;
        }
        
        case '#' : token = NUMBER_SIGN;	    	break;
        case '(' : token = OPEN_PARENTHISIS;	break;
        case '[' : token = OPEN_BRACKET;    	break;
        case '{' : token = OPEN_BRACE;	    	break;
        case ',' : token = COMMA;	        	break;
        case ']' : token = CLOSE_BRACKET;	    break;
        case ')' : token = CLOSE_PARENTHISIS;	break;
        case '}' : token = CLOSE_BRACE;		    break;
        case ';' : token = SEMI_COLON;		    break;
        case '/' : token = DIVIDE;              break;
        case '%' : token = MODULO;              break;
        case '*' : token = ASTERISK;            break;
        case '!' : token = NOT;                 break;
        case ':' : token = COLON;               break;        
        case '^' : token = EXCLUSIVE_OR;        break;        
        case '&' : token = AMPERSAND;           break;        
        case '|' : token = BITWISE_OR;          break;
        case '@' : token = AT_SIGN;             break;
  
        case '\\' :
        {
            while( ( ch=Get_ind()) != '\n' && ch );
            In_pushed_back = TRUE;
            goto again;
        }

        case '+' :
            if( (ch=Get_ind()) == '+' )
                token = INCREMENT;
            else
            {
                token = PLUS;
                In_pushed_back = TRUE;
            }
            break;

        case '-' :
            if( (ch = Get_ind()) == '-' )
                token = DECREMENT;
            else
            {
                token = MINUS;
                In_pushed_back = TRUE;
            }
            break;

        case '<'	:
            if( (ch = Get_ind()) == '<' ) {
                token = LEFT_SHIFT;
                break;
            }
            In_pushed_back = TRUE;
            ch = '<';
            goto bad;

        case '>'	:
            if( (ch = Get_ind()) == '>'	) {
                token = RIGHT_SHIFT;
                break;
            }
            In_pushed_back = TRUE;
            ch = '>';
            goto bad;

        case '\'' :
        {
            Process_char();
            token = NUMBER;
            break;
        }
        case '"' :
        {
            Process_string();
            token = STRING;
            break;
        }
        case '0' :
        {
            Process_const( 8 );
            token = NUMBER;
            break;
        }
        case '$' :
        {
            Process_const( 16 );
            token = NUMBER;
            break;
        }
        default :
        {
            if( ch>='1' && ch<='9' )
            {
                In_pushed_back = TRUE;
                Process_const( 10 );
                token = NUMBER;
                break;
            }
            if( (ch>='a' && ch<='z') ||
                (ch>='A' && ch<='Z') ||
                ch == '_' || ch == '.' )
            {
                In_pushed_back = TRUE;
                Process_identifier();
                if( token == 0 ) goto again; // MANIFEST expansion
                break;
            }
bad:
            Error( "bad character in source" );
            goto again;
        }
    }
    return;
}
