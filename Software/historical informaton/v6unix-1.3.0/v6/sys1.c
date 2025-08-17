
#include "param.h"
#include "systm.h"
#include "user.h"
#include "proc.h"
#include "buf.h"
#include "reg.h"
#include "inode.h"
#include "file.h"
#include "tty.h"


extern int maxproc;

/*
 * exec system call.
 * Because of the fact that an I/O buffer is used
 * to store the caller's arguments during exec,
 * and more buffers are needed to read in the text file,
 * deadly embraces waiting for free buffers are possible.
 * Therefore the number of processes simultaneously
 * running in exec has to be limited to NEXEC.
 */
#define EXPRI	-1

void
exec()
{
	int na, nc, ds;
	struct buf *bp;
	register int c;
	register struct inode *ip;
	register char *cp;
	int *sp, sep;
	int ost;
	char *up;
	extern int uchar();

	/*
	 * pick up file names
	 * and check various modes
	 * for execute permission
	 */
	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	while(execnt >= NEXEC)
		sleep(&execnt, EXPRI);
	execnt++;
	bp = getblk(NODEV);
	if(access(ip, IEXEC) || (ip->i_mode&IFMT)!=0)
		goto bad;

	/*
	 * pack up arguments into
	 * allocated disk buffer
	 */
	cp = bp->b_addr;
	na = 0;
	nc = 0;
	while(up = (char*) fuword(u.u_arg[1])) {
		na++;
		u.u_arg[1] += 2;
		for(;;) {
			c = fubyte(up++);	/* char from user space */
			*cp++ = c;
			nc++;
			if(nc > 510) {
				u.u_error = E2BIG;
				goto bad;
			}
			if(c == 0)
				break;
		}
	}
	if((nc&1) != 0) {
		*cp++ = 0;
		nc++;
	}

	/*
	 * read in first 12 bytes of file for segment sizes:
	 * w0 = 407/410/411 (410 implies RO text) (411 implies sep ID)
	 * w1 = text size
	 * w2 = data size
	 * w3 = bss size
	 * w4 = symbol table size
	 * w5 = entry point
	 */
	u.u_base = (char*) &u.u_arg[0];
	u.u_count = 12;
	u.u_offset = 0;
	u.u_segflg++;
	readi(ip);
	u.u_segflg--;
	if(u.u_error) {
		goto bad;
	}

	sep = 0;
	if(u.u_arg[0] == 0407) {
		u.u_arg[2] += u.u_arg[1];
		u.u_arg[1] = 0;
	} else {
		if(u.u_arg[0] == 0411)
			sep++;
		else if(u.u_arg[0] != 0410) {
			u.u_error = ENOEXEC;
			goto bad;
		}
	}
	if(u.u_arg[5] != BOTUSR) {
		u.u_error = EINVAL;
		goto bad;		
	}
	if(u.u_arg[1]!=0 && (ip->i_flag&ITEXT)==0 && ip->i_count!=1) {
		u.u_error = ETXTBSY;
		goto bad;
	}

	/*
	 * find text and data sizes
	 * try them out for possible
	 * exceed of max sizes
	 */
	ds = u.u_arg[2]+u.u_arg[3];
	c = nc + na*2 + 6;
	if(ds + SSIZE > (unsigned)UCORE)
		goto bad;

	/*
	 * allocate and clear core
	 * at this point, committed
	 * to the new image
	 */

	/*ost = splx(0);*/
	clearimg(u.u_procp->p_slot);
	/*splx(ost);*/

	/*
	 * read in text + data segment
	 */
	u.u_base   = (char*) u.u_arg[5];
	u.u_offset = 020;
	u.u_count  = u.u_arg[2];
	readi(ip);

	/*
	 * initialize stack segment
	 */
	u.u_dsize = ds;
	u.u_ssize = SSIZE;
	cp = bp->b_addr;
	u.u_ar0[PSP] = TOPUSR - c;
	up = (char*) (TOPUSR - nc);
	sp = (int*) u.u_ar0[PSP];
	suword(sp++, na);
	suword(sp, (int)sp+2);
	sp++;
	while(na--) {
		suword(sp++, (int)up);
		do {
			subyte(up++, *cp);
		} while(*cp++);
	}
	suword(sp, 0);

	/*
	 * set SUID/SGID protections, if no tracing
	 */
	if ((u.u_procp->p_flag&STRC)==0) {
		if(ip->i_mode&ISUID)
			if(u.u_uid != 0) {
				u.u_uid = ip->i_uid;
				u.u_procp->p_uid = ip->i_uid;
			}
		if(ip->i_mode&ISGID)
			u.u_gid = ip->i_gid;
	}

	/*
	 * clear sigs, regs and return
	 */
	for(sp = &u.u_signal[0]; sp < &u.u_signal[NSIG]; sp++)
		if((*sp & 1) == 0)
			*sp = 0;
	u.u_ar0[R0]  = 0;
	u.u_ar0[R1]  = 0;
	u.u_ar0[R2]  = 0;
	u.u_ar0[R3]  = 0;
#ifdef TI_CTX
	u.u_ar0[PWP] = 0x1010; /*0xf000;*/
	u.u_ar0[PPC] = u.u_arg[5]+48; /*16;*/
#else
	u.u_ar0[PWP] = 0x1010;
	u.u_ar0[PPC] = u.u_arg[5]+48;
#endif

bad:
	iput(ip);
	brelse(bp);
	if(execnt >= NEXEC)
		wakeup(&execnt);
	execnt--;
}

