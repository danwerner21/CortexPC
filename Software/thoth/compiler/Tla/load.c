/* Linker support routines
 *
 * The routines below are based on Bonkowski section 2.5, pp. 14-16
 * and also on Appendix II of Sager CS-77-15.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "load.h"
#include "mani.h"

int L_buf[32], L_ptr;
int O_buf[64], O_ptr;

int bpc = 1; /* bytes per cell */
int bpb = 8; /* bits per byte  */
int bpw = 2; /* bytes per word */

char Load_command[40];
int OFile;

/* Write a command to the output file, including
 * the chacksum byte.
 */
void
Put_command(int len)
{
    int i, check;
    
    check = 0;
    for( i = 0; i < len; ++i) {
        check ^= Load_command[i];
    }
    Load_command[len] = check;
    if( write(OFile, Load_command, len+1) < 0 ) {
        fprintf(stderr, "error writing output file [%c]\n", Load_command[0]);
        exit(1);
    }
}

/* Loads a target word of data into the L-buffer and
 * updates the current location counter by the number
 * of bytes in the word.
 */
void
Load_word(int data)
{
    int i;

    if( L_ptr > (32 - bpw) ) Flush_load_bufs();

    for( i = bpw - 1; i >= 0; --i ) {
        L_buf[L_ptr++] = (data >> 8*i) & 0xff;
    }
    *Counter += bpw;
}

/* Loads a target word of data into the L-buffer, updates
 * a location counter and constructs an O-buffer word pair
 * using the Rd_index, Wsd_index and the L-buffer index at
 * which the data was stored.
 */
void
Rload_word(int Rd_index, int Wsd_index, int data)
{
    int i;

    if( L_ptr > (32 - bpw) ) Flush_load_bufs();

    for( i = bpw - 1; i >= 0; --i ) {
        L_buf[L_ptr++] = (data >> 8*i) & 0xff;
    }
    O_buf[O_ptr++] = ((L_ptr - 1) << 3) | Rd_index;
    O_buf[O_ptr++] = Wsd_index;
    *Counter += bpw;
}

/* Outputs the L and O directives using the contents of
 * the L-buffer and O-buffer.
 */
void
Flush_load_bufs(void)
{
    int i;

    Load_command[0] = 'L';
    Load_command[1] = L_ptr;
    if( (L_buf[2] = L_ptr - 3) > 0 ) {

        for( i = 0; i < L_ptr; ++i) {
            Load_command[2+i] = L_buf[i];
        }
        Put_command( L_ptr + 2 );
        L_ptr = 0;
        
        if( O_ptr > 0 ) {
            Load_command[0] = 'O';
            Load_command[1] = O_ptr;
            for( i = 0; i < O_ptr; ++i) {
                Load_command[2+i] = O_buf[i];
            }
            Put_command( O_ptr + 2 );
            O_ptr = 0;
        }
    }
    Set_load_L_addr(Counter_level, *Counter);
}

/* Outputs an M directive for the specified module.
 */
void
Put_M_dir(int Rbr, int multiplier, char *module_name)
{
    int i, c;

    Load_command[0] = 'M';
    Load_command[2] = Rbr;
    Load_command[3] = multiplier;
    
    for( i = 0; (i < 32) && (c=module_name[i]); ++i) {
        Load_command[i+4] = c;
    }
    Load_command[1] = i+2;
    Put_command( i+4 );
}

/* Outputs the 't' or 'g' symbol reference directive for the specified
 * symbol.
 */
void
Put_load_ref(int type, int Wsd_index, char *symbol_name)
{
    int i, c;

    Load_command[0] = type;
    Load_command[2] = Wsd_index;
    
    for( i = 0; (i < 32) && (c=symbol_name[i]); ++i) {
        Load_command[i+3] = c;
    }
    Load_command[1] = i+1;
    Put_command( i+3 );  
}

/* Outputs the 'T' or 'G' symbol definition directive for the specified
 * symbol.
 */
void
Put_load_def(int type, int Rbr, int offset, char *name)
{
    int i, c;

    Load_command[0] = type;
    Load_command[2] = Rbr;
    Load_command[3] = (offset >> 8) & 0xff;
    Load_command[4] = offset & 0xff;
    
    for( i = 0; (i < 32) && (c=name[i]); ++i) {
        Load_command[i+5] = c;
    }
    Load_command[1] = i+3;
    Put_command( i+5 );   
}

/* Outputs an I directive to increment the Rbr. */
void
Put_I_dir(int Rbr, int increment)
{
    Load_command[0] = 'I';
    Load_command[1] = 3;
    Load_command[2] = Rbr;
    Load_command[3] = (increment >> 8) & 0xff;
    Load_command[4] = increment & 0xff;
    Put_command( 5 );
}

/* Outputs an A directive to align the Rbr. */
void
Put_A_dir(int Rbr, int multiplier)
{
    Load_command[0] = 'A';
    Load_command[1] = 2;
    Load_command[2] = Rbr;
    Load_command[3] = multiplier & 0xff;
    Put_command( 4 );
}

/* A general purpose function to output the vector specified by
 * 'directive'.
 */
void
Put_load_dir(char *directive)
{
    int len, i;
    
    len = directive[1];
    for(i = 0; i < len+2; ++i) Load_command[i] = directive[i];
    Put_command( len+2 );
}

/* Loads the address in the first few bytes in the L-buffer and
 * sets the first O-buffer word pair to relocate the address.
 */
 
void
Set_load_L_addr(int Wsd_index, int address)
{
    L_ptr = 0;
    O_ptr = 0;
    L_buf[L_ptr++] = (address >> 8) & 0xff;
    L_buf[L_ptr++] = address & 0xff;
    L_buf[L_ptr++] = 0;
     
    O_buf[O_ptr++] = ((L_ptr - 2) << 3) | ADDR_RELDESC;
    O_buf[O_ptr++] = Wsd_index;
}

void
Load_byte(int data)
{
    int i;
    
    if( L_ptr == 32 ) Flush_load_bufs();

    L_buf[L_ptr++] = data;
    *Counter += 1;
}

int Counters[WSD_LAST];
int *Counter;
int Counter_level;

int
Set_loc(int segment)
{
    int old = Counter_level;
    
    Counter = Counters + segment;
    Counter_level = segment;
    Flush_load_bufs();
    return( old );
}
int WSD_index = 8;

int
Next_Wsd( void )
{
    return( WSD_index++ );
}

void Clean(void)
{
    int i;
    
    for( i = 0; i < WSD_LAST; i++ )
        Counters[i] = 0;
    Counter = &Counters[0];
    Counter_level = 0;
    L_ptr = 0;
    O_ptr = 0;
    Set_load_L_addr(0, 0);
    WSD_index = 8;
}



