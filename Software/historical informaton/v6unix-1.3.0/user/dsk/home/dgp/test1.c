#include <unistd.h>
#include <sgtty.h>

int
main()
{
	int rc;
	struct sgttyb ttyb;

	rc = gtty(0, &ttyb);
	printf ("gtty: rc = %d\n", rc);
	printf ("ispeed = %d\n", ttyb.sg_ispeed);
	printf ("ospeed = %d\n", ttyb.sg_ospeed);
	printf ("erase  = 0%o\n", ttyb.sg_erase);
	printf ("kill   = 0%o\n", ttyb.sg_kill);
	printf ("flags  = 0%o\n", ttyb.sg_flags);
}
