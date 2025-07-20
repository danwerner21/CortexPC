// Unix V6 system call
// C interface: rc = write(fileno, buf, count)
// AS interface: fileno in (sp), buf in @2(sp),
//		 count in @4(sp), rc returned in r0
//
	.globl	cerror
	write = 4

	.globl	_write
_write:
	sys	write
	b	@cerror

