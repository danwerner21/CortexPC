
#include "param.h"

/*
 * CORE
 */

#define FULL	1
#define FREE	0

char coremap[CMAPSIZ];	/* space for core allocation */
extern int maxslot;

/* Find a free slot in physical memory and return its slot
 * number. Return zero if no free slot could be found.
 */
int
memget()
{
	register int i;
	
	for(i=1; i<maxslot; i++) {
		if(coremap[i]==FREE) break;
	}
	if(i<maxslot) {
		coremap[i] = FULL;
		return i;
	}
	return 0;
}

/* Return memory slot to the free pool */
void
memfree(slot)
{
	coremap[slot] = FREE;
}

/*
 * SWAP
 */

char	swapmap[3*SMAPSIZ];	/* space for swap allocation */

#define PROCSIZ		120	/* 120 blocks = 60KB */
extern daddr_t swplo;		/* start of swap space */

/* Find a free slot in swap space and return the disk address
 * of its first block. Return zero if no free slot could be
 * found. If 'zombie' is non zero space for single block is
 * returned, else space for 60KB is returned.
 */
daddr_t
swpget(zombie)
{
	register int i;

	if(zombie) {
		for(i=SMAPSIZ; i<3*SMAPSIZ; i++) {
			if(swapmap[i]==FREE) break;
		}
		if(i<3*SMAPSIZ) {
			swapmap[i] = FULL;
			return swplo + (SMAPSIZ*PROCSIZ) + i;
		}
		return 0;		
	}
	else {
		for(i=0; i<SMAPSIZ; i++) {
			if(swapmap[i]==FREE) break;
		}
		if(i<SMAPSIZ) {
			swapmap[i] = FULL;
			return swplo + (i*PROCSIZ);
		}
		return 0;
	}
}

/* Return swap slot to the free pool */
void
swpfree(slot)
{
	register daddr_t i;
	
	i = slot - swplo;
	if (i<SMAPSIZ*PROCSIZ )
		i /= PROCSIZ;
	else
		i -= SMAPSIZ*PROCSIZ;
	swapmap[i] = FREE;
}

/* Return the physical page number for address 'a' in the
 * virtual memory space of the process in slot 's'. For the benefit
 * of swapping, virtual page zero maps to the process' u page.
 */
unsigned int
physpage(a, s)
	char *a;
{
	register int i;

	i = (unsigned)a>>12;
	return (i | s<<4);
}
