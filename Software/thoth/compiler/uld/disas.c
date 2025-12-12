// dis9900.c
// EP 2019-09-12
// Quick and dirty disassembler for TMS9900.
// This is a line by line disassembler designed to dissassemble from
// stdin to stdout, so that it can be used with gtkwave to analyze instructions.

#include <stdio.h>

#include "uld.h"

enum ins9900 { Optype_1, Optype_2, Optype_2b, Optype_3,
    Optype_2c, Optype_10, Optype_6, Optype_8, Optype_5,
    Optype_7, Optype_4, Optype_9 };

#define INTLEN 32

struct instrucion_t {
    unsigned bin;
    unsigned mask;
    enum ins9900 type;
    char str[8];
} instructions[] = {
    // dual operand instructions.
    // dual operand with multiple addressing modes for source and destination
    { 0xa000, 0xf000, Optype_1, "a" },
    { 0xb000, 0xf000, Optype_1, "ab" },
    { 0x8000, 0xf000, Optype_1, "c" },
    { 0x9000, 0xf000, Optype_1, "cb" },
    { 0x6000, 0xf000, Optype_1, "s" },
    { 0x7000, 0xf000, Optype_1, "sb" },
    { 0xe000, 0xf000, Optype_1, "soc" },
    { 0xf000, 0xf000, Optype_1, "socb" },
    { 0x4000, 0xf000, Optype_1, "szc" },
    { 0x5000, 0xf000, Optype_1, "szcb" },
    { 0xc000, 0xf000, Optype_1, "mov" },
    { 0xd000, 0xf000, Optype_1, "movb" },
    // dual operand with multiple addressing modes for source and workspace reg for dest
    { 0x2000, 0xfc00, Optype_2, "coc" },
    { 0x2400, 0xfc00, Optype_2, "czc" },
    { 0x2800, 0xfc00, Optype_2, "xor" },
    { 0x3800, 0xfc00, Optype_2, "mpy" },
    { 0x3c00, 0xfc00, Optype_2, "div" },
    // xop
    { 0x2c00, 0xfc00, Optype_2b, "xop" },
    // single operand instructions
    { 0x0440, 0xffc0, Optype_3, "b" },
    { 0x0680, 0xffc0, Optype_3, "bl" },
    { 0x0400, 0xffc0, Optype_3, "blwp" },
    { 0x04c0, 0xffc0, Optype_3, "clr" },
    { 0x0700, 0xffc0, Optype_3, "seto" },
    { 0x0540, 0xffc0, Optype_3, "inv" },
    { 0x0500, 0xffc0, Optype_3, "neg" },
    { 0x0740, 0xffc0, Optype_3, "abs" },
    { 0x06c0, 0xffc0, Optype_3, "swpb" },
    { 0x0580, 0xffc0, Optype_3, "inc" },
    { 0x05c0, 0xffc0, Optype_3, "inct" },
    { 0x0600, 0xffc0, Optype_3, "dec" },
    { 0x0640, 0xffc0, Optype_3, "dect" },
    { 0x0480, 0xffc0, Optype_3, "x" },
    // cru multibit
    { 0x3000, 0xfc00, Optype_2c, "ldcr" },
    { 0x3400, 0xfc00, Optype_2c, "stcr" },
    // cru single bit
    { 0x1d00, 0xff00, Optype_10, "sbo"},
    { 0x1e00, 0xff00, Optype_10, "sbz"},
    { 0x1f00, 0xff00, Optype_10, "tb"},
    // jump instructions
    { 0x1300, 0xff00, Optype_6, "jeq" }, 
    { 0x1500, 0xff00, Optype_6, "jgt" }, 
    { 0x1b00, 0xff00, Optype_6, "jh" }, 
    { 0x1400, 0xff00, Optype_6, "jhe" }, 
    { 0x1a00, 0xff00, Optype_6, "jl" }, 
    { 0x1200, 0xff00, Optype_6, "jle" }, 
    { 0x1100, 0xff00, Optype_6, "jlt" }, 
    { 0x1000, 0xff00, Optype_6, "jmp" }, 
    { 0x1700, 0xff00, Optype_6, "jnc" }, 
    { 0x1600, 0xff00, Optype_6, "jne" }, 
    { 0x1900, 0xff00, Optype_6, "jno" }, 
    { 0x1800, 0xff00, Optype_6, "joc" }, 
    { 0x1c00, 0xff00, Optype_6, "jop" }, 
    // Optype_8
    { 0x0a00, 0xff00, Optype_8, "sla" }, 
    { 0x0800, 0xff00, Optype_8, "sra" }, 
    { 0x0b00, 0xff00, Optype_8, "src" }, 
    { 0x0900, 0xff00, Optype_8, "srl" }, 
    // immediate instructions, don't care n
    { 0x0220, 0xffe0, Optype_5, "ai" },
    { 0x0240, 0xffe0, Optype_5, "andi" },
    { 0x0280, 0xffe0, Optype_5, "ci" },
    { 0x0200, 0xffe0, Optype_5, "li" },
    { 0x0260, 0xffe0, Optype_5, "ori" },
    // internal register load immediate
    { 0x02e0, 0xffe0, Optype_7, "lwpi"},
    { 0x0300, 0xffe0, Optype_7, "limi"},
    // internal register store/load
    { 0x02c0, 0xffe0, Optype_4, "stst"},
    { 0x02a0, 0xffe0, Optype_4, "stwp"},
    { 0x0080, 0xffe0, Optype_4, "lwp" },
    { 0x0090, 0xffe0, Optype_4, "lst" },
    // Optype_9 and external instructions
    { 0x0380, 0xffe0, Optype_9, "rtwp" },
    { 0x0340, 0xffe0, Optype_9, "idle" },
    { 0x0360, 0xffe0, Optype_9, "rset" },
    { 0x03c0, 0xffe0, Optype_9, "ckof" },
    { 0x03a0, 0xffe0, Optype_9, "ckon" },
    { 0x03e0, 0xffe0, Optype_9, "lrex" },
    // end

    { 0,0, Optype_1, "" }
};

