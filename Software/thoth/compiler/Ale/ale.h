
/* modules.c */
struct Module {
    int symindex;
    int refcount;
    short *refs;
    char data[4];
};

extern struct Module Modules[];
extern int Next_module;
extern int Next_ref;
extern short References[];

#define MAXMOD  512
#define MAXREFS 2048

void Process_out_file( void );
short *Refs_base(int count);

/* lib.c */
int   Length(char *s);
int   Equal(char *s1, char *s2);
char* Copy(char *src, char *dst);

/* symbols.c */
struct Symbol {
    struct Symbol *link;
    char *name;
    struct Module *module;
    int count;
};

#define MAXSYM  512
extern int Next_symbol;

struct Symbol* Sym_lookup( unsigned char *nm );
struct Symbol* Sym_byindex( int index );
int Sym_index( struct Symbol *sym );

/* ale.c */
extern int OFile, Mode;
extern char Path[];
extern char *Ptr;
extern unsigned char Buf[];

int  Get_command(void);
void Put_command(int cmd, int len);
