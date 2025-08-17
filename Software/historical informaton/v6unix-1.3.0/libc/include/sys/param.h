/* system parameters */

#ifndef _SYS_PARAM_H

#define MAXPATHLEN  512
#define MAX(x,y) ((x>y)?(x):(y))

#define BSIZE 512
#define BSHIFT 9
#define MAXBSIZE BSIZE

#define NICFREE 100
#define NICINOD 100

#define major(d) (int)(d&0xff)
#define minor(d) (int)((unsigned)(d)>>8)

#define dev_t int
#define ino_t unsigned int
#define off_t unsigned long
#define daddr_t unsigned int
#define time_t  unsigned long

#endif

