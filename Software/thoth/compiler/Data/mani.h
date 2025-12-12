
#define WORD_LENGTH             16
#define ADDR_LENGTH             16
#define ADDR_UNITS               8
#define EHADDR_FACTOR           (WORD_LENGTH / ADDR_UNITS)
#define BYTE_LENGTH              8
#define LOAD_ADDRLEN            ((ADDR_LENGTH+BYTE_LENGTH-1) / BYTE_LENGTH)
#define MACHADDR_FACTOR         (ADDR_UNITS / BYTE_LENGTH)
#define TARGET_BPW              (WORD_LENGTH / BYTE_LENGTH)
#define BYTE_MASK               (0377)
#define EXTERN_RB               1
#define DATA_RB                 2
#define PTR_RELDESC             3
#define ADDR_RELDESC            1
#define ALIGN_EXTRN             2
//#define ALIGN_EXTERNAL_DATA     2