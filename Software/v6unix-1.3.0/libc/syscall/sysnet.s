//
// Net<->kernel system call interface
//
	.globl	cerror, _sysnet
	sysnet = 45

_sysnet:
	sys	sysnet
	b	@cerror


