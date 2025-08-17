/*
 * Configuration file, hand crafted for now.
 * Two character devices, one block device
 */

#include "param.h"
#include "conf.h"
#include "buf.h"

int acaopen();
int acaclose();
int acaread();
int acawrite();
int acasgtty();

#ifndef TI_CTX
int c403open();
int c403close();
int c403read();
int c403write();
int c403sgtty();

int mtopen();
int mtclose();
int mtread();
int mtwrite();

int prtopen();
int prtclose();
int prtwrite();

int v911open();
int v911close();
int v911read();
int v911write();
int v911sgtty();

int netread();
int netwrite();
int netcontrol();
#endif

int nulldev();
int mmread();
int mmwrite();
int nodev();

int cfread();
int cfwrite();
int cfstrategy();
extern struct devtab cftab;

struct cdevsw cdevsw[NCHRDEV] = {
	/* Major 0 = 9902/EIA ACA Console */
	{ &acaopen,  &acaclose,  &acaread,  &acawrite,  &acasgtty  },
	/* Major 1 = Memory */
	{ &nulldev,  &nulldev,   &mmread,   &mmwrite,   &nodev     },
	/* Major 2 = Direct access */
	{ &nulldev,  &nulldev,   &cfread,   &cfwrite,   &nodev     },
#ifndef TI_CTX
	/* Major 3 = Network */
	{ &nulldev,  &nulldev,   &netread,  &netwrite,  &netcontrol}, 
	/* Major 4 = CI403 Serial Multiplexor */
	{ &c403open, &c403close, &c403read, &c403write, &c403sgtty },
	/* Major 5 = Mag tape */
	{ &mtopen,   &mtclose,   &mtread,   &mtwrite,   &nodev     },
	/* Major 6 = Printer */
	{ &prtopen,  &prtclose,  &nodev,    &prtwrite,  &nodev     },
	/* Major 7 = 911 VDT */
	{ &v911open, &v911close, &v911read, &v911write, &v911sgtty },
#endif
	/* Sentinel */
	{ 0,         0,          0,         0,          0          }
};

struct bdevsw bdevsw[2] = {
	/* Major 0 = Disk device */
	{ &nulldev, &nulldev, &cfstrategy, &cftab },
	/* sentinel */
	{ 0,        0,        0,           0      }
};
