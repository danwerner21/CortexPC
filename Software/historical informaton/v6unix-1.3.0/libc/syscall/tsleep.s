// Unix V6 system call
//
	.globl	cerror
	tsleep = 29

	.globl	_tsleep
_tsleep:
	sys	tsleep
	b	@cerror