int ptr;

int
get_word(void)
{
    int word;
    
    word = (Memory[ptr]<<8)+Memory[ptr+1];
    ptr += 2;
    return( word );
}

char *addr_mode(char *s, int mode)
{
    switch( (mode >> 4) & 3 )
    {
        case 0: sprintf(s, "r%d",  mode & 0xf); break;
        case 1: sprintf(s, "*r%d", mode & 0xf); break;
        case 2: 
            if (mode & 0xf)
                sprintf(s, "@%d(r%d)", (short)get_word(), mode & 0xf); 
            else
                sprintf(s, "@%04x", get_word());
            break;
        case 3: sprintf(s, "*r%d+", mode & 0xf); break;
    }
    return s;
}

int dasm_one(char *buf, int addr)
{
    int opcode;
    int j = -1;

    ptr = addr;
    opcode = get_word();

    for(int i=0; instructions[i].bin; i++) {
        if((opcode & instructions[i].mask) == instructions[i].bin) {
            j = i;
            break;
        }
    }
    if(j == -1) {
        sprintf(buf, ">%04X      <<<<<", opcode);
        return ptr;
    }

    // display addressing modes
    char s1[20], s2[20];
    int  count;
    int  offset;
    
    switch(instructions[j].type)
    {
        case Optype_1:
            sprintf(buf, "%-4s %s,%s", instructions[j].str,
                addr_mode(s1, opcode & 0x3f), 
                addr_mode(s2, (opcode>>6) & 0x3f));
            break;
        case Optype_2: 
            sprintf(buf, "%-4s %s,r%d", instructions[j].str,
                addr_mode(s1, opcode & 0x3f), 
                (opcode >> 6) & 0xf);
            break;
        case Optype_2b:
            sprintf(buf, "%-4s %s,%d ", instructions[j].str,
                addr_mode(s1, opcode & 0x3f), (opcode >> 6) & 0xf
                );
            break;
        case Optype_3:
            sprintf(buf, "%-4s %s", instructions[j].str,
                addr_mode(s1, opcode & 0x3f)
                );
            break;
        case Optype_2c:
            count = (opcode >> 6) & 0xf;
            sprintf(buf, "%-4s %s,%d", instructions[j].str,
                addr_mode(s1, opcode & 0x3f), 
                count ? count : 16);
            break;
        case Optype_10:
            offset = opcode & 0xff;
            sprintf(buf, "%-4s %d", instructions[j].str, offset);        
            break;
        case Optype_6:
            offset = ((int)opcode << (INTLEN-8)) >> (INTLEN-9);
            sprintf(buf, "%-4s %04x", instructions[j].str, addr+2+offset);
            break;
        case Optype_8: 
            if(opcode & 0x00f0) {
                // count in the instruction
                sprintf(buf, "%-4s r%d,%d", instructions[j].str, 
                    opcode & 0xf, (opcode >> 4) & 0xf);
            } else {
                sprintf(buf, "%-4s r%d,r0", instructions[j].str, opcode & 0xf);
            }
            break;
        case Optype_5:
            sprintf(buf, "%-4s r%d,%04x", instructions[j].str, opcode & 0xf, get_word()); 
            break;
        case Optype_7:
            sprintf(buf, "%-4s %04x", instructions[j].str, get_word()); 
            break;
        case Optype_4:
            sprintf(buf, "%-4s r%d", instructions[j].str, opcode & 0xf); 
            break;
        case Optype_9:
            sprintf(buf, "%-4s ", instructions[j].str); 
            break;
        default:
            sprintf(buf, "????");
            break;
    }
    return ptr;
}

