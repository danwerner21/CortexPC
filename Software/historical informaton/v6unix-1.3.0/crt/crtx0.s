// C run time start routine for Cortex cassette file
//

	  .globl _main
	  .text

          ramtop = 0xea00 

start:    stwp      r0
          mov       r0,@wpsav
          lwpi      ramtop
          stwp      sp
          dect      sp
          bl        @_main
          
stop:     mov       @wpsav,r0
          lwp       r0
          b         @0x0080

          .bss
        
wpsav:    .=.+2

