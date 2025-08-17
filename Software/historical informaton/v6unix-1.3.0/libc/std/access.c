#include <sys/types.h>
#include <sys/stat.h>

int
access(name, mode)
	char *name;
{
	struct stat foo;

	if (stat (name, &foo) < 0)
		return -1;
	if (foo.st_mode & mode == mode)
		return 0;
	return -1;
}
