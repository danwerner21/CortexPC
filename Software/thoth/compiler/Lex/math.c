
/* 128-bit math routines as per Stafford section 3.5.1, pp 27-29 */

#include "lex.h"

struct Number {
    int  len;
    char byte[16];
};

struct Number Const_val;

void
Const_clr(void)
{
    Const_val.len = 1;
    Const_val.byte[0] = 0;
    Const_val.byte[1] = 0;
}

void
Const_put(void)
{
    int i;

    Put( 128 + Const_val.len );
    for(i = Const_val.len-1; i >= 0; --i) {
        Put( Const_val.byte[i] );
    }
}

void
Const_add(int val)
{
    int i;
    unsigned v;
    
    for(i = 0; i < 16; ++i) {
	v = Const_val.byte[i] & 0xff;
	v += val;
	Const_val.byte[i] = v;
	if( (val = v >> 8) == 0 ) break;
    }
    i++;
    if( Const_val.len < i ) Const_val.len = i ;
}

void
Const_mul(int val)
{
    unsigned v, carry;
    int i;

    carry = 0;
    for(i = 0; i < Const_val.len; ++i) {
	v = (Const_val.byte[i] & 0xff) * val + carry;
	Const_val.byte[i] = v & 0xff;
	carry = (v >>= 8);
    }
    if( carry > 0 ) {
	Const_val.byte[i] = carry;
	++Const_val.len;
    }
}
