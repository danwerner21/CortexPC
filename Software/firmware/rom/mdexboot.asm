;
        AORG >7800
*        BOOT loader for MDEX on the 9995 breadboard with a CF Card
*
*        SYMBOLIC NAMES
*
*        DEFINES for the CF card registers
CFREGS          EQU >fe00
CFDATA          EQU CFREGS+0
CFERR           EQU CFREGS+1                      ; rd
CFFEAT          EQU CFREGS+1                      ; wr
CFCNT           EQU CFREGS+2
CFLBA0          EQU CFREGS+3
CFLBA1          EQU CFREGS+4
CFLBA2          EQU CFREGS+5
CFLBA3          EQU CFREGS+6
CFSTAT          EQU CFREGS+7                      ; rd
CFCMD           EQU CFREGS+7                      ; wr

;        DEFINES for CF card commands in upper byte
SETFEAT         EQU >ef00                         ; set feature
RDSECT          EQU >2000                         ; read sector
WRSECT          EQU >3000                         ; write sector
IDENT           EQU >ec00                         ; identify drive
BYTEMOD         EQU >0100                         ; 8 bit access mode

;        OFFSETS in partition table records
PTYPE           EQU 4                             ; type offset
START           EQU 8                             ; START sector offset
;
;        VARIABLES
;
SECBUF          EQU >9000                         ; sector read buffer
TMP             EQU >ff80                         ; temp var
CLUSLBA         EQU >ff82                         ; LBA of FAT cluster #2
;
;       BOOT LOADER START
;
;       .TEXT
BEGIN
        LWPI                                      >f000
        B                                         @MAIN

;        GET a (possibly unaligned) word value and flip endian
;        R2 points to value, return in R0
;
GET16
TAG0
        MOVB                                      *R2+,R0
        SWPB                                      R0
        MOVB                                      *R2,R0
        B                                         *R11

;        GET a (possibly unaligned) double word and flip endian
;        R2 points to value, return in R0,R1
;
GET32
        MOVB                                      *R2+,R1
        SWPB                                      R1
        MOVB                                      *R2+,R1
        JMP                                       TAG0

;        READ a sector into the buffer, LBA sector # in R0,R1
;        R0,R1 are destroyed
;
CFREAD
TAG1
        MOVB                                      @CFSTAT,@TMP		; wait for card ready
        JLT                                       TAG1
        MOVB                                      R1,@CFLBA1		; write the LBA address
        SWPB                                      R1
        MOVB                                      R1,@CFLBA0
        ANDI                                      R0,>0fff
        ORI                                       R0,>e000
        MOVB                                      R0,@CFLBA3
        SWPB                                      R0
        MOVB                                      R0,@CFLBA2
        LI                                        R0,>0100		; set sector count to 1
        MOVB                                      R0,@CFCNT
        LI                                        R0,RDSECT		; read sector
        MOVB                                      R0,@CFCMD
TAG2
        MOVB                                      @CFSTAT,R0		; wait for card ready
        JLT                                       TAG2
        LI                                        R0,SECBUF		; read data into SECBUF
        LI                                        R1,512
TAG3
        MOVB                                      @CFDATA,*R0+
        DEC                                       R1
        JNE                                       TAG3
        B                                         *R11			; done

;        CONVERT a FAT32 cluster number in R0,R1 to an LBA sector number in R0,R1
;        R2 is destroyed
;
CLUS2LBA
        DECT                                      R1			; clus = clus - 2
        JOC                                       TAG4
        DEC                                       R0
TAG4
        LI                                        R2,3			; sec = clus * 8
TAG5
        SLA                                       R0,1
        SLA                                       R1,1
        JNC                                       TAG6
        INC                                       R0
TAG6
        DEC                                       R2
        JNE                                       TAG5
        A                                         @CLUSLBA,R0		; lba = sec + CLUSLBA
        A                                         @CLUSLBA+2,R1
        JNC                                       TAG7
        INC                                       R0
TAG7
        B                                         *R11

;        DOES this FAT32 directory entry match? If so, fetch START sector#
;        R2 points to entry, R3 points to 8.3 filename
;        IF it is a match, LBA of file is in R0,R1 and EQ is set
;        R2-R4,R12 are destroyed in all cases
;        ON no match, EQ is reset and R0,R1 are unchanged
;
FATTEST
        MOV                                       R11,R12
        CLR                                       R4			; attribute is >00 or >20?
        MOVB                                      @11(R2),R4
        JEQ                                       TAG8
        CI                                        R4,>2000
        JNE                                       TAG10			; no
TAG8
        LI                                        R4,11			; yes; does name match?
TAG9
        CB                                        *R2+,*R3+
        JNE                                       TAG10			; no
        DEC                                       R4
        JNE                                       TAG9
