
/* Stafford writes in Appendix E: ''In order for the intermediate language
    * representions to be readable, they have been passed through two utility
    * programs which print them in a readable form. These two utility programs,
    * "Plexil" and "Pil", have been invaluable in debugging the compiler.''
    *
    * This program is intended to replicate the functionality of "Plexil".
    * It reads the output of Lex (i.e. lexil code) and prints a reconstructed
    * pre-processed source file (manifests and includes remain expanded).
    */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

char *tokstr[] = {
    NULL,                                              /* 0 = IDENT */
    "=", "/=", "*=", "%=", "+=", "-=", "<<", ">>",     /*  1- 8 */
    "&=", "|=", "^=", "==", "!=", ">",  ">=", "<=",    /*  9-16 */
    "<",  ">>", "<<", "|",  "^",  "/",  "%",  "++",    /* 17-24 */
    "--", "~",  " !",  "(",  ")",  "[",  "]",  "{",    /* 25-32 */ 
    "}",  ", ",  ";",  "?",  "||", "&&", "+",  "-",    /* 33-40 */
    "*",  "&",  NULL, "::", ":",  "#",  NULL, "auto ", /* 41-48 */
    "extrn ", NULL, NULL, NULL, "if", "else ", "for",  /* 49-55 */
    "repeat", "while", "select", "break", "goto ",     /* 56-60 */
    "next", "return ", "case ", "default", "enable",   /* 61-65 */
    "disable", "twit", "active"                        /* 66-68 */
};

int IFile;
int SFile;

int
Get(int fd)
{
    char ch;
    
    if( read(fd, &ch, 1) <= 0 ) {
        exit(1);
    }
    return (ch & 0xff);
}

int
GetW(int fd)
{
    return (Get(fd) << 8) + Get(fd);
}

void
print_zstr()
{
    int loc, ch;
    
    loc = GetW(IFile);
    lseek(SFile, loc, SEEK_SET);
    do {
        ch = Get(SFile);
        if( ch ) printf("%c", ch);
    } while (ch != 0);
    printf(" ");
}

void
print_lstr()
{
    int loc, len, i, ch;
    
    loc = GetW(IFile);
    lseek(SFile, loc, SEEK_SET);
    len = GetW(SFile);
    printf("\"");
    for( i = 0; i < len - 1; ++i ) {
        ch = Get(SFile);
        if( ch == 0 )
            printf("*$00");  
        else if( ch == 10 )
            printf("*n");  
        else
            printf("%c", ch);  
    }
    printf("\" ");
}

void
print_const(int token)
{
    int len, val, i;
    
    val = 0;
    len = token & 0x7f;
    for( i = 0; i < len; ++i ) {
        val = (val << 8) + Get(IFile);
    }
    printf("0%o ", val);
}

int indent = 0;
int col;

void
print_token(int token)
{
    char *str;
    int i;

    /* pad out indentation Eh style */
    if( token == 33 ) indent--;
    if( col++ == 0 ) {
        for(i = 0; i < indent; ++i) printf("    ");
        if( token == 32 ) {
            printf("  { ");
            indent++;
            return;
        } else if( token  == 33 ) {
            printf("  } ");
            return;
        }
    }
    if( token == 32 ) indent++;
    
    /* operator or keyword */
    if( token >= 1 && token <= 68) {
        str = tokstr[token];
        if( str == NULL ) {
            printf("\nbad token %d\n", token);
            return;
        }
        if( token >= 48 )
            printf("%s", str);
        else
            printf("%s", str);
        return;
    }
    /* identifier */
    if( token == 124 ) {
        print_zstr();
        // printf("");
        return;
    }
    /* numeric / character constant */
    if( token >= 128 && token < 253 ) {
        print_const(token);
        return;
    }
    /* string constant */
    if( token == 125 ) {
        print_lstr();
        return;
    }
    /* new line */
    if( token == 126 ) {
        printf("\n");
        col = 0;
        return;
    }
    /* new file */
    if( token == 127 ) {
        printf("\n\\ Entering file ");
        print_zstr();
        printf("on line %d\n", GetW(IFile));
        Get(IFile); /* eat segment info */
        return;
    }
}

int
main()
{
    int ch;

    IFile = 0;

    if( (SFile = open("strings", O_RDONLY)) < 0 ) {
        printf("cannot open strings file\n");
        exit(1);
    }
    while(1) {
        ch = Get(IFile) & 0xff;
        //fprintf(stderr, "%x ", ch);
        print_token(ch);
    }
}
