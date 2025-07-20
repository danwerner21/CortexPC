#include "param.h"
#include "buf.h"
#include "conf.h"

/*
 * In case console is off,
 * panicstr contains argument to last
 * call to panic.
 */

char	*panicstr;

/*
 * Scaled down version of C Library printk.
 * Only %s %l %d %x (==%l) %o are recognized.
 * Used to print diagnostic information
 * directly on console tty.
 * It is not interrupt driven.
 * printk should not be used for chit-chat.
 */
printk(fmt,x1)
char fmt[];
{
	register char *s;
	register int *adx, c;

	adx = &x1;
loop:
	while((c = *fmt++) != '%') {
		if (c == '\n') {
			kputc('\r');
		}
		if(c == '\0') {
			return;
		}
		kputc(c);
	}
	c = *fmt++;
	if(c == 'x')
		printx(*adx);
	else if(c == 'd' || c == 'l' || c == 'o')
		printn(*adx, c=='o'? 8: 10);
	else if(c == 's') {
		s = (char*) *adx;
		while(c = *s++)
			kputc(c);
	}
	adx++;
	goto loop;
}

/*
 * Print an unsigned integer in base b.
 */
printn(n, b)
	unsigned int n,b;
{
	register unsigned int a;

	if(a = n/b)
		printn(a, b);
	kputc(n%b + '0');
}
/*
 * Print an unsigned integer in hex.
 */
printx(n)
	unsigned int n;
{
	register unsigned int a;

	if(a = n >> 4)
		printx(a);
	n = n & 0xF;
	n += '0';
	if (n > '9')
	   n += 7;
	kputc(n);
}

/*
 * Panic is called on unresolvable
 * fatal errors.
 * It syncs, prints "panic: mesg" and
 * then loops.
 */
panic(s)
char *s;
{
	panicstr = s;
	update();
	printk("panic: %s\n", s);
	splx(0);
	for(;;)
		idle();
}

/*
 * prdev prints a warning message of the
 * form "mesg on dev x/y".
 * x and y are the major and minor parts of
 * the device argument.
 */
prdev(str, dev)
{
	printk("%s on dev %d/%d\n", str, MAJOR(dev), MINOR(dev));
}

/*
 * deverr prints a diagnostic from
 * a device driver.
 * It prints the device, block number,
 * and an octal word (usually some error
 * status register) passed as argument.
 */
deverror(bp, o1, o2)
struct buf *bp;
{
	register struct buf *rbp;

	rbp = bp;
	prdev("err", rbp->b_dev);
	printk("bn%l err 0x%x 0x%x\n", rbp->b_blkno, o1, o2);
}
