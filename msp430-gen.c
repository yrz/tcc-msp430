/*
 *  MSP430 code generator for TCC
 * 
 *  Copyright (c) 2001, 2002 Fabrice Bellard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "msp430-opcodes.h"

#define ASSEMBLY_LISTING_MSP430
#define DEBUG 1

/* number of available registers */
#define NB_REGS            16

/* pretty names for the registers */
enum
{
  TREG_R0 = 0,			// PC program counter, jump assigning here
  TREG_R1,			// SP stack pointer, use to call and push
  TREG_R2,			// SR status register
  TREG_R3,			// hardwired to 0
  TREG_R4,
  TREG_R5,
  TREG_R6,
  TREG_R7,
  TREG_R8,
  TREG_R9,
  TREG_R10,
  TREG_R11,
  TREG_R12,
  TREG_R13,
  TREG_R14,
  TREG_R15
};

#define RC_INT		0x0001
#define RC_FLOAT	0x0002

int reg_classes[NB_REGS] = {
  0 /*  TREG_R0 */ ,		// PC program counter
  0 /*  TREG_R1 */ ,		// SP stack pointer, use to call and push
  0 /*  TREG_R2 */ ,		// SR status register
  0 /* TREG_R3 */ ,		// hardwired to 0
  0 /* TREG_R4 */ ,
  TREG_R5 | RC_INT | RC_FLOAT,
  TREG_R6 | RC_INT | RC_FLOAT,
  TREG_R7 | RC_INT | RC_FLOAT,
  TREG_R8 | RC_INT | RC_FLOAT,
  TREG_R9 | RC_INT | RC_FLOAT,
  TREG_R10 | RC_INT | RC_FLOAT,
  TREG_R11 | RC_INT | RC_FLOAT,
  TREG_R12 | RC_INT | RC_FLOAT,
  TREG_R13 | RC_INT | RC_FLOAT,
  TREG_R14 | RC_INT | RC_FLOAT,
  TREG_R15 | RC_INT | RC_FLOAT,
};

#define	RC_IRET		TREG_R15
#define	REG_IRET	TREG_R15
#define	RC_LRET		TREG_R14
#define REG_LRET	TREG_R14

/* float RET */
#define RC_FRET		TREG_R15
#define REG_FRET	TREG_R15
#define RC_FRET2	TREG_R14

#define ALWAYS_ASSERT(x) \
do {\
   if (!(x))\
       error("internal compiler error file at %s:%d", __FILE__, __LINE__);\
} while (0)

#define LDOUBLE_SIZE	8
#define LDOUBLE_ALIGN	8
#define PTR_SIZE 2

/* maximum alignment (for aligned attribute support) */
#define MAX_ALIGN     4

/******************************************************/
/* ELF defines */

#define EM_TCC_TARGET EM_MSP430

/* relocation type for 32 bit data relocation */
#define R_DATA_32	2
#define R_JMP_SLOT	0
#define R_COPY		2

#define ELF_START_ADDR 0x00008000
#define ELF_PAGE_SIZE  0x0100

/******************************************************/
static unsigned long func_sub_sp_offset;
static unsigned long func_param_mem;


#ifdef ASSEMBLY_LISTING_MSP430
FILE *f = NULL;
#endif

void
o16 (int c)
{
  int ind1;

#ifdef DEBUG
  printf (" %04x ind = %08x\n", c, ind);
#endif
  ind1 = ind + 2;
  if (ind1 > cur_text_section->data_allocated)
    section_realloc (cur_text_section, ind1);
  cur_text_section->data[ind] = c & 0xff;
  cur_text_section->data[ind + 1] = (c >> 8) & 0xff;
  ind = ind1;
}

int
bld_dbl_op (int op, int bytemod, int as, int src, int ad, int dst)
{
  int r;
  r = op | (bytemod << 6) | (as << 4) | (src << 8) | (as << 7) | dst;
  return r;
}

