#include <dirent.h>
#include <sys/stat.h>

int
ttyn(fd)
{
	register int name, dev;
	register struct stat buf;
	static struct dirent dirent;
	
	name = 'x';
	if(fstat(fd, &buf)<0)
		return(name);
	if((dev=open("/dev", 0))<0)
		return(name);
	while(read(dev, &dirent, 16)==16) {
		if(dirent.d_ino!=buf.st_ino) continue;
		if(dirent.d_name[4]) continue;
		if(strncmp(dirent.d_name, "tty", 3)) continue;
		name = dirent.d_name[3];
	}
	close(dev);
	return(name);
}

