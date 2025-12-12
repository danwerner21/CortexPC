
#include <unistd.h>
#include <fcntl.h>

#include "lex.h"

/* These operations cover the "Strings" file, as described in Stafford
 * section 3.7.2, pp. 37-38. On page 34 it says that string ID's are
 * two bytes in size and page 38 suggests that the ID is the location
 * at which the string is stored.
 */

int SFile;
int SFilePtr;
int StrStart;

extern char Name[];
extern char Path[];

int
Intern_ident(void)
{
    int len;
    int strID = SFilePtr;

    len = Length(Name) + 1;
    write(SFile, Name, len);
    SFilePtr += len;
    return strID;
}

int
Intern_path( char *path )
{
    int len;
    int strID = SFilePtr;

    len = Length( path ) + 1;
    write(SFile, path, len);
    SFilePtr += len;
    return strID;
}

void
Intern_char(int ch)
{
	char c = ch;

	write(SFile, &c, 1);
	++SFilePtr;
}

void
Intern_start(void)
{
    StrStart = SFilePtr;
    write(SFile, "\0\0", 2);
    SFilePtr += 2;
}

int
Intern_length(void)
{
    int len, id;
    char byte;
    
    Intern_char(0);
    len = SFilePtr - StrStart - 2;
    lseek(SFile, StrStart, SEEK_SET);
    byte = (len >> 8) & 0xff;
    write(SFile, &byte, 1);
    byte = len & 0xff;
    write(SFile, &byte, 1);
    SFilePtr = lseek(SFile, 0, SEEK_END);
    return(StrStart);
}

void
Intern_init(void)
{
    struct Symbol *sym;
    
    SFile = open( "strings", O_WRONLY|O_CREAT|O_TRUNC, 0644 );
    
    /* Frist three entries are null string, .Nargs and .Arg;
     * the well-known offsets are 0, 3 and 10. See Stafford
     * page 38.
     */

    /* The first entry is the empty string, both as zstr and as lstr */
    Intern_char(0);
    Intern_char(0);
    Intern_char(0);
    
    Copy(".Nargs", Name);
    sym = Sym_lookup(TRUE);
    sym->type = IDENTIFIER;
    sym->loc = Intern_ident(); 
    
    Copy(".Arg", Name);
    sym = Sym_lookup(TRUE);
    sym->type = IDENTIFIER;
    sym->loc = Intern_ident(); 
}

