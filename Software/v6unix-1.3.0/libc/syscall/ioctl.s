//
// ioctl() system call interface
//
	.globl	cerror, _ioctl
	ioctl = 49

_ioctl:
	sys	ioctl
	b	@cerror


