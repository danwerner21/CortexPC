
#include "lex.h"
#include "lexil.h"

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
    int i;
    
    Const_clr();
    while(1) {
        ch=Ctab[Get_ind() & 0x7f];
        if( (ch & D) == 0 || (ch & 0x0f) >= base) break;
        Const_mul( base );
        Const_add( ch & 0x0f );
    }
    In_pushed_back = TRUE;
    Const_put();
}

void
Process_char()
{
    int ch;

    In_string = TRUE;
    if( (ch = Escaped_char('\'')) >= 0 ) {
        Get_ind();
        Put(0x81);
        Put(ch);
    } else
        Error("empty character constant");
    In_string = FALSE;
}

void
Process_string()
{
    int StrID, ch;
    
    In_string = TRUE;
    Intern_start();
    while( (ch = Escaped_char('"')) >= 0 ) {
        Intern_char(ch);
    }
    StrID = Intern_length();
    Put(STRING);
    PutW( StrID );
    In_string = FALSE;
}

/* Keyword test as per section 3.2, pg 12 */

extern char Name[];

int
Is_kw(void)
{
    switch( Name[0]+(Name[1]<<8) ) {
    case 'a'+('u'<<8): if( Equal(Name, "auto" ) )    return KW_AUTO;    break;
    case 'e'+('x'<<8): if( Equal(Name, "extrn" ) )   return KW_EXTRN;   break;
    case 'i'+('f'<<8): if( Name[2]==0 )              return KW_IF;      break;
    case 'e'+('l'<<8): if( Equal(Name, "else" ) )    return KW_ELSE;    break;
    case 'f'+('o'<<8): if( Equal(Name, "for" ) )     return KW_FOR;     break;
    case 'r'+('e'<<8): if( Equal(Name, "repeat" ) )  return KW_REPEAT;
                       if( Equal(Name, "return" ) )  return KW_RETURN;  break;
    case 'w'+('h'<<8): if( Equal(Name, "while" ) )   return KW_WHILE;   break;
    case 's'+('e'<<8): if( Equal(Name, "select" ) )  return KW_SELECT;  break;
    case 'b'+('r'<<8): if( Equal(Name, "break" ) )   return KW_BREAK;   break;
    case 'g'+('o'<<8): if( Equal(Name, "goto" ) )    return KW_GOTO;    break;
    case 'n'+('e'<<8): if( Equal(Name, "next" ) )    return KW_NEXT;    break;
    case 'c'+('a'<<8): if( Equal(Name, "case" ) )    return KW_CASE;    break;
    case 'd'+('e'<<8): if( Equal(Name, "default" ) ) return KW_DEFAULT; break;
    case 'e'+('n'<<8): if( Equal(Name, "enable" ) )  return KW_ENABLE;  break;
    case 'd'+('i'<<8): if( Equal(Name, "disable" ) ) return KW_DISABLE; break;
    case 't'+('w'<<8): if( Equal(Name, "twit" ) )    return KW_TWIT;    break;
    case 'a'+('c'<<8): if( Equal(Name, "active" ) )  return KW_ACTIVE;  break;
    }
    return 0;
}

/* Process a name as per Stafford section 3.2 pp 13-14 */
void
Process_ident_or_kw()
{
    int kw;
    struct Symbol *sym;
    
    Get_ident();
    if( (kw = Is_kw()) ) {
        Put(kw);
        return;
    }
    
    sym = Sym_lookup(TRUE);
    //fprintf(stderr, "lookup '%s', type = %d\n", Name, sym->type);
    switch( sym->type ) {
    case UNDEFINED:
        sym->type = IDENTIFIER;
        sym->loc = Intern_ident(); 
        /* fall through */
    case IDENTIFIER:
        Put(ID);
        PutW(sym->loc);
        return;
    case MANIFEST:
        In_expand(sym->loc);
        return;
    }
}

/* Main lexer routine as per Stafford, Appendix A */