; we have a match
        AI                                        R2,15			; move to clus_lo field
        BL                                        @GET16
        MOV                                       R0,R1
        AI                                        R2,-7			; move to clus_hi field
        BL                                        @GET16
        BL                                        @CLUS2LBA		; cluster => sector
        C                                         R0,R0			; set EQ flag
TAG10
        B                                         *R12

;        DOES this MDEX directory entry match 'BOOT$.SAV'? If so fetch START sector#
;        R2 points to entry
;        IF it is a match, MDEX sector # is in R3 and EQ is set
;        R2-R4 are destroyed in all cases, and EQ is reset on failure
;
ISBOOT
        LI                                        R4,12			; 12 characters in mdex file name
        LI                                        R3,BOOTSAV		; 'BOOT$.SAV   '
TAG11
        CB                                        *R2+,*R3+
        JNE                                       TAG12
        DEC                                       R4
        JNE                                       TAG11
        MOV                                       *R2,R3			; found 'BOOT$.SAV', get first sector#
        C                                         R0,R0			; set EQ flag
TAG12
        B                                         *R11

;        READ a sector from mdex FD0 image
;        R1 holds the mdex sector number
;        R0-R1 are destroyed
RD_MDEX
        MOV                                       @>ff00,R0		; add in the LBA of FD0 sector 0
        A                                         @>ff02,R1
        JNC                                       TAG13
        INC                                       R0
TAG13
        B                                         @CFREAD			; read the sector

;        REPORT a fatal ERROR
;
ERROR
        XOP                                       @ERRMSG,14		; print ERROR message
        B                                         @>0142			; return to EVMBUG

PROGRESS
        DATA >F020                                ; bar
        DATA >78F0

BAR
        LI                                        R0,'>'*256
        XOP                                       R0,12
        RTWP

;
;        HERE is the START of the main program. It has three phases:
;        1. Init the CF card
;        2. Read the FAT32 file system and find the location of the FD0 and FD1 images
;        3. In the FD0 image, find 'BOOT$.SAV' in the MDEX file system and load it
;
;        INIT CF card, set 8-bit mode
MAIN
TAG14
        MOVB                                      @CFSTAT,R0		; wait for card ready
        JLT                                       TAG14
        MOVB                                      @ZERO,@CFLBA3		; set 8 bit access mode
        LI                                        R0,BYTEMOD
        MOVB                                      R0,@CFFEAT
        LI                                        R0,SETFEAT
        MOVB                                      R0,@CFCMD

        BLWP                                      @PROGRESS		; >

;        READ the Master Boot Record (contains the Partition Table)
;
        CLR                                       R0			; read sector 0, the MBR
        CLR                                       R1
        BL                                        @CFREAD
        LI                                        R2,SECBUF+>1be		; point R2 at partition table
        LI                                        R3,4			; four entries
        CLR                                       R0
TAG15
        MOVB                                      @PTYPE(R2),R0
        CI                                        R0,11*256		; FAT32 is type 11 or 12
        JEQ                                       TAG16
        CI                                        R0,12*256
        JEQ                                       TAG16
        DEC                                       R3			; next partition of 4
        JEQ                                       ERROR
        AI                                        R2,16
        JMP                                       TAG15
TAG16
        AI                                        R2, START		; found FAT32 partition, read START
        BL                                        @GET32			;  sector

        BLWP                                      @PROGRESS		; >>

;        READ the FAT32 boot block
;
        MOV                                       R0,@CLUSLBA		; save base sector #
        MOV                                       R1,@CLUSLBA+2
        BL                                        @CFREAD			; read boot block
        LI                                        R2,SECBUF+14
        BL                                        @GET16			; add reserved sectors to base
        A                                         R0,@CLUSLBA+2
        JNC                                       TAG17
        INC                                       @CLUSLBA
TAG17
        CLR                                       R0			; assert that there are 2 FAT copies
        MOVB                                      @SECBUF+16,R0
        CI                                        R0,>0200
        JNE                                       ERROR
        LI                                        R2,SECBUF+36		; get FAT table length in sectors
        BL                                        @GET32
        SLA                                       R0,1			; multiply by 2
        SLA                                       R1,1
        JNC                                       TAG18
        INC                                       R0
TAG18
        A                                         R0,@CLUSLBA		; add FAT length * 2 to base
        A                                         R1,@CLUSLBA+2
        JNC                                       TAG19
        INC                                       @CLUSLBA
TAG19
        CLR                                       R0			; fetch sectors per cluster
        MOVB                                      @SECBUF+13,R0
        CI                                        R0,>0800		; assert 8 sectors/cluster
        JNE                                       ERROR
        LI                                        R2,SECBUF+44
        BL                                        @GET32			; fetch root dir START cluster

        BLWP                                      @PROGRESS		; >>>

;        READ the first sector of the FAT32 root directory
;
        BL                                        @CLUS2LBA		; convert to sector lba
        BL                                        @CFREAD			; read root dir first sector
        LI                                        R6,16			; 16 entries
        CLR                                       R7			; no matches yet
        LI                                        R5,SECBUF		; point to first entry
