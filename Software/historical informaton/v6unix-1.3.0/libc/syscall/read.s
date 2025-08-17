// Unix V6 system call
// C interface: rc = read(fileno, buf, count)
// AS interface: fileno in (sp), buf in @2(sp),
//		 count in @4(sp), rc returned in r0
//
	.globl	cerror
	read = 3

	.globl	_read
_read:
	sys	read
	b	@cerror

