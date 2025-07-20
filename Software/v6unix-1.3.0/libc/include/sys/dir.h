/* directory parameters */

#ifndef _SYS_DIR_H

struct direct {
	short d_ino;			/* inode number */
	char d_name [14];		/* file name */
};

#define DIRSIZ    16

#define MAXNAMLEN 14 

#endif

