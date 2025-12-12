
/* general */
#define TRUE			1
#define FALSE			0

/* input.c */
extern int In_string;
extern int In_pushed_back;
extern int Line_no;

int  Get_ind(void);
void In_expand(int);
void Error(char *);
void Put(int token);
void PutW(int value);

/* string.c */
void Intern_init(void);
int  Intern_ident(void);
int  Intern_path( char *path );
void Intern_start(void);
void Intern_char(int ch);
int  Intern_length(void);

/* math.c */
void Const_clr(void);
void Const_put(void);
void Const_add(int val);
void Const_mul(int val);

/* symbol.c */
struct Symbol {
    struct Symbol *link;
    int type;
    int loc;
    char name[1];
};
void Get_ident(void);
struct Symbol* Sym_lookup(int insert);
void Sym_dump(void);

#define IDENTIFIER		0
#define UNDEFINED       -1
#define MANIFEST		255

/* lib.c */
int  Length(char *s);
int  Equal(char *s1, char *s2);
void Copy(char *src, char *dst);

/* token.c */
extern char Ctab[];

void Lex(void);

#define D 0x10
#define L 0x20

