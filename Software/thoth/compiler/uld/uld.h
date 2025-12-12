
#define TRUE  1
#define FALSE 0

/* uld.c */
#define PASS_ONE 1
#define PASS_TWO 2

struct Files {
    struct Files *link;
    char *path;
    char type;
    struct Symbol *modules;
};

extern int IFile;
extern unsigned char *Memory;
extern int flagv, flagl;

int Get( void );

/* link.c */
extern int Rbr[8];
extern struct Symbol *Wsd[256];
extern struct Symbol *Module;

void Pass_one( void );
void Pass_two( void );
void Rbr_rebase( void );

/* symtab.c */
#define DEFINED (0x8000)
#define PENDING (0x4000)

struct Symbol {
    struct Symbol *link;
    struct Symbol *local;
    int Rbr;
    int value;
    char name[1];
};

int   Length(char *s);
int   Equal(char *s1, char *s2);
char *Copy(char *src, char *dst);

struct Symbol* Sym_alloc( char *name );
struct Symbol* Global_lookup( void *nm, int insert );
struct Symbol* Local_lookup( void *nm );
void Sym_rebase( int rbr, int base );
void Sym_dump( void );

/* rdlib.c */
void Scan_lib( struct Files *file );

/* mdex.c */
void Gen_listing( void );
void Gen_MDEX( void );

/* disas.c */
int dasm_one(char *buf, int addr);
