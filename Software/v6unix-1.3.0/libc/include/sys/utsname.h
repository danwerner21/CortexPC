#ifndef __UTSNAME_H__
#define __UTSNAME_H__

#include <ansidecl.h>

struct utsname {
	char sysname[33];
	char nodename[33];
	char release[33];
	char version[33];
	char machine[33];
};

extern int uname PARAMS((struct utsname*));
#endif
