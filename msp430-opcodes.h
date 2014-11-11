/*
 *  MSP430 opcodes
 * 
 *  Copyright (c) 2011 Pierpaolo Giacomin
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* addressing mode */
#define AD_DIR	0x0 /* register direct r4 */
#define AD_IDX	0x1 /* register indexed offset(r4), or #4 as src when r0 -> PCREL */

#define AD_IND	0x2 /* register indirect @r4 */
#define AD_INC	0x3 /* register indirect  with increment @r4+ */

/* unary 
 * sum dest registry
 */
#define RRC	0x1000
#define SWPB	0x1080
#define RRA	0x1100
#define SXT	0x1180
#define PUSH	0x1200
#define CALL	0x1280
#define RETI	0x1300

/* conditional 
 * just plain sum half of jump range -1024 +1022
 */
#define JNZ	0x2000
#define JEQ	0x2400
#define JNC	0x2800
#define JC	0x2c00
#define JN	0x3000
#define JGE	0x3400
#define JL	0x3800
#define JMP	0x3c00

/* dual 
 * sum dest registry
 * sum << 8 src registry
 */
#define MOV	0x4000
#define ADD	0x5000
#define ADDC	0x6000
#define SUBC	0x7000
#define SUB	0x8000
#define CMP	0x9000
#define DADD	0xa000
#define BIT	0xb000
#define BIC	0xc000
#define BIS	0xd000
#define XOR	0xe000
#define AND	0xf000

#define NOP	0x4304

/* sum registry destination */
#define POP	0x4130

/* move to r0/PC */
#define RET	0x4130

/* sum registry source <<8 */
#define BR	0x4130

/* sum to set as byte instruction */
#define BYTE	0x0040
