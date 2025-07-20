// Unix V6 system call
//
	.globl	cerror
	profil = 44

	.globl	_profil
_profil:
	sys	profil
	b	@cerror