TAG20
        MOV                                       R5,R2
        LI                                        R3,FD0IMG
        BL                                        @FATTEST
        JNE                                       TAG21
        MOV                                       R0,@>ff00		; found 'MDEX_FD0.DSK' image
        MOV                                       R1,@>ff02
        JMP                                       TAG22
TAG21
        MOV                                       R5,R2
        LI                                        R3,FD1IMG
        BL                                        @FATTEST
        JNE                                       TAG23
        MOV                                       R0,@>ff04		; found 'MDEX_FD1.DSK' image
        MOV                                       R1,@>ff06
TAG22
        INC                                       R7
        CI                                        R7,2			; have we found both?
        JEQ                                       READ_MDEX		; yes, now read FD0 contents
TAG23
        AI                                        R5,32			; no, move to next dir entry
        CI                                        R5,SECBUF+512		; past end?
        JLT                                       TAG20			; no
        B                                         @>0142
        B                                         @ERROR			; yes, signal an ERROR (todo: read full dir)

;        NOW read the directory of MDEX_FD0 and find the BOOT$.SAV file location
;
READ_MDEX
        BLWP                                      @PROGRESS		; >>>>

        CLR                                       R4			; START at image sector 0
TAG24
        MOV                                       R4,R1			; read sector
        BL                                        @RD_MDEX
        LI                                        R5,7			; 7 entries per 128 byte sector
        LI                                        R6,SECBUF
TAG25
        MOV                                       R6,R2
        C                                         *R2,@ENDDIR		; >ffff marker? => not bootable
;;;;    vvvvvvvvvvvvvvvvvv BJEQ ERROR	; *****ddw WAS BJEQ, CHANGED TO JEQ SO IT WOULD ASSEMBLE****
        DATA >1602
        DATA >0460
        DATA >78E4
;;;;   ^^^^^^^^^^^^^^^^^^ totally need to figure this out . . . looks like a macro of some kind
        BL                                        @ISBOOT			; is this 'boot$.sav'?
        JEQ                                       LOADSYS			; yes, load the file
        DEC                                       R5			; more entries in this sector?
        JEQ                                       TAG26			; no, read next sector
        AI                                        R6,18			; yes, look at next entry
        JMP                                       TAG25
TAG26
        INC                                       R4			; increment image sector number
        JMP                                       TAG24			; go back and load/scan next sector

;        FINALLY, read the BOOT$.SAV file and transfer control to the MDEX kernel
;
LOADSYS
        BLWP                                      @PROGRESS		; >>>>>

        MOV                                       R3,R1			; load first sector of 'boot$.sav'
        BL                                        @RD_MDEX
        LI                                        R2,SECBUF
        MOV                                       @6(R2),R9		; load address
        MOV                                       @8(R2),R10		; program length in bytes
        AI                                        R10,127			; round up & convert to sector length
        SRA                                       R10,7

        MOV                                       @10(R2),R13		; prepare initial WS/PC/ST
        JNE                                       TAG27
        LI                                        R13,>f000
TAG27
        MOV                                       @12(R2),R14
        CLR                                       R15

TAG28
        INC                                       R3			; now load the actual binary
        MOV                                       R3,R1			; load a sector
        BL                                        @RD_MDEX
        LI                                        R2,64			; move 128 bytes into place (64 words)
        LI                                        R1,SECBUF
TAG29
        MOV                                       *R1+,*R9+
        DEC                                       R2
        JNE                                       TAG29
        DEC                                       R10			; more sectors to load?
        JNE                                       TAG28

        BLWP                                      @PROGRESS		; >>>>>>
        LI                                        R12,>0d00		; '\r'
        XOP                                       R12,12

;       FOR a ROM'ed boot loader we cannot swap out the ROM whilst using it:
; place the last few instructions in the WS and execute from there.
; 	place the last few instructions in the WS and execute from there.
;       PLACE the last few instructions in the WS and execute from there.
;
        LI                                        R0,>020c		; li R12,>0040
        LI                                        R1,>0040
        LI                                        R2,>1d00		; sbo 0: swap out ROM
        LI                                        R3,>0380		; rwtp: jump into mdex kernel
        B                                         R0


;
;        DATA CONSTANTS
;
;        .DATA
FD0IMG
        TEXT 'MDEX_FD0DSK'                        ; FAT filename of FD0
FD1IMG
        TEXT 'MDEX_FD1DSK'                        ; FAT filename of FD1
BOOTSAV
        TEXT 'BOOT$.SAV   '                       ; MDEX filename of MDEX kernel
ERRMSG
        TEXT 'Unrecoverable error booting MDEX'
        DATA >0D00
;
;        .EVEN
ZERO
        DATA >0000                                ; convenience ZERO
ENDDIR
        DATA >FFFF                                ; end-of-dir marker


        END
