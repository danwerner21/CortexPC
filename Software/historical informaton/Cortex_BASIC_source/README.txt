The Cortex ROM can be built from source.  The source files build an exact image of the EPROMs that were supplied with the Cortex.  In its simplest form, to build the ROM Image file 'rom.bin', perform the following steps:

1) Unzip all files into a directory (READ LAST PARAGRAPH IF YOU WANT TO CHANGE ANYTHING).
2) Open a DOS prompt and change directory to where the files were unzipped.
3) Execute the 'clean' command.  This will delete everything but the source and build files.
4) Execute the 'asmall' command.  This will produce object files from the assembler source files.
6) Execute the 'link' command.  This will link all source objects into one big object file.
7) Execute the 'mkrom' command.  This will produce the (EP)ROM image file 'rom.bin'.

'rom.bin' is the same as CORTEX.BIN that is used by the Emulator.  This can also be used by an EPROM Programmer to make a set of EPROMs for a Cortex.

If you modify a source file (e.g. vdp.asm), instead of using 'asmall', you can assemble it with the command:

	asm vdp 
	
The 'asmall.bat' file contains additional assembler options so that the listed output files (*.lst) match those that were in the original 1982 Cortex ROM Source listing.  These are not necessary to make modifications and are only included for historical reasons.

One final point.  When the Cortex/Emulator is fired up, it copies the EPROMs into RAM and performs a Checksum test.  The checksum is hard-coded into the source file 'text.asm' (see label CHKWRD).  If any source is altered, the checksum will be different and hence the EPROM image won't load.  The easiest way to overcome this is change the conditional jump instruction to an unconditional jump.  See below for the relevant code from 'text.asm'.  Change the fourth from last instruction from JEQ RAMST to JMP RAMST.  Also, a lot of programs were written that referenced specific memory addresses.  If the size of the EPROM image is changed, it is pretty likely that these programs will stop working!  
	

************************************************************
*                                                          *
*      CHECKSUM THE EPROMS AND GENERATE A SYSTEM           *
*      CRASH IF THIS FAILS. THIS ALSO COPIES THE           *
*      EPROMS INTO RAM.                                    *
*                                                          *
************************************************************
START  EQU  $                 RESET ENTRY POINT
       CLR  R1                START FROM 0
       LI   R2,CHKWRD         POINT TO THE CHECKWORD
       MOV  R2,R0             INITIALISE CHECKSUM
L0     MOV  R1,R4             SAVE THE COPY POINTER
       ANDI R4,>FFFE          KILL LS BIT
       C    R4,R2             ARE WE AT THE CHECKWORD?
       JEQ  L2                Y, DONT ADD IT IN
       AB   *R1,@WP10L        ADD INTO R0 LSB
       JOP  L1                ODD PARITY, MODIFY CHECKWORD
       XOR  R1,R0             MODIFY THE CHECKSUM
L1     SRC  R0,0              FRIG THE CHECKWORD
L2     MOVB *R1,*R1+          COPY THE ROM WORD INTO RAM
       CI   R1,>6000          DONE?
       JL   L0                N, LOOP
       CLR  R1
       ASMIF PIO
       LI   R12,2*PIO         POINT TO PARALLEL I/O
       ASMELS
       CLR  R12
       ASMEND
       C    R0,*R2            DOES IT MATCH?
       JEQ  RAMST             Y, GO TURN ROM OFF <<<<<<<          *** CHANGE THIS INSTRUCTION TO 'JMP  RAMST' ***
       SBO  BELLON-PIO        N, TURN THE BELL ON
       B    @CRASH$               GO DIE!!!!
CHKWRD DATA >1B70             REV 1.1  CHECSUM WORD
