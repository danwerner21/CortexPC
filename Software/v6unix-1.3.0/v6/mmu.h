/*
 * Cortex MMU registers
 */

#ifdef TI_CTX
/* 'mmu' is set to 0xfe40 by mmu.s */
extern unsigned char mmu[16];
extern char sysmap[];
extern char usrmap[];
#else
extern unsigned int sysmap[];
extern unsigned int usrmap[];
#endif


#define NOWR (0x80)
#define REMAP(a,p)	(char*)((unsigned int)(a)&0x0fff|((p)<<12))

#ifdef KERNEL
int memget();
void memfree();
unsigned int swpget();
void swpfree();
unsigned int physpage();
#endif
