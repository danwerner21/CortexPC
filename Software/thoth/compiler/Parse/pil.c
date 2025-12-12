
/* Stafford writes in Appendix E: ''In order for the intermediate language
    * representions to be readable, they have been passed through two utility
    * programs which print them in a readable form. These two utility programs,
    * "Plexil" and "Pil", have been invaluable in debugging the compiler.''
    *
    * This program is intended to replicate the functionality of "Pil".
    * It reads the output of Parse (i.e. parsil code) and prints a human
    * readable rendering of it.
    */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

char *opcstr[] = {
    "=", "/=", "*=", "%=", "+=", "-=", "<<=", ">>=",  /*   1-15  */
    "&=", "|=", "^=", "==", "!=", ">",  ">=", "<=",   /*  17-31  */
    "<",  ">>", "<<", "|",  "&",  "^",  "/",  "%",    /*  33-47  */
    "++pre", "--pre", "++post", "--post",             /*  49-55  */
    "~",  "!",  "[",  "{",  ",",  "?",  ":",  "||",   /*  57-71  */ 
    "&&", "+",  "-",  "*",  "LV", "RV", "{=",         /*  73-85  */
    "By_fet", "Wd_fet", "(nargs", "(", "mark", "",    /*  87-97  */
    "R=", "R/=", "R*=", "R%=", "R+=", "R-=", "R<<=",  /*  99-111 */
    "R>>=", "R&=", "R|=", "R^=", "R>>", "R<<", "R-",  /* 113-125 */  
    "R{=", "R/", "R%", "R{", "N,", "&~", "ADDR",      /* 127-139 */
    "U-"                                              /* 141     */
};

#define TRUE  1
#define FALSE 0

int IFile;
int SFile;
int DFile;
int In_twit;

int
Get(void)
{
    char ch;
    
    if( read(IFile, &ch, 1) <= 0 ) {
        exit(1);
    }
    return (ch & 0xff);
}

int
GetW(void)
{
    return (Get() << 8) + Get();
}

int
GetR(void)
{
    int val;
    
    val = Get();
    if( val & 0x80 ) {
        val = (val & 0x7f) + (Get() << 7);
    }
    return val;
}

void
print_zstr(int str_id)
{
    char ch;
    int len, i;

    lseek(SFile, str_id, SEEK_SET);
    do {
        read(SFile, &ch, 1);
        printf("%c", ch);  
    } while( ch != 0 );
}

void
print_lstr(int str_id)
{
    char ch;
    int len, i;

    lseek(SFile, str_id, SEEK_SET);
    read(SFile, &ch, 1);
    len = ch;
    read(SFile, &ch, 1);
    len = (len << 8) + ch;
    printf("\"");
    for( i = 0; i < len - 1; ++i ) {
        read(SFile, &ch, 1);
        if( ch == 0 )
            printf("*$00");  
        else if( ch == 10 )
            printf("*n");  
        else
            printf("%c", ch);  
    }
    printf("\"");
}

#define MAXREF 240

char reftyp[MAXREF];
int  refnam[MAXREF];

void
set_ref(int ref, char type)
{
    if( ref < MAXREF ) {
        reftyp[ref>>1] = type;
    }
}

void
set_refnam(int ref, int str_id)
{
    if( ref < MAXREF ) {
        refnam[ref>>1] = str_id;
    }
}

void
print_ref(int ref)
{
    int type;

    type = reftyp[ref>>1];
    switch( type ) {
    case 'c': printf("C%o", ref);     break;
    case '"': printf("S%o", ref);     break;
    case 'a': printf("arg%o", ref);   break;
    case 'A': printf("A%o", ref);     break;
    case 'V': printf("A_v%o", ref);   break;
    case 'v': printf("A_s%o", ref);   break;
    case 'g': print_zstr(refnam[ref>>1]); break;
    case 'l': printf("label%o", ref); break;
    default:
        if( In_twit ) {
            set_ref( ref, 'l');
            printf("label%o", ref);
            break;
        }
        printf("error: undeclared reference\n");
    }
}

