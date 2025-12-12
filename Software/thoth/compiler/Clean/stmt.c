
/* This file implements Stafford section 5.3 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "../Parse/il_opc.h"

struct stmt {
    struct stmt *next;
    int     len;
    char    text[1];
};
struct stmt *First, *Last;

extern char buffer[];
extern char *ptr;
extern int OFile;

void
Add_stmt(void)
{
    struct stmt *line;
    int len = ptr - buffer;

    line = (struct stmt *) malloc( sizeof(struct stmt) + len );
    line->next = NULL;
    line->len = len;
    memcpy( &line->text, buffer, len);
    ptr = buffer;

    if( Last ) Last->next = line;
    Last = line;
    if( !First ) First = line;
}

int Log_calls(char *text, int idx, int offs);

void
Put_stmts(void)
{
    struct stmt *s, *t;
    
    s = First;
    while( s ) {
        // find function calls and log
        switch( s->text[0] ) {
        // if the stmt has an expression, check for calls made
        case 't': case 'E': case 'R': case 'W':
        case 'S': case 'T': case 'F':
            Log_calls(s->text, 1, lseek(OFile, 0, SEEK_CUR));
        }
        //fprintf(stderr, "%c ", s->text[0]);
        write(OFile, &s->text, s->len);
        t = s->next;
        free( s );
        s = t;
    }
    First = Last = NULL;
}

/* ============= */

int Changed;

#define FALSE 0
#define TRUE  1

#define cmd text[0]

int
Get_ref(struct stmt *stmt)
{
    char *text;
    int ref;
    
    text = stmt->text + 1;
    ref = *text++;
    if( ref & 0x80 ) {
        return( ((*text & 0xff) << 7 ) + (ref & 0x7f) );
    }
    return( ref );
}

void
Change_ref(struct stmt *stmt, int new )
{
    int old;
    
    old = Get_ref( stmt );
    if( old < 128 && new >= 128 ) {
        memmove(stmt->text+3, stmt->text+2, stmt->len - 2);
        stmt->len++;
    }
    if( old >= 128 && new < 128 ) {
        memmove(stmt->text+2, stmt->text+3, stmt->len - 3);
        stmt->len--;
    }
    if( new < 128 )
        stmt->text[1] = new;
    else {
        stmt->text[1] = (new & 0x7f) | 0x80;
        stmt->text[2] = (new >> 7 ) & 0x7f;
    }
}

/* This function implements Stafford section 5.3.1 */
void
Unreachable(void)
{
    char c;
    struct stmt *stmt, *s;

    for( stmt = First; stmt != NULL; stmt = stmt->next ) {
        if( (c = stmt->cmd) == 'J' || c == 'R' ) {
            while( stmt->next && (c = stmt->next->cmd) != 'l'
                    && c != 'i' && c!='r' && c != 'b' && c != 'D'
                    && c != 'w' && c != 's' )
            {
                stmt->next->cmd = '0';
                stmt = stmt->next;
                Changed = TRUE;
            }
        }
    }
}

#define MAXLABEL 100

/* This function implements Stafford section 5.3.2 */
void
Straighten_transfers(void)
{
    int c, i, ref, tgt;
    int labels[MAXLABEL], last_label = 0;
    struct stmt *stmt;
    
    /* build list of twisted labels */
    for( stmt = First; stmt != NULL; stmt = stmt->next ) {
        if( stmt->cmd == 'l' &&
            stmt->next && stmt->next->cmd == 'J' )
        {
            labels[ last_label++ ] = Get_ref( stmt );
            labels[ last_label++ ] = Get_ref( stmt->next );
        }
    }
    
    /* adjust jumps to use real target labels */
    for( stmt = First; stmt != NULL; stmt = stmt->next ) {
        if( (c = stmt->cmd) == 'J' || c == 'T' || c == 'F' ) {
            ref = Get_ref( stmt );
            for( i = 0; i < last_label; i += 2 )
                if( labels[i] == ref ) break;
            if( i < last_label ) {
                tgt = labels[ i+1 ];
                Change_ref( stmt, tgt );
                Changed = TRUE;
            }
        }
    }
}