int
bld_sgl_op (int op, int bytemod, int as, int src, int ad, int dst)
{
  int r;
  r = op | (bytemod << 6) | (as << 4) | (src << 8) | (as << 7) | dst;
  return r;
}

int
bld_dbl_op (int op, int bytemod, int as, int src, int ad, int dst)
{
  int r;
  r = op | (bytemod << 6) | (as << 4) | (src << 8) | (as << 7) | dst;
  return r;
}

/* load 'r' from value 'sv' */
void
load (int r, SValue * sv)
{
  int v, t, ft, fc, fr;
  SValue v1;

  fr = sv->r;
  ft = sv->type.t;
  fc = sv->c.ul;

#ifdef DEBUG
  printf ("LOAD fr %04x ft %04x fc %04x\n", fr, ft, fc);
#endif

  v = fr & VT_VALMASK;

  if (v == VT_LDOUBLE)
  {
    error ("long double not supported");
  }

  if (v == VT_CONST)
  {
    o16 (MOV | (AD_INC << 4) | r);	/* direct = 0 and immediate = 0 */
    o16 (fc);
  }
}

/* store register 'r' in lvalue 'v' */
void
store (int r, SValue * v)
{
  int fr, bt, ft, fc;

  ft = v->type.t;
  fc = v->c.ul;
  fr = v->r & VT_VALMASK;
  bt = ft & VT_BTYPE;

#ifdef DEBUG
  printf ("STORE on r%d fr %04x ft %04x fc %04x bt %04x\n", r, fr, ft, fc,
	  bt);
#endif

/*
        if (bt == VT_INT) {
	    o16(MOV | TREG_R4 | AD_IDX);
	    o16(fc & 0xffff);
	    */
}

/* 'is_jmp' is '1' if it is a jump */
static void
gcall_or_jmp (int is_jmp)
{
}

/* generate function call with address in (vtop->t, vtop->c) and free function
   context. Stack entry is popped */
void
gfunc_call (int nb_args)
{
  o16 (0x120b); // o16 ( PUSH | TREG_R11 );
  o16 (0x0000);
}


void
gfunc_prolog (CType * func_type)
{
  int offset, align, size, func_call;
  int param_index, param_addr;
  Sym *sym;
  CType *type;
  int func_args;

  int frame_size;

#ifdef DEBUG
  printf ("INIT prolog ind=%d\n", ind);
#endif

  sym = func_type->ref;
  func_call = FUNC_CALL (sym->r);
  func_args = FUNC_ARGS (sym->r);
  offset = 0;
  loc = 0;
  param_index = 0;

  //ind += FUNC_PROLOG_SIZE; //XXX calc REAL size 
  func_sub_sp_offset = ind;

  /* if the function returns a structure, then add an
     implicit pointer parameter */
  func_vt = sym->type;
  if ((func_vt.t & VT_BTYPE) == VT_STRUCT)
  {
    printf ("do your function return a struct?\n");
  }

  while ((sym = sym->next))
  {
    type = &sym->type;
    sym_push (sym->v & ~SYM_FIELD, type, VT_LOCAL | lvalue_type (type->t),
	      offset);
    size = type_size (type, &align);
    size = ((size + 1) & ~1);
#ifdef DEBUG
    printf ("size = type_size(type, &align) = %d, ind = %d\n", size, ind);
#endif
    offset += size;
#ifdef DEBUG
    printf ("offset %d, align %d, size %d, func_call %d\n", offset, align,
	    size, func_call);
    printf ("param_index %d, param_addr %d, func_args %d\n", param_index,
	    param_addr, func_args);
#endif
    /* XXX use registries? */
  }
/* ?  func_ret_sub = 0; */

  o16 (0x1204);			// push r4
  if (offset > 0)
  {
    if (offset == 2 || offset == 4 || offset == 8)
    {
      o16 (0x8031 | offset << 8);
    }
    else
    {
      o16 (0x8031);
      o16 (offset);
    }
  }
  func_param_mem = offset;

  o16 (0x4104);			// mov r1,r4

#ifdef DEBUG
  printf ("END prolog\n");
#endif
/*
    frame_size=4;
    o16(SUB | TREG_R1 | AS_IND | (frame_size<<7));
    o16(MOV | TREG_R4 | (TREG_R1<<8));           
    */

#ifdef DEBUG
  printf ("\n");
#endif

  /* XXX: implement bound checking code */
}

