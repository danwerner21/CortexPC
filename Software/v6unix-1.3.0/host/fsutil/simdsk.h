/***********************************************************************
*
* simdsk.h - Definitions for TILINE disks.
*
* Changes:
*   12/17/08   DGP   Original.
*   08/01/20   DGP   Added FD800 support.
*   08/03/20   DGP   Added share field.
*
***********************************************************************/

#ifndef __SIMDSK_H__
#define __SIMDSK_H__

#define DSKOVERHEAD 16
#define MAXDISKS 25

typedef struct
{
   char *model;
   int cyls;
   int heads;
   int sectrk;
   int bytsec;
   int overhead;
   int share;
} disk_types;


#ifdef EXTERN
extern disk_types disks[MAXDISKS+1];
#else
disk_types disks[MAXDISKS+1] =
{
   { "FD800"     ,  77,  1,  26, 128,   0, 1 },
   { "CMD16"     , 806,  1,  66, 256,   0, 0 },
   { "CMD80"     , 806,  5,  66, 256,   0, 0 },
   { "DS10"      , 408,  2,  20, 288,  96, 0 },
   { "DS25"      , 408,  5,  38, 288,   0, 0 },
   { "DS31"      , 203,  2,  24, 288,   0, 0 }, /* DS31/DS32 */
   { "DS50"      , 815,  5,  38, 288,   0, 0 },
   { "DS80"      , 803,  5,  61, 256,   0, 0 },
   { "DS200"     , 815, 19,  38, 288,   0, 0 },
   { "DS300"     , 803, 19,  61, 256,   0, 0 },
   { "FD1000"    ,  77,  2,  26, 288,   0, 1 },
   { "SI470"     ,  30, 10,  88, 256,   0, 0 },
   { "WD500"     , 150,  4,  32, 256,   0, 0 },
   { "WD500A"    , 694,  3,  32, 256,   0, 0 },
   { "WD800-18"  , 603,  3,  37, 256,   0, 0 },
   { "WD800-43"  , 603,  7,  37, 256,   0, 0 },
   { "WD800A-43" , 871,  3,  64, 256,   0, 0 },
   { "WD800A-100", 871,  7,  64, 256,   0, 0 },
   { "WD900"     , 815, 24,  38, 256,   0, 0 },
   { "MSU2"      , 957,  9,  36, 512,   0, 2 },
   { "MSU2A"     ,1204, 15,  36, 512,   0, 2 },
   { "CP3540"    ,1805, 12,  49, 512,   0, 2 }, /* Connor CP3540 */
   { "ST312"     ,1495, 18, 127, 256,   0, 2 }, /* Seagate ST31200N */
   { "SD512"     , 916, 18, 127, 256, 256, 2 }, /* SD card config to 512KB */
   { "ST512"     , 916, 18, 127, 256,   0, 2 }, /* SD card via scsi 512KB */
   { "UNKNOWN"   ,   0,  0,   0,   0,   0, 0 }
};
#endif

#endif
