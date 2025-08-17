/*
 * Include file for use with ttyctl.c routines.
 * You should include globdefs.h first.
 */

#include <sgtty.h>

struct tm
{
    struct sgttyb tm_sgtty;
    int tm_local;
};

typedef struct tm TTYMODE;

extern TTYMODE *AllocMode();
extern TTYMODE *ChgMode();
extern TTYMODE *OrigMode();
extern TTYMODE *CurMode();
