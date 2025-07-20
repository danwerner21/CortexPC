// Unix V6 system call
//
	.globl	cerror
	stat = 18

	.globl	_stat
_stat:
	sys	stat
	b	@cerror