void
print_expr(void)
{
    int ref;

    while( (ref = GetR()) != 0 ) {
        if( ref & 1 ) {
            printf("%s ", opcstr[ref>>1]);
            continue;
        }
        print_ref(ref); printf(" ");
    }
}

void
print_func(void)
{
    int token, val, ref;

    while( (token = Get()) != 0 ) {
        switch( token ) {
        
        /* ==== function header and declarations ==== */

        case 'a':  /* argument */
            ref = GetR();
            set_ref(ref, 'a');
            print_ref(ref); printf(" ");
            break;

        case 'P':  /* prolog */
            val = Get(); /* min */
            val = Get(); /* max */
            val = Get(); /* seg */
            printf(")\n\t\t#(%d,%d)\n", (val>>4) & 0xf, val & 0xf);
            break;

        case 'A':  /* auto */
            ref = GetR();
            set_ref(ref, 'A');
            printf("\tauto "); print_ref(ref); printf(";\n");
            break;
        
        case 'V':  /* auto vector */
            ref = GetR();
            val = GetW();
            set_ref(ref, 'V');
            printf("\tauto "); print_ref(ref); printf("[%d];\n", val);
            break;
        
        case 'v':  /* auto string */
            ref = GetR();
            val = GetW();
            set_ref(ref, 'v');
            printf("\tauto "); print_ref(ref); printf("{%d};\n", val);
            break;

        case 'g':  /* external name */
            ref = GetR();
            val = GetW();
            set_ref(ref, 'g');
            set_refnam(ref, val);
            printf("\textrn "); print_zstr(val); printf(";\n");
            break;
        
        case 'H':  /* start of executable body */
            printf("\t\\ End of declarations\n");
            break;

        /* ==== expressions ==== */

        case 'c':  /* declare constant REF */
            ref = GetR();
            val = GetW();
            set_ref(ref, 'c');
            printf("\t"); print_ref(ref); printf(" = %d\n", val);
            break;
        
        case '\'': /* declare twit string */
        case '"':  /* declare string REF */
            ref = GetR();
            val = GetW();
            set_ref(ref, '"');
            printf("\t"); print_ref(ref);
            printf(" = "); print_lstr(val); printf(";\n");
            break;
        
        case 'E': /* expression */
            printf("\t ");
            print_expr();
            printf(";\n");
            break;
        
        /* ===== statements ====== */

        case 'R': /* return */
            printf("\treturn( ");
            print_expr();
            printf(");\n");
            break;
        
        case 'l':  /* label */
            ref = GetR();
            set_ref(ref, 'l');
            print_ref(ref); printf(":\n");
            break;
            
        case 'J': /* goto label */
            ref = GetR();
            set_ref(ref, 'l');
            printf("\tgoto "); print_ref(ref); printf(";\n");
            break;

        case 'T': /* if expr goto label */
        case 'F': /* if !expr goto label */
            ref = GetR();
            set_ref(ref, 'l');
            printf( (token == 'T') ? "\tif( (" : "\tif( !(");
            print_expr();
            printf(") ) goto "); print_ref(ref); printf(";\n");
            break;

        case 'W': /* select word */
            printf("\tselect( "); print_expr(); printf(")\n\t{\n");
            break;
        
        case 'S': /* select string */
            printf("\tselect{ "); print_expr(); printf("}\n\t{\n");
            break;
            
        case 'D': /* default */
            printf("\tdefault:\n");
            break;
        
        case 'i': /* case <const> */
            printf("\tcase ");
            val = GetW();
            printf("%d:\n", val);
            break;
        
        case 'r': /* case <const>::<const> */
            printf("\tcase ");
            val = GetW();
            printf("%d::", val);
            val = GetW();
            printf("%d:\n", val);
            break;
        
        case 'b': /* case <str> */
            val = GetW();
            printf("\tcase ");
            print_lstr(val);
            printf(":\n");
            break;
        
        case 'w': /* end select word */
        case 's': /* end select string */
            printf("\t};\n");
            break;
            
        case 'e':
            printf("\tenable;\n");
            break;
            
        case 'd':
            printf("\tdisable;\n");
            break;
                    
        case 't':
            In_twit = TRUE;
            printf("\ttwit( "); print_expr(); printf(");\n");
            In_twit = FALSE;
            break;

        default: 
            printf("unrecognised IL statement: '%c' (%d)\n", token, token);
        }
    }
}

