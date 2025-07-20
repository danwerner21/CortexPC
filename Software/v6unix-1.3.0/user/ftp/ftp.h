#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>

#include "net.h"
#include "con.h"

#define mkanyhost(a) (a).s_addr = 0;
#define isbadhost(a) ((a).s_addr==(-1L) ? 1 : 0)

#define ERRFD   stderr       /* file descriptor for error output */

#define TNDM    242                 /*     function   */
#define TNEC    247
#define TNEL    248
#define TNWILL  251
#define TNWONT  252
#define TNDO    253
#define TNDONT  254
#define TNIAC   255

#define CEOR	0300	/* end of record for mode = text */
#define CEOF	0301	/* end of file for mode = text */

#define FTPTIMO 45
#define FTPSOCK 21

#define U4 0
#define U5 1
#define PORT "port"
#define QUIT "quit"
#define MSRQ "msrq"
#define MRCP "mrcp"
#define MSND "msam"
#define MSOM "msom"

typedef unsigned short portsock;		/* 16-bit data ports */
#define TOSOCK(a) (a)
#define ATOSOCK(a) atoi (a)
/*
 */

struct net_stuff {      /* structure contains useful information which i */
			/* would normally obtain through fstat() (in the */
			/* NCP), or tcp_stat() (in TCP).                 */

     int fds;		/* file-descriptor for the network	*/
     struct con np;	/* net structure			*/
     struct netstate ns;
 };

extern NetInit();
extern net_listen();
extern inherit_net();
extern void ins();
extern net_open();
extern net_close();
extern ftp2_plumber();
extern netaddr GetHostNum();
extern net_read();
extern net_write();

#define netioctl(f,s,l) ioctl((f),(s),(l))
