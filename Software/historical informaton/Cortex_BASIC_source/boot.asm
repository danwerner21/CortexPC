       TITL 'BOOT STATEMENT - CORTEX BASIC REV. 1.1'
       IDT  'BOOT'
*
*
* ROUTINE LIST:
*
*       BOOTY                 ;BOOT STATEMENT
*
       DEF  BOOTY
       REF  TMPBUF            1K scratch buffer
       REF  WPR1              System workspace
*
*      Track 0  Sector 1  Data Format
*                         -----------
*
*             0     2     4     6     8     A     C     E
*          +-----+-----+-----+-----+-----+-----+-----+-----+
*   00     | WP  | PC  |-----|-----|-----|-----|-----|-----|
*          +-----+-----+-----+-----+-----+-----+-----+-----+
*   10     |-----|-----| LN  | SA  |-----|-----|-----| CS  |
*          +-----+-----+-----+-----+-----+-----+-----+-----+
*   20     | CO  | PY  | RI  | GH  | T^  | T.  | I.  | L^  |
*          +-----+-----+-----+-----+-----+-----+-----+-----+
*   30     | BO  | OT  | ^L  | OA  | DE  | R^  | V1  | .0  |
*          +-----+-----+-----+-----+-----+-----+-----+-----+
*          ~     ~     ~     ~     ~     ~     ~     ~     ~
*
*      WP = Entry WP
*      PC = Entry PC
*      LN = Two times number of bytes to transfer
*      SA = Load address for absolute code program
*      CS = Checksum on words hex00 to hex1C inclusive
*      ^  = Space Character
*      All other locations are reserved for vector storage.
*
       PAGE
*
*      MEMORY MAPPED I/O EQUATES
*
FDC    EQU  >F140             FDC memory address
*
*      CRU BASE EQUATES
*
DMAC   EQU  >01C0             DMAC cru address
*
*      CRU I/O BITS
*
SIZEI  EQU  2                 Drive size  1 = 8"   0 = 5.25"
DNSTY  EQU  3                 Drive density  1 = DD  0 = DD
FDCINT EQU  4                 FDC interrupt flag
SIZEO  EQU  4                 Drive size  1 = 8"   0 = 5.25"
*
*      SECTOR BUFFER EQUATES
*
       DORG 0
       BSS  20                Vectors
LENGTH BSS  2                 Twice number of bytes to xfer
SADDR  BSS  8                 Start address for load
SUM    BSS  2                 Checksum
       PAGE
       RORG 0
*
*      BOOT COMMAND ENTRY POINT
*
BOOTY  LWPI WPR1              Use internal RAM for wp
       LIMI 0                 Mask all interrupts
       LI   R1,TABLE          Load pointer to cmd table
       LI   R6,WAIT           Load pointer to wait routine
       LI   R8,ENCODE         Load pointer to 5.25" values
       LI   R9,FDC            Load pointer to FDC
       CLR  R12               Load CRU base of i/o
       MOVB *R1+,*R9          Send RSTC
       BL   *R6               Call wait
*
*      CHECK SIZE JUMPER
*
       SBZ  SIZEO             Set 5.25" drive
       TB   SIZEI             Read jumper
       JNE  MINI              Skip if 5.25"
       C    *R8+,*R8+         Bump pointer by 4 to 8" values
       SBO  SIZEO             Set 8" drive
*
*      RE-CALIBRATE DRIVE
*
MINI   MOV  R9,R10            Copy FDC pointer 
       LI   R7,7              Load byte count
LOOP1  MOVB *R1+,*R10+        Send byte
       DEC  R7                Done ?
       JNE  LOOP1             No , loop
       MOVB @2(R8),*R10       Send last byte
       BL   *R6               Call wait
       PAGE
*
*      CHECK DENSITY JUMPER
*
       TB   DNSTY             Read jumper
       JNE  SD                Skip if single density
       INC  R8                Bump pointer to double d values
       LI   R1,DOUBLE         Change pointer for double d
SD     MOV  R9,R10            Copy FDC pointer
       MOVB *R1+,*R10+        Send AFAS command 
       MOVB *R1+,*R10+       
       MOVB *R1+,*R10+       
       MOVB *R1+,*R10
*
*      SET UP DMAC
*       
       LI   R12,DMAC          Load cru base of DMAC
       SBO  31                Reset
       SBO  25                Select channel 1
       SBZ  20                Select byte xfers
       SBZ  19                Set memory write
       LDCR @ERAM,0           Send stop address
       SBO  16                Enable load of start address
       LI   R2,TMPBUF         Load buffer start
       LDCR R2,0              Send start address
       SBO  17                Enable channel
*
*      SET UP FDC FOR READ
*
       MOV  R9,R10            copy FDC pointer
       LI   R7,8              Load byte count
LOOP2  MOVB *R1+,*R10+        Send byte
       DEC  R7                Done ?
       JNE  LOOP2             No , loop
       MOV  R9,R10            Copy FDC pointer
       MOVB *R1+,*R10+        Send cmd byte
       MOVB *R8,*R10+         Send encode & xfer rate
       LI   R7,6              Load byte count
