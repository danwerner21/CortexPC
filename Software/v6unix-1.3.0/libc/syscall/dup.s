// Unix V6 system call
//
	.globl	cerror
	dup = 41

	.globl	_dup
_dup:
	sys	dup
	b	@cerror