/* This function implements Stafford section 5.3.3 */
void
Unreferenced_labels(void)
{
    int c, ref, i;
    int labels[MAXLABEL], last_label = 0;
    struct stmt *stmt;
    
    /* build list of referenced labels */
    for( stmt = First; stmt != NULL; stmt = stmt->next ) {
        if( (c = stmt->cmd) == 'J' || c == 'T' || c == 'F' ) {
            labels[ last_label++ ] = Get_ref( stmt );
        }
    }
    
    /* remove the unused labels */
    for( stmt = First; stmt != NULL; stmt = stmt->next ) {
        if( stmt->cmd == 'l' ) {
            ref = Get_ref( stmt );
            for( i = 0; i < last_label; ++i )
                if( labels[i] == ref ) break;
            if( i < last_label ) continue;
            stmt->cmd = '0';
            Changed = TRUE;
        }
    }
}

/* This function implements Stafford section 5.3.4 */
void
Null_transfers(void)
{
    int target, label;
    struct stmt *stmt;
    
    for( stmt = First; stmt != NULL; stmt = stmt->next ) {
        if( stmt->cmd == 'J' ) {
            target = Get_ref( stmt );
            if( stmt->next && stmt->next->cmd == 'l' &&
                Get_ref( stmt->next ) == target )
            {
                stmt->cmd = '0';
                Changed = TRUE;
            }
        }
    }
}

/* This function implements Stafford sectin 5.3.5 */
void
Drop_condition(void)
{
    int c, target1, target2;
    struct stmt *stmt, *jmp;

    for( stmt = First; stmt != NULL; stmt = stmt->next ) {
        if( (c = stmt->cmd) == 'T' || c == 'F' ) {
            target1 = Get_ref( stmt );
            jmp = stmt->next;
            if( jmp && jmp->cmd == 'J' ) {
                target2 = Get_ref( jmp );
                if( target1 == target2 )
                {
                    stmt->cmd = 'E';
                    memmove( stmt->text + 1, stmt->text + 3, stmt->len - 3 );
                    stmt->len -= 2;
                    Changed = TRUE;
                }
            }
        }
    }
}

/* This function implements Stafford section 5.3.6
 * Note that the expression optimiser adds an IL_RQD node to the
 * end of every expression with side effects.
 */
void
Useless_expr(void)
{
    struct stmt *stmt;
    unsigned char *cp;

    for( stmt = First; stmt != NULL; stmt = stmt->next ) {
        cp = (unsigned char *)stmt->text + stmt->len - 2;
        if( stmt->cmd == 'E' && cp[0] != IL_RQD ) {
            stmt->cmd = '0';
            Changed = TRUE;
        }
    }
}

/* This function implements Stafford section 5.3.7 */
void
Reverse_jumps(void)
{
    int c, target1, target2, label;
    struct stmt *stmt, *jmp, *lbl;

    for( stmt = First; stmt != NULL; stmt = stmt->next ) {
        if( (c = stmt->cmd) == 'T' || c == 'F' ) {
            target1 = Get_ref( stmt );
            jmp = stmt->next;
            if( jmp && jmp->cmd == 'J' ) {
                target2 = Get_ref( jmp );
                lbl = jmp->next;
                if( lbl && lbl->cmd == 'l' && Get_ref( lbl ) == target1 )
                {
                    stmt->cmd = 'T' + 'F' - stmt->cmd;
                    Change_ref( stmt, target2 );
                    jmp->cmd = '0';
                    Changed = TRUE;
                }
            }
        }
    }
}

/* Remove all statements marked for deletion */
void
Remove_deleted(void)
{
    char c;
    struct stmt *stmt, *prev, *next;

    prev = NULL;
    for( stmt = First; stmt != NULL; stmt = next ) {
        next = stmt->next;
        if( stmt->cmd == '0' ) {
            if( prev == NULL ) First = next; else prev->next = next;
            free( stmt );
            continue;
        }
        prev = stmt;
    }
}

void
Clean_stmts(void)
{
    do {
        Changed = FALSE;
        Unreachable();
        Straighten_transfers();
        Unreferenced_labels();
        Null_transfers();
        Drop_condition();
        Useless_expr();
        Reverse_jumps();
        Remove_deleted();
    }
    while( Changed );
}