LOOP3  MOVB *R1+,*R10+        Send byte
       DEC  R7                Done ?
       JNE  LOOP3             No , loop
       BL   *R6               Call wait
       PAGE
*
*      CALCULATE CHECKSUM AND VERIFY VALUE
*
       CLR  R3                 Clear
       LI   R4,SUM-2           Load index
ADD    A    @TMPBUF(R4),R3     Add entry
       DECT R4                 Decrement index
       JOC  ADD                Done ? No , loop
       C    R3,@SUM(R2)        Checksum ok ? 
       JNE  BOOTY              No , re-try
*
*      COPY TRACK 0 INTO MEMORY
*
       MOVB @SORD,*R9         Send read cmd 
       MOVB @LENGTH(R2),@3(R9) Send no of sectors to xfer
       LI   R12,DMAC          Load cru base
       MOV  @SADDR(R2),R3     Fetch start address
       LDCR R3,0              Send new start address
*
*      LOAD INTERNAL RAM WITH CODE
*
       CLR  R12               Load CRU base of i/o
       LI   R0,>1F04          Load "TB   4  " instruction
       LI   R1,>13FE          Load "JEQ  $-2" instruction
       LI   R2,>0413          Load "BLWP *R3" instruction
       MOVB @7(R9),@7(R9)     Re-send last byte
       B    R0                Execute from internal RAM
       PAGE
*
*      WAIT FOR INTERRUPT OR TIMEOUT
*
WAIT   CLR  R0                Set timeout count
       SETO R5                Set count out
WAIT1  CLR  R12               Load CRU base for i/o
       DEC  R0                Timeout ?
       JNE  CHKINT            No , check interrupt
       INC  R5                Count out ?
       JNE  BOOTY             Yes , re-try
CHKINT TB   FDCINT            Interrupt active ?
       JEQ  WAIT1             No , loop
       MOVB @CINT,*R9         Send CINT
       CB   *R9,@OK           Status ok ?
       JNE  BOOTY             No , re-try
       RT
       PAGE
*
*      MISC DATA
*
ERAM   DATA >EFFF             Last dram location
OK     EQU  $-1               Ok status is hex FF
*                             5.25" encode & drive rates
ENCODE BYTE >50,>A1           FM 125K BPS , MFM 250K BPS
       BYTE >A5               Drives 0,1=Rate B   2,3=Rate A
CINT   BYTE >00
*                             8" encode & drive rates
       BYTE >60,>B1           FM 250K BPS , MFM 500K BPS
       BYTE >5A               Drives 0,1=Rate B   2,3=Rate A
*
*      SINGLE DENSITY TABLE
*
TABLE  BYTE >20               RSTC
       BYTE >41               RDAR
       BYTE >15                 A. head step time     10 ms
       BYTE >11                 A. head settle time    8 ms
       BYTE >47                 A. head load time     35ms
       BYTE >64                 B. head step time     50 ms
       BYTE >46                 B. head settle time   35ms
       BYTE >00                 B. head load time      0 ms
*                               entry supplied code
       BYTE >C0               AFAS
       BYTE >FF                 fill
       BYTE >00                 sync
       BYTE >06                 number of sync before AM
       BYTE >30               AIDA
       BYTE >C7                 id am clock pattern
       BYTE >FE                 id am data  pattern
       BYTE >00                 id byte 0 .. track number
       BYTE >00                 id byte 1 .. side
       BYTE >01                 id byte 2 .. sector
       BYTE >00                 id byte 3 .. record length
       BYTE >A5                 crc , 1 am , byte 2, 5 bytes 
SORD   BYTE >88               SORD
*                               entry supplied by code
       BYTE >81                 record length
       BYTE >01                 no lwcur , no pre , 1 sector
       BYTE >00                 track 0
       BYTE >C7                 data am clock pattern
       BYTE >FB                 data am data  pattern
       BYTE >6B                 1 am , xfer , crc , 11 bytes
*      
*      DOUBLE DENSITY TABLE
*       
DOUBLE BYTE >C0               AFAS
       BYTE >FF                 fill
       BYTE >00                 sync
       BYTE >0C                 number of sync before AM
       BYTE >30               AIDA
       BYTE >0A                 id am clock pattern
       BYTE >A1                 id am data  pattern
       BYTE >00                 id byte 0 .. track number
       BYTE >00                 id byte 1 .. side
       BYTE >01                 id byte 2 .. sector
       BYTE >01                 id byte 3 .. record length
       BYTE >E7                 crc , 3 am , byte 2, 7 bytes 
       BYTE >88               SORD
*                               entry supplied by code
       BYTE >03                 record length
       BYTE >01                 no lwcur , no pre , 1 sector
       BYTE >00                 track 0
       BYTE >0A                 data am clock pattern
       BYTE >A1                 data am data  pattern
       BYTE >F6                 3 am , xfer , crc , 22 bytes
       END