void
print_parsil(void)
{
    int token, val, ref;

    while( (token = Get()) != 0 )
    {
        switch( token ) {

        case ':':  /* external def */
            val = GetW();
            print_zstr(val); printf(";\n");
            break;

        case '(':  /* function def */
            val = GetW();
            print_zstr(val); printf("(");
            print_func();
            break;

        default: {
            printf("unrecognised parsil statement: '%c'\n", token);
            }
        }
    }
}

int
DGet(void)
{
    char ch;
    
    if( read(DFile, &ch, 1) <= 0 ) {
        exit(1);
    }
    return (ch & 0xff);
}

int
DGetW(void)
{
    return (DGet() << 8) + DGet();
}

void
print_init(void)
{
    int byte, len, val, i;
    
    byte = DGet();
    if( byte & 0x80 ) {
        val = 0;
        len = byte & 0x7f;
        for( i = 0; i < len; ++i ) {
            val = (val << 8) + DGet();
        }
        printf("%d", val);
    }
    else if( byte == 'g' ) {
        val = DGetW();
        print_zstr(val);
    }
    else
        printf("unrecognised external init: '%c'\n", byte);
}

void
print_list(void)
{
    int byte, len, val, i;

    while(1) {
        byte = DGet();
        if( byte == 0 ) break;
        
        if( byte & 0x80 ) {
            val = 0;
            len = byte & 0x7f;
            for( i = 0; i < len; ++i ) {
                val = (val << 8) + DGet();
            }
            printf("%d ", val);
            continue;
        }
        
        if( byte == 'g' ) {
            val = DGetW();
            print_zstr(val); printf(" ");
            continue;
        }
        
        if( byte == '"' ) {
            val = DGetW();
            print_lstr(val); printf(" ");
            continue;
        }
        
        if( byte == '[' ) {
            printf("[ ");
            continue;
        }
        
        if( byte == ']' ) {
            printf("] ");
            continue;
        }
        printf("bad character in initialiser list");
    }
}

void
print_datil(void)
{
    int token, val, ref, rbr;

    while( (token = DGet()) != 0 )
    {
        switch( token ) {

        case ':':  /* simple def */
            val = DGetW(); print_zstr(val);
            rbr = DGet(); printf("  #(%d,%d) : ", (rbr>>4)&0xf, rbr&0xf);
            print_init(); printf(";\n");
            break;

        case '"':  /* string def */
            val = DGetW(); print_zstr(val);
            rbr = DGet(); 
            printf("{ "); print_init(); printf(" }");
            printf("  #(%d,%d) : ", (rbr>>4)&0xf, rbr&0xf);
            val = DGetW(); print_lstr(val); printf(";\n");
            break;
        
        case '[':  /* vector def */
            val = DGetW();
            print_zstr(val);
            rbr = DGet();
            printf("[ "); print_init(); printf(" ]");
            printf("  #(%d,%d) : ", (rbr>>4)&0xf, rbr&0xf);
            print_list();
            printf(";\n");
            break;
            
        default: {
            printf("unrecognised parsil statement: '%c'\n", token);
            }
        }
    }
}

int
main(int argc, char **argv)
{
    char *name = "functions";
    
    if( argc >= 2 ) name = argv[1];
    
    if( (IFile = open(name, O_RDONLY)) < 0 ) {
        printf("cannot open parsil functions file\n");
        exit(1);
    }
    if( (DFile = open("externals", O_RDONLY)) < 0 ) {
        printf("cannot open parsil externals file\n");
        exit(1);
    }
    if( (SFile = open("strings", O_RDONLY)) < 0 ) {
        printf("cannot open strings file\n");
        exit(1);
    }
    print_datil();
    print_parsil();
}