/*
 * exit system call:
 * pass back caller's arg0
 */
void
rexit()
{
	/* exit param is already in u.u_arg[0] */
	u.u_arg[0] <<= 8;
	pexit();
}

/*
 * Release resources.
 * Save u. area for parent to look at.
 * Enter zombie state.
 * Wake up parent and init processes,
 * and dispose of children.
 */
int
pexit()
{
	register int *s;
	register struct file **fp;
	register struct proc *p, *q;
	struct buf *bp;
	int i, swpslot;

	p = u.u_procp;
	p->p_flag &= ~STRC;
	for(s = &u.u_signal[0]; s < &u.u_signal[NSIG];)
		*s++ = 1;
	for(i = 0, fp = &u.u_ofile[0]; fp < &u.u_ofile[NOFILE]; i++, fp++)
		if(*fp != NULL) {
			if ((*fp)->f_flag & FNET)
				closen(*fp, i);
			else
				closef(*fp);
			*fp = NULL;
		}
	iput(u.u_cdir);
	update();

	/*xfree(); */
	swpslot = swpget(1);
	if(swpslot == NULL)
		panic("out of swap");
	bp = getblk(swapdev, swpslot);
	memcpy(bp->b_addr, &u, 512);
	bwrite(bp);

	q = u.u_procp;
	memfree(q->p_slot);
	q->p_slot = swpslot;
	q->p_stat = SZOMB;

loop:
	for(p = &proc[0]; p < &proc[maxproc]; p++)
		if(q->p_ppid == p->p_pid) {
			if (p != &proc[1])
				wakeup(&proc[1]);
			wakeup(p);
			for(p = &proc[0]; p < &proc[maxproc]; p++)
				if(q->p_pid == p->p_ppid) {
					p->p_ppid  = 1;
					if (p->p_stat == SSTOP)
						setrun(p);
			}
			swtch();
			/* no return */
		}
	q->p_ppid = 1;
	goto loop;
}

/*
 * Wait system call.
 * Search for a terminated (zombie) child,
 * finally lay it to rest, and collect its status.
 * NOTE: if cpid == maxproc then the status read in is erroneous
 */
void
wait()
{
	register struct buf  *bp;
	register struct proc *p;
	register struct user *up;
	int f;

	f = 0;
loop:
	for(p = &proc[0]; p < &proc[maxproc]; p++)
		if(p->p_ppid == u.u_procp->p_pid) {
			f++;
			if(p->p_stat == SZOMB) {
				u.u_ar0[R0] = p->p_pid;
				bp = bread(swapdev, p->p_slot);
				swpfree(p->p_slot);
				p->p_stat = 0;
				p->p_pid  = 0;
				p->p_ppid = 0;
				p->p_sig  = 0;
				p->p_ttyp = NULL;
				p->p_flag = 0;
				up = (struct user*)bp->b_addr;
				u.u_cstime += up->u_cstime;
				u.u_cstime += up->u_stime;
				u.u_cutime += up->u_cutime;
				u.u_cutime += up->u_utime;
				u.u_ar0[R1] = up->u_arg[0];
				brelse(bp);
				return;
			}
			if(p->p_stat == SSTOP) {
				if((p->p_flag&SWTED) == 0) {
					p->p_flag  |= SWTED;
					u.u_ar0[R0] = p->p_pid;
					u.u_ar0[R1] = (p->p_sig<<8) | 0177;
					return;
				}
				p->p_flag &= ~(STRC|SWTED);
				setrun(p);
			}
		}
	if(f) {
		sleep(u.u_procp, PWAIT);
		goto loop;
	}
	u.u_error = ECHILD;
}

/*
 * fork system call.
 */
void
fork()
{
	register struct proc *p1, *p2;

	p1 = u.u_procp;
	for(p2 = &proc[0]; p2 < &proc[maxproc]; p2++)
		if(p2->p_stat == NULL)
			goto found;
	u.u_error = EAGAIN;
	goto out;

found:
	if(newproc()) {
		u.u_ar0[R0] = p1->p_pid;
		u.u_cstime  = 0;
		u.u_stime   = 0;
		u.u_cutime  = 0;
		u.u_utime   = 0;
		return;
	}
	u.u_ar0[R0] = p2->p_pid;

out:
	u.u_ar0[PPC] += 2;
}

/*
 * break system call.
 *  -- bad planning: "break" is a dirty word in C.
 */
void
sbreak()
{
	register unsigned n, d;

	/*
	 * set n to new data size
	 * set d to new-old
	 * set n to new total size
	 */
	n = ((u.u_arg[0]+1)&~1)-BOTUSR;
	d = n - u.u_dsize;
	if(n > UCORE) {
		u.u_error = E2BIG;
		return;
	}
	u.u_dsize += d;
}

