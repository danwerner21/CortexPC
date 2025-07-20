
/*
 *	Memory special file
 *	minor device 0 is physical memory	/dev/mem
 *	minor device 1 is kernel memory		/dev/kmem
 *	minor device 2 is EOF/RATHOLE		/dev/null
 *	minor device 3 is ZERO/RATHOLE		/dev/zero
 */

#include "param.h"
#include "user.h"
#include "conf.h"
#include "mmu.h"

mmread(dev)
{
	register int c, pg, m;
	register char *ip;

	if(MINOR(dev) == 2)
		return;
	do {
		if (MINOR(dev) == 3)
			c = 0;
		else if(MINOR(dev) == 1 || u.u_offset < 0x10000) {
			ip = (char*)u.u_offset;
			c = *ip;
		}
		else {
#ifdef TI_CTX
			ip = (char*)((u.u_offset & 0x0fff) | 0xd000);
			pg = (u.u_offset >> 12) & 0x7f;
			m = mmu[13];
			mmu[13] = pg;
			c = *((char*)(ip));
			mmu[13] = m;
#else
			ip = (char*)(u.u_offset & 0xffff);
			pg = (u.u_offset >> 16) & 0x3f;
			c = gsbyte (pg, ip);
#endif
		}
		subyte(u.u_base++, c);
		u.u_offset++;
		u.u_count--;
	} while(u.u_error==0 && u.u_count>0);
}

mmwrite(dev)
{
	register int c, pg, m;
	register char *ip;

	if(MINOR(dev) == 2 || MINOR(dev) == 3) {
		c = u.u_count;
		u.u_count   = 0;
		u.u_base   += c;
		u.u_offset += c;
		return;
	}
	for(;;) {
		if (u.u_count<=0 || u.u_error!=0)
			break;
		c = fubyte(u.u_base++);
		u.u_offset++;
		u.u_count--;
		if(MINOR(dev) == 1 || u.u_offset < 0x10000) {
			ip = (char*)u.u_offset;
			*ip = c;
		}
		else {
#ifdef TI_CTX
			ip = (char*)((u.u_offset & 0x0fff) | 0xd000);
			pg = (u.u_offset >> 12) & 0x7f;
			m = mmu[13];
			mmu[13] = pg;
			*((char*)(ip)) = c;
			mmu[13] = m;
#else
			ip = (char*)(u.u_offset & 0xffff);
			pg = (u.u_offset >> 16) & 0x3f;
			psbyte (pg, ip, c);
#endif
		}		
	}
}