void
Lex(void)
{
    int token, ch;

    while(1)
    {
        switch( ch=Get_ind() )
        {
            case ' ' :		//	ignore blanks
            case '\t' :		//	and tabs
            case '\f' :		//	and form feeds
                continue;
                
            case 0 : return;	//	end of input, finished.

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
            case '~' : token = ONES_COMPLEMENT;	    break;
            case '?' : token = QUERY;		        break;
            case ']' : token = CLOSE_BRACKET;	    break;
            case ')' : token = CLOSE_PARENTHISIS;	break;
            case '}' : token = CLOSE_BRACE;		    break;
            case ';' : token = SEMI_COLON;		    break;
            case '/' :		
                if( (ch = Get_ind()) == '=' )
                    token = ASSIGN_DIV;
                else
                {
                    token = DIVIDE;
                    In_pushed_back = TRUE;
                }
                break;
            case '\\' :
            {
                while( ( ch=Get_ind()) != '\n' && ch );
                In_pushed_back = TRUE;
                continue;
            }
            case '!' :
                if( (ch = Get_ind()) == '=' )
                    token = NOT_EQUAL;
                else
                {
                    token = NOT;
                    In_pushed_back = TRUE;
                }
                break;
            case '+' :
                if( (ch=Get_ind()) == '+' )
                    token = INCREMENT;
                else if( ch == '=' )
                    token = ASSIGN_ADD;
                else
                {
                    token = PLUS;
                    In_pushed_back = TRUE;
                }
                break;
            case '-' :
                if( (ch = Get_ind()) == '-' )
                    token = DECREMENT;
                else if( ch == '=' )
                    token = ASSIGN_SUB;
                else
                {
                    token = MINUS;
                    In_pushed_back = TRUE;
                }
                break;
            case ':' :
                if( (ch=Get_ind()) == ':'	)
                    token = COLON_COLON;
                else
                {
                    token = COLON;
                    In_pushed_back = TRUE;
                }
                break;
            case '|' :
                if( (ch = Get_ind()) == '|' )
                    token = LOGICAL_OR;
                else if( ch == '=' )
                    token =	ASSIGN_OR;
                else
                {
                    token = BITWISE_OR;
                    In_pushed_back = TRUE;
                }
                break;
            case '^' :
                if( (ch=Get_ind()) ==	'=' )
                    token = ASSIGN_XOR;
                else
                {
                    token = EXCLUSIVE_OR;
                    In_pushed_back = TRUE;
                }
                break;
            case '&' :
                if( (ch = Get_ind()) == '&' )
                    token = LOGICAL_AND;
                else if( ch == '=' )
                    token = ASSIGN_AND;
                else
                {
                    token = AMPERSAND;
                    In_pushed_back = TRUE;
                }
                break;
            case '*' :
                if( (ch = Get_ind()) == '=' )
                    token = ASSIGN_MULT;
                else
                {
                    token = ASTERISK;
                    In_pushed_back = TRUE;
                }
                break;
            case '%' :
                if( (ch = Get_ind()) == '=' )
                    token = ASSIGN_MOD;
                else
                {
                    token = MODULO;
                    In_pushed_back = TRUE;
                }
                break;
            case '<'	:
                if( (ch = Get_ind()) == '<' )
                    if( (ch = Get_ind()) == '=' )
                        token = ASSIGN_LEFT_SHIFT;
                    else
                    {
                        token = LEFT_SHIFT;
                        In_pushed_back = TRUE;
                    }
                else if( ch == '=' )
                    token = LESS_OR_EQUAL;
                else
                {
                    token = LESS_THAN;
                    In_pushed_back = TRUE;
                }
                break;
            case '>'	:
                if( (ch=Get_ind()) =='>'	)
                    if( (ch=Get_ind()) == '=' )
                        token = ASSIGN_RIGHT_SHIFT;
                    else
                    {
                        token = RIGHT_SHIFT;
                        In_pushed_back = TRUE;
                    }
                else if( ch == '=' )
                    token = GREATER_OR_EQUAL;
                else
                {
                    token = GREATER_THAN;
                    In_pushed_back = TRUE;
                }
                break;
            case '='	:
                if( (ch = Get_ind()) == '=' )
                    token = EQUAL;
                else
                {
                    token = ASSIGN;
                    In_pushed_back = TRUE;
                }
                break;
            case '\'' :
            {
                Process_char();
                continue;
            }
            case '"' :
            {
                Process_string();
                continue;
            }
            case '0' :
            {
                Process_const( 010 );
                continue;
            }
            case '$' :
            {
                Process_const( 0x10 );
                continue;
            }
            default :
            {
                if( ch>='1' && ch<='9' )
                {
                    In_pushed_back = TRUE;
                    Process_const( 10 );
                    continue;
                }
                if( (ch>='a' && ch<='z') ||
                    (ch>='A' && ch<='Z') ||
                    ch == '_' || ch == '.' )
                {
                    In_pushed_back = TRUE;
                    Process_ident_or_kw();
                    continue;
                }
                fprintf(stderr, "ch=%d\n", ch);
                Error( "bad character in source" );
                continue;
            }
        }
        Put( token );
    }
}
