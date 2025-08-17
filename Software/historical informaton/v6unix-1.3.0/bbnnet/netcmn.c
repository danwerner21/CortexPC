#include "net.h"
#include "mbuf.h"
#include "ip.h"
#include "ifcb.h"
#include "ucb.h"
#include "tcp.h"
#include "fsm.h"

/* common variables */

/* Hosts */
struct host hosts[NHOST], *host, *hostNHOST;
int nhost;

/* TCP Work queues */
struct work works[NWORK], *workfree, *workNHOST, *workNWORK;
int nwork;

/* MBUFs */
struct mbuf  mbuffers[NMBUF+1];
struct mbuf* mbuf_free;

/* I/F control blocks */
struct ifcb ifcb[NIFCB];

/* -> gateway table */
struct gway gates[NGATE], *gateway, *gateNGATE;
int ngate;

/* ->start of proto hdr table */
struct proto ***protab;
int nproto;

struct net netcb;
struct net_stat netstat;

