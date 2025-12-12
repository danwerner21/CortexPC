
/* load.c */

void Load_word(int data);
void Rload_word(int Rd_index, int Wsd_index, int data);
void Flush_load_bufs(void);
void Put_M_dir(int Rbr, int multiplier, char *module_name);
void Put_load_ref(int type, int Wsd_index, char *symbol_name);
void Put_load_def(int type, int Rbr, int offset, char *name);
void Put_I_dir(int Rbr, int increment);
void Put_A_dir(int Rbr, int multiplier);
void Put_load_dir(char *directive);
void Set_load_L_addr(int Wsd_index, int address);
void Load_byte(int data);

/* util.c */

int  Get(void);
int  Get_constant(int c);
void Get_name(int id, char *name);
int  Get_id(void);
void Error(char *msg);
void Copy_string(int x, int y);
void Initialize(int argc, char **argv);

extern int *Counter;
extern int  Counter_level;
extern int  Last_data_ref;
extern int refno;
extern int OFile;

void  Pop_loc(void);
void  Push_loc(void);
void  Pointer_gen(void);
void  Clean(void);

int Search_wsd(int id, char* name);
char *Create_name(int wsd);
