
#define SECTOR_SIZE   256
#define SECTOR_WORDS (SECTOR_SIZE / 2)
#define SECTSZ_LOG2   9

struct Disk {
    int size;
    int first;
} Geom;

struct FCB {
    short   index;
    short   pos_blk;
    short   disc_blk;
    short   flags;
    struct Mem_node *node;
    char    *buffer;
} FCB;

struct Disc_node {
    char    Name[32];
    short   Size_blk;
    short   Size_idx;
    short   father;
    short   son;
    short   brother;
} *Node;

#define BASE sizeof(struct Disc_node)

struct Mem_node {
    char    Name[32];
    short   Size_blk;
    short   Size_idx;
    short   father;
    short   son;
    short   brother;
    short   self;
    short   refcount;
    short   flags;
} *Cur_node, *Root_node;

extern int dsk;
extern char buf[];

#define FLUSH 0
#define CLOSE 1

#define FAIL  -1
#define ERROR  0
#define OK     1

#define WRITTEN_ON 1

// Compiled little endian, TI990 is big-endian
#define SWAP(s) ((s&0xff)<<8)|((s&0xff00)>>8)

// driver
void Disc_init( void );
int  Disc_read(int block, char* buffer);
int  Disc_write(int block, char* buffer);

// map ops
void Get_map(int sec);
int  Alloc_block( void );
int  Next_block(int block, int alloc);
void Free_chain(int block);

// node cache
struct Mem_node *Get_node(int block);
int Put_node( struct Mem_node *mnode, int op );

// node ops
struct Mem_node *Seek_node( char *path, int *creatable );
int Remove_node(char *path);
int Make_node(char *path);

// file api
void Seek( struct FCB *fcb, int offset, int how );
int  Where( struct FCB *fcb );
int  Get( void );
void Put(char byte);
struct FCB *Open( char *path, char *mode );
void Close(struct FCB *fcb);
