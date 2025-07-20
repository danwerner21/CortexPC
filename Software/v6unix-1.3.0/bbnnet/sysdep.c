#include <stdio.h>
#include <sys/types.h>

#include "param.h"
#include "inode.h"
#include "sysnet.h"

#include "mbuf.h"
#include "net.h"
#include "ifcb.h"
#include "tcp.h"
#include "ip.h"
#include "ucb.h"
#include "fsm.h"

#include "buf.h"
#include "user.h"
#include "proc.h"
#include "reg.h"
#include "file.h"

netlog(mp, s)
struct mbuf *mp;
char *s;
{
#ifdef DEBUG_LOG
   printf ("ERROR: %s: mbuf = >%04X\n", s, mp);
#endif
   m_freem(mp);
   return;
}

/* OS stubs */

void
wakeup(chan)
{
   sysnet (SYSWAKEUP, chan);
}

void
ksleep(chan, prio)
{
   sysnet (SYSSLEEP, chan, prio);
}

void
nwakeup(chan)
{
   return;
}

void
nsleep(chan, prio)
{
   printf ("nsleep: chan = 0x%x, prio = %d\n", chan, prio);
   sleep(1);
}

void
psignal()
{
   return;
}
