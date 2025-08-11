;
; Boot loader for MDEX on the 9995 breadboard with a CF Card
;
        AORG >7B00
;
; SYMBOLIC NAMES
;

; defines for the CF card registers
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

; defines for CF card commands in upper byte
SETFEAT         EQU >ef00                         ; set feature
RDSECT          EQU >2000                         ; read sector
WRSECT          EQU >3000                         ; write sector
IDENT           EQU >ec00                         ; identify drive
BYTEMOD         EQU >0100                         ; 8 bit access mode

; offsets in partition table records
PTYPE           EQU 4                             ; type offset
START           EQU 8                             ; START sector offset

;        .EVEN
ENDDIR          EQU >FFFF                         ; end-of-dir marker

;
; VARIABLES
;
SECBUF          EQU >8000                         ; sector read buffer
TMP             EQU >ff80                         ; temp var
CLUSLBA         EQU >ff82                         ; LBA of FAT cluster #2

;
; BOOT LOADER START
;
;        .TEXT
BEGIN
        LWPI                                      >f000
        B                                         @MAIN

; Get a (possibly unaligned) word value and flip endian
; R2 points to value, return in R0
;
GET16
TAG0    MOVB                                      *R2+,R0
        SWPB                                      R0
        MOVB                                      *R2,R0
        B                                         *R11

; Get a (possibly unaligned) double word and flip endian
; R2 points to value, return in R0,R1
;
GET32
        MOVB                                      *R2+,R1
        SWPB                                      R1
        MOVB                                      *R2+,R1
        JMP                                       TAG0

; Read a sector into the buffer, LBA sector # in R0,R1
; R0,R1 are destroyed
;
CFREAD
TAG1    MOVB                                      @CFSTAT,@TMP		; wait for card ready
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
TAG2    MOVB                                      @CFSTAT,R0		; wait for card ready
        JLT                                       TAG2
        LI                                        R0,SECBUF		; read data into SECBUF
        LI                                        R1,512
TAG3    MOVB                                      @CFDATA,*R0+
        DEC                                       R1
        JNE                                       TAG3
        B                                         *R11			; done

; Convert a FAT32 cluster number in R0,R1 to an LBA sector number in R0,R1
; R2 is destroyed
;
CLUS2LBA
        DECT                                      R1			; clus = clus - 2
        JOC                                       TAG4
        DEC                                       R0
TAG4    LI                                        R2,3			; sec = clus * 8
TAG5    SLA                                       R0,1
        SLA                                       R1,1
        JNC                                       TAG6
        INC                                       R0
TAG6    DEC                                       R2
        JNE                                       TAG5
        A                                         @CLUSLBA,R0		; lba = sec + CLUSLBA
        A                                         @CLUSLBA+2,R1
        JNC                                       TAG7
        INC                                       R0
TAG7    B                                         *R11

; Does this FAT32 directory entry match? If so, fetch START sector#
; R2 points to entry, R3 points to 8.3 filename
; if it is a match, LBA of file is in R0,R1 and EQ is set
; R2-R4,R12 are destroyed in all cases
; on no match, EQ is reset and R0,R1 are unchanged
;
FATTEST
        MOV                                       R11,R12
        CLR                                       R4			; attribute is >00 or >20?
        MOVB                                      @11(R2),R4
        JEQ                                       TAG8
        CI                                        R4,>2000
        JNE                                       TAG10			; no
TAG8    LI                                        R4,11			; yes; does name match?
TAG9    CB                                        *R2+,*R3+
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
TAG10   B                                         *R12

; Read a sector from the dev 0 image
; R0-R1 are destroyed
RD_BOOT
        MOV                                       @>ff00,R0		; the LBA of DEV0 is sector 0
        MOV                                       @>ff02,R1
        JNC                                       TAG11
        INC                                       R0
TAG11   B                                         @CFREAD			; read the sector

; Report a fatal ERROR
;
ERROR
        XOP                                       @ERRMSG,14		; print ERROR message
        B                                         @>0142			; return to EVMBUG

PROGRESS
        DATA >F020                                ; BAR
        DATA >7BDA
BAR
        LI                                        R0,'>'*256
        XOP                                       R0,12
        RTWP

;
; Here is the START of the MAIN program. It has three phases:
; 1. Init the CF card
; 2. Read the FAT32 file system and find the location of the FD0 and FD1 images
; 3. In the FD0 image, find 'BOOT$.SAV' in the MDEX file system and load it
;

