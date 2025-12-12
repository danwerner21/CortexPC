
\ Expression tree interior node
LEFT     = 0;
OP       = 1;
RIGHT    = 2;
RESULTIN = 3;
SCRATCH  = 4;

\ Expression tree leaf nodes: base, string, constant
LINK          = 0; \ base
REF           = 1;
VALUE         = 2;
ADDRMOD       = 3;
TYPE          = 4;
STRING_ID     = 5; \ string
NEXT_CONSTANT = 5; \ constant
CONST_VALUE   = 6;

\ Expression leaf type
\ +----+--------+----------+------------------+
\ | D  | TYPE   | REL      |         WSD      |
\ +----+--------+----------+------------------+
\   15  14       10         7                0

SYMBOL_DEFINED = $8000
TYPE_FIELD     = $7800
REL_WSD_FIELD  = $07FF
WSD_FIELD      = $00FF

\ TYPE_FIELD values
CONST_TYPE
STRING_TYPE
AUTO_TYPE
EXTRN_TYPE

\ Code table test entry
\ +--+--------------+--------------+--------------+
\ |X | action count | test operand | test code    |
\ +--+--------------+--------------+--------------+
\  15 14             9              4            0

CONTINUE     = $8000
ACTION_COUNT = $7C00
TEST_OP      = $03E0
TEST_CODE    = $001F

\ Test and Action operands 
left        = 1       
right       = 2  
dest        = 3
scratch     = 4
const_1     = 5
value_1     = 6
const_2     = 7
const_4     = 8
const_8     = 9
value_8     = 10
const_16    = 11
val_left    = 12
const_val_right = 13
val_dest    = 14
r11         = 15
r12         = 16
temp1       = 17
temp2       = 18
temp3       = 19
temp4       = 20
temp5       = 21
stk1        = 22
stk2        = 23
stk3        = 24
arg1        = 25
arg2        = 26
arg3        = 27
return_val_locn = 28
addrmod_dest = 29

\ Test codes
const_or_str = 2
in_reg       = 3
is_arg1      = 4
is_arg2      = 5
is_auto      = 6
is_const_0   = 7
is_const_1   = 8
is_constant  = 9
is_extrn     = 10
is_gt_0      = 11
is_gt_15     = 12
is_in_arg1   = 13
is_in_dest   = 14
is_left      = 15
is_neg_1     = 16
is_right     = 17
is_string    = 18
is_volatile  = 19

\ Code table action entry
\ +--+-------------+---------------+---------------+
\ |X | action code | operand_1     | operand_2     |
\ +--+-------------+---------------+---------------+
\ |     optional operand_3                         |
\ +------------------------------------------------+
\  15  14            9              4             0

OP3_EXISTS = $8000
OPCODE     = $7C00
OP1        = $03E0
OP2        = $001F

Left;
Right;
Destination;
Scratch;
R;
Temp_1;
Temp_2;
Temp_3;
Temp_4;
Temp_5;
Const_one;
Constant_2;
Constant_4;
Constant_8;
Constant_16;
Return_val_locn;

\ In total, approximately 185 tests are made and about 460 actions
\ are performed. There are 16 different types of actions and 19
\ different types of tests involved. Of the total number of actions,
\ 182 are calls to the Code function and another 93 are calls to one
\ of 34 functions which perform commonly used actions.

