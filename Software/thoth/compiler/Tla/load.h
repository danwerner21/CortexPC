
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

extern int IFile;
extern int OFile;

#define WSD_DATA        0x01
#define WSD_CODE        0x02
#define WSD_STRING      0x03
#define WSD_CONSTANT    0x04
#define WSD_LAST        0xff

extern int *Counter;
extern int Counter_level;

int  Set_loc(int segment);
int  Next_Wsd( void );
void Clean(void);
