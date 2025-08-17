/*
 * Everything in this file is a routine implementing a system call.
 */
#include "param.h"
#include "user.h"
#include "reg.h"
#include "inode.h"
#include "systm.h"
#include "proc.h"
#include "file.h"

extern int cpuid;
extern int maxproc;

/* read the console switches; no-op on a Cortex */
void
getswit()
{
	u.u_ar0[R0] = 0;
}

void
utsinfo()
{
	struct utsname *uts;

	uts = (struct utsname *)u.u_arg[0];
	sustr (uts->sysname, "Unix");
	sustr (uts->nodename, "");
	sustr (uts->release, "1.3.0");
	sustr (uts->version, "V6");
	switch (cpuid) {
	case 0:
		sustr (uts->machine, "990/10");
		break;
	case 1:
		sustr (uts->machine, "990/12");
		break;
	case 2:
		sustr (uts->machine, "990/10A");
		break;
	case 3:
		sustr (uts->machine, "TMS-9995");
		break;
	default:
		sustr (uts->machine, "unknown");
	}
}

void
gtime()
{
	int *t = (int *)u.u_arg[0];
	suword(t,   ((int*) &time)[0]);
	suword(t+1, ((int*) &time)[1]);
}

void
stime()
{
	register struct proc *pp;
	int *t;

	if(suser()) {
		t = (int *)u.u_arg[0];
		((int*) &time) [0] = fuword(t);
		((int*) &time) [1] = fuword(t+1);
		for(pp = &proc[0]; pp < &proc[maxproc]; pp++) {
			/* Wake up any sleepers */
			if(pp->p_tout)
				wakeup(&pp->p_tout);
		}
	}
}

void
setuid()
{
	char uid;

	uid = u.u_arg[0];
	if(u.u_ruid == uid || suser()) {
		u.u_uid = uid;
		u.u_procp->p_uid = uid;
		u.u_ruid = uid;
	}
}

void
getuid()
{
	u.u_ar0[R0] = (u.u_uid<<8) + u.u_ruid;
}

void
setgid()
{
	char gid;

	gid = u.u_arg[0];
	if(u.u_rgid == gid || suser()) {
		u.u_gid = gid;
		u.u_rgid = gid;
	}
}

void
getgid()
{
	u.u_ar0[R0] = (u.u_gid<<8) + u.u_rgid;
}

void
getpid()
{
	u.u_ar0[R0] = u.u_procp->p_pid;
}

void
sync()
{
	update();
}

void
nice()
{
	register n;

	n = u.u_arg[0];
	if(n > 20)
		n = 20;
	if(n < 0 && !suser())
		n = 0;
	u.u_procp->p_nice = n;
}

void
unlink()
{
	register struct inode *ip, *pp;
	extern int uchar();

	pp = namei(&uchar, 2);
	if(pp == NULL)
		return;
	prele(pp);
	ip = iget(pp->i_dev, u.u_dent.u_ino);
	if(ip == NULL)
		panic("unlink -- iget");
	if((ip->i_mode&IFMT)==IFDIR && !suser())
		goto out;
	u.u_offset -= DIRSIZ+2;
	u.u_base = (char*) &u.u_dent;
	u.u_count = DIRSIZ+2;
	u.u_dent.u_ino = 0;
	u.u_segflg++;
	writei(pp);
	u.u_segflg--;
	ip->i_nlink--;
	ip->i_flag |= IUPD;
	iput(ip);
out:
	iput(pp);
	u.u_ar0[R0] = 0;
}

void
chdir()
{
	register struct inode *ip;
	extern int uchar();

	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	if((ip->i_mode&IFMT) != IFDIR) {
		u.u_error = ENOTDIR;
	bad:
		iput(ip);
		return;
	}
	if(access(ip, IEXEC))
		goto bad;
	iput(u.u_cdir);
	u.u_cdir = ip;
	prele(ip);
}

void
chmod()
{
	register struct inode *ip;

	if ((ip = owner()) == NULL)
		return;
	ip->i_mode &= ~07777;
	if (u.u_uid)
		u.u_arg[1] &= ~ISVTX;
	ip->i_mode |= u.u_arg[1]&07777;
	ip->i_flag |= IUPD;
	iput(ip);
}

void
chown()
{
	register struct inode *ip;

	if (!suser() || (ip = owner()) == NULL)
		return;
	ip->i_uid = u.u_arg[1];
	ip->i_gid = u.u_arg[1]>>8;
	ip->i_flag |= IUPD;
	iput(ip);
}

void
utime()
{
	register struct inode *ip;
	int *t;
	int tbuf[2];

	if ((ip = owner()) == NULL)
		return;
	ip->i_flag |= IUPD;
	t = (int *)u.u_arg[1];
	tbuf[0] = fuword(t+2);	/* Only use the mtime */
	tbuf[1] = fuword(t+3);
	iupdat(ip, &tbuf);
	ip->i_flag &= ~IUPD;
	iput(ip);
}

void
ssig()
{
	register int a;

	a = u.u_arg[0];
	if(a<=0 || a>=NSIG || a ==SIGKIL) {
		u.u_error = EINVAL;
		return;
	}
	u.u_ar0[R0] = u.u_signal[a];
	u.u_signal[a] = u.u_arg[1];
	if(u.u_procp->p_sig == a)
		u.u_procp->p_sig = 0;
}

void
kill()
{
	register struct proc *p, *q;
	register a;
	int f;

	f = 0;
	a = u.u_arg[0];
	q = u.u_procp;
	for(p = &proc[0]; p < &proc[maxproc]; p++) {
		if(p == q)
			continue;
		if(a != 0 && p->p_pid != a)
			continue;
		if(a == 0 && (p->p_ttyp != q->p_ttyp || p <= &proc[1]))
			continue;
		if(u.u_uid != 0 && u.u_uid != p->p_uid)
			continue;
		f++;
		psignal(p, u.u_arg[1]);
	}
	if(f == 0)
		u.u_error = ESRCH;
}

void
times()
{
	register int *p;

	for(p = &u.u_utime; p  < &u.u_utime+6;) {
		suword(u.u_arg[0], *p++);
		u.u_arg[0] += 2;
	}
}

/* copy of u.u_proc[], used by low-level clock routine */
extern int uprof[];

void
profil()
{

	u.u_prof[0] = u.u_arg[0] & ~1;		/* base of sample buf */
	u.u_prof[1] = u.u_arg[1];		/* size of same */
	u.u_prof[2] = u.u_arg[2];		/* pc offset */
	u.u_prof[3] = (u.u_arg[3]>>1) & 077777;	/* pc scale */

	uprof[0] = u.u_prof[0];
	uprof[1] = u.u_prof[1];
	uprof[2] = u.u_prof[2];
	uprof[3] = u.u_prof[3];
}