/* generate function epilog */
void
gfunc_epilog (void)
{
  int frame_size = func_param_mem;

  o16 (0x450f);			//XXX mov r5, r15
  if (func_param_mem > 0)
  {
    //XXX optimize for 2 4 8
    o16 (0x8031);		/* sub #-frame_size */
    o16 (-frame_size);
  }
  o16 (POP | TREG_R4);
  o16 (RET);
#ifdef DEBUG
  printf ("epilog func_sub_sp_offset %d\n", func_sub_sp_offset);
#endif
}

/* generate a jump to a label */
int
gjmp (int t)
{
  return 0;
}

/* generate a jump to a fixed address */
void
gjmp_addr (int a)
{
}

/* generate a test. set 'inv' to invert test. Stack entry is popped */
int
gtst (int inv, int t)
{
  return 0;
}

/* generate an integer binary operation */
void
gen_opi (int op)
{
  int r, opc, c;
#ifdef DEBUG
  printf ("OPI op %d\n", op);
#endif

  switch (op)
  {
  case '+':
  case TOK_ADDC1:		/* add with carry generation */
    opc = 0;
  gen_op8:
    if ((vtop->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST)
    {
      r = gv (RC_INT);
      printf ("HMM r %d and c %d\n", r, vtop->c.i);
      vswap ();
      c = vtop->c.i;
      o16 (0x5030 + r);
      o16 (c);
    }
//      if (c == (char)c) {
//                 /* XXX: generate inc and dec for smaller code ? */
//                 o(0x83);
//                 o(0xc0 | (opc << 3) | r);
//                 g(c);
//             } else {
//                 o(0x81);
//                 oad(0xc0 | (opc << 3) | r, c);
//             }
//         } else {
//             gv2(RC_INT, RC_INT);
//             r = vtop[-1].r;
//             fr = vtop[0].r;
//             o((opc << 3) | 0x01);
//             o(0xc0 + r + fr * 8); 
//         }
//         vtop--;
//         if (op >= TOK_ULT && op <= TOK_GT) {
//             vtop->r = VT_CMP;
//             vtop->c.i = op;
//         }
    break;
  case '-':
  case TOK_SUBC1:		/* sub with carry generation */
    opc = 5;
    goto gen_op8;
  case TOK_ADDC2:		/* add with carry use */
    opc = 2;
    goto gen_op8;
  case TOK_SUBC2:		/* sub with carry use */
    opc = 3;
    goto gen_op8;
  default:
    printf ("unsupported operator\n");
  }
}

/* generate a floating point operation 'v = t1 op t2' instruction. The
   two operands are guaranted to have the same floating point type */
/* XXX: need to use ST1 too */
void
gen_opf (int op)
{
#ifdef DEBUG
  printf ("OPF op %d\n", op);
#endif
}


/* convert integers to fp 't' type. Must handle 'int', 'unsigned int'
   and 'long long' cases. */
void
gen_cvt_itof (int t)
{
}

/* convert fp to int 't' type */
/* XXX: handle long long case */
void
gen_cvt_ftoi (int t)
{
}

/* convert from one floating point type to another */
void
gen_cvt_ftof (int t)
{
}

/* computed goto support */
void
ggoto (void)
{
}


/* output a symbol and patch all calls to it */
void
gsym_addr (int t, int a)
{
}

void
gsym (int t)
{
  gsym_addr (t, ind);
}

/* end of MSP430 code generator */