; Init CF card, set 8-bit mode
MAIN
TAG12   MOVB                                      @CFSTAT,R0		; wait for card ready
        JLT                                       TAG12
        MOVB                                      @ZERO,@CFLBA3		; set 8 bit access mode
        LI                                        R0,BYTEMOD
        MOVB                                      R0,@CFFEAT
        LI                                        R0,SETFEAT
        MOVB                                      R0,@CFCMD

        BLWP                                      @PROGRESS		; >

; Read the Master Boot Record (contains the Partition Table)
;
        CLR                                       R0			; read sector 0, the MBR
        CLR                                       R1
        BL                                        @CFREAD
        LI                                        R2,SECBUF+>1be		; point R2 at partition table
        LI                                        R3,4			; four entries
        CLR                                       R0
TAG13   MOVB                                      @PTYPE(R2),R0
        CI                                        R0,11*256		; FAT32 is type 11 or 12
        JEQ                                       TAG14
        CI                                        R0,12*256
        JEQ                                       TAG14
        DEC                                       R3			; next partition of 4
        JEQ                                       ERROR
        AI                                        R2,16
        JMP                                       TAG13
TAG14   AI                                        R2, START		; found FAT32 partition, read START
        BL                                        @GET32			;  sector

        BLWP                                      @PROGRESS		; >>

; Read the FAT32 boot block
;
        MOV                                       R0,@CLUSLBA		; save base sector #
        MOV                                       R1,@CLUSLBA+2
        BL                                        @CFREAD			; read boot block
        LI                                        R2,SECBUF+14
        BL                                        @GET16			; add reserved sectors to base
        A                                         R0,@CLUSLBA+2
        JNC                                       TAG15
        INC                                       @CLUSLBA
TAG15   CLR                                       R0			; assert that there are 2 FAT copies
        MOVB                                      @SECBUF+16,R0
        CI                                        R0,>0200
        JNE                                       ERROR
        LI                                        R2,SECBUF+36		; get FAT table length in sectors
        BL                                        @GET32
        SLA                                       R0,1			; multiply by 2
        SLA                                       R1,1
        JNC                                       TAG16
        INC                                       R0
TAG16   A                                         R0,@CLUSLBA		; add FAT length * 2 to base
        A                                         R1,@CLUSLBA+2
        JNC                                       TAG17
        INC                                       @CLUSLBA
TAG17   CLR                                       R0			; fetch sectors per cluster
        MOVB                                      @SECBUF+13,R0
        CI                                        R0,>0800		; assert 8 sectors/cluster
        JNE                                       ERROR
        LI                                        R2,SECBUF+44
        BL                                        @GET32			; fetch root dir START cluster

        BLWP                                      @PROGRESS		; >>>

; Read the first sector of the FAT32 root directory
;
        BL                                        @CLUS2LBA		; convert to sector lba
        BL                                        @CFREAD			; read root dir first sector
        LI                                        R6,16			; 16 entries
        CLR                                       R7			; no matches yet
        LI                                        R5,SECBUF		; point to first entry
TAG18   MOV                                       R5,R2
        LI                                        R3,FD0IMG
        BL                                        @FATTEST
        JNE                                       TAG19
        MOV                                       R0,@>ff00		; found 'LSX_DEV0.DSK' image
        MOV                                       R1,@>ff02
        JMP                                       TAG20
TAG19   MOV                                       R5,R2
        LI                                        R3,FD1IMG
        BL                                        @FATTEST
        JNE                                       TAG21
        MOV                                       R0,@>ff04		; found 'LSX_DEV1.DSK' image
        MOV                                       R1,@>ff06
TAG20   INC                                       R7
        CI                                        R7,2			; have we found both?
        JEQ                                       READ_BOOT		; yes, now read dev 0 boot block
TAG21   AI                                        R5,32			; no, move to next dir entry
        CI                                        R5,SECBUF+512		; past end?
        JLT                                       TAG18			; no
        B                                         @ERROR			; yes, signal an ERROR (todo: read full dir)

; Now read the boot block of LSX_DEV0 and pass control
;
READ_BOOT
        BLWP                                      @PROGRESS		; >>>>
        BL                                        @RD_BOOT
        BLWP                                      @PROGRESS		; >>>>>
        LI                                        R15,>effe		; ********************* DDW CHANGE SP TO R15
        B                                         @SECBUF			; pass control to the LSX boot loader


;
; DATA CONSTANTS
;
;        .DATA
FD0IMG
        TEXT 'LSX_DEV0DSK'                        ; FAT filename of FD0
FD1IMG
        TEXT 'LSX_DEV1DSK'                        ; FAT filename of FD1
ERRMSG
        TEXT 'Unrecoverable error booting LS'
        DATA >580D
        DATA >0000
ZERO
        DATA >0000                                ; convenience ZERO

        END
