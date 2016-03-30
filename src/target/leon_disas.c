/**
 * OOCD target for Leon (Sparc v8) CPUs
 * (C)2016 Roman Bartosinski, daiteq,s.r.o.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "leon.h"


#define LEON_DISAS_BUFFER_SIZE     30
static char leon_disas_buffer[LEON_DISAS_BUFFER_SIZE];
static char leon_disas_aux_buffer[LEON_DISAS_BUFFER_SIZE];

#define LDIS_OP(op)          (((op) >> 30) & 0x00000003)
#define LDIS_OP1_DISP30(op)  ((op & 0x20000000) ? 1+(0xC0000000 | (op & 0x3FFFFFFF)) : (((op) >>  0) & 0x1FFFFFFF))
#define LDIS_OP2(op)         (((op) >> 22) & 0x00000007)
#define LDIS_OP2_RD(op)      (((op) >> 25) & 0x0000001F)
#define LDIS_OP2_IMM22(op)   (((op) >>  0) & 0x003FFFFF)
#define LDIS_OP2_A(op)       (((op) >> 29) & 0x00000001)
#define LDIS_OP2_COND(op)    (((op) >> 25) & 0x0000000F)
#define LDIS_OP2_DISP22(op)  ((op & 0x00200000) ? 1+(0xFFC00000 | (op & 0x003FFFFF)) : (((op) >>  0) & 0x001FFFFF))
#define LDIS_OP3_RD(op)      (((op) >> 25) & 0x0000001F)
#define LDIS_OP3(op)         (((op) >> 19) & 0x0000003F)
#define LDIS_OP3_RS1(op)     (((op) >> 14) & 0x0000001F)
#define LDIS_OP3_I(op)       (((op) >> 13) & 0x00000001)
#define LDIS_OP3_ASI(op)     (((op) >>  5) & 0x000000FF)
#define LDIS_OP3_RS2(op)     (((op) >>  0) & 0x0000001F)
#define LDIS_OP3_SIMM13(op)  ((op & 0x00001000) ? 1+(0xffffe000 | (op & 0x0001FFF)) : (((op) >>  0) & 0x0000FFF))
#define LDIS_OP3_OPF(op)     (((op) >>  5) & 0x000001FF)


static const char *op2_names[] = {
  "unimp", "-illegal-", "b", "-illegal-", "sethi", "-illegal-", "fb", "cb"
};
static const char *op2_bcond[] = {
  "n", "e", "le", "l", "leu", "cs", "neg", "vs", "a", "ne", "g", "ge", "gu", "cc", "pos", "bvs"
};
static const char *op2_fbcond[] = {
  "n", "ne", "lg", "ul", "l", "ug", "g", "u", "a", "e", "ue", "ge", "uge", "le", "ule", "o"
};
static const char *op2_cbcond[] = {
  "n", "123", "12", "13", "1", "23", "2", "3", "a", "0", "03", "02", "023", "01", "013", "012"
};

static const char *op32_names[] = {
  "add", "and", "or", "xor", "sub", "andn", "orn", "xnor", "addx", "", "umul", "smul", "subx", "", "udiv", "sdiv",
  "addcc", "andcc", "orcc", "xorcc", "subcc", "andncc", "orncc", "xnorcc", "addxcc", "", "umulcc", "smulcc", "subxcc", "", "udivcc", "sdivcc",
  "taddcc", "tsubcc", "taddcctv", "tsubcctv", "mulscc", "sll", "srl", "sra", "rd", "rdpsr", "rdwim", "rdtbr", "", "", "", "",
  "wr", "wrpsr", "wrwim", "wrtbr", "FPop1", "FPop2", "CPop1", "CPop2", "jmpl", "rett", "t", "flush", "save", "restore", "", ""
};
static const char *op33_names[] = {
  "ld", "ldub", "lduh", "ldd", "st", "stb", "sth", "std", "", "ldsb", "ldsh", "", "", "ldstub", "", "swap",
  "lda", "lduba", "lduha", "ldda", "sta", "stba", "stha", "stda", "", "ldsba", "ldsha", "", "", "ldstuba", "", "swapa",
  "ldf", "ldfsr", "", "lddf", "stf", "stfsr", "stdfq", "stdf", "", "", "", "", "", "", "", "",
  "ldc", "ldcsr", "", "lddc", "stc", "stcsr", "stdcq", "stdc", "", "", "", "", "", "", "", ""
};

static const char *regs[] = {
  "%g0", "%g1", "%g2", "%g3",
  "%g4", "%g5", "%g6", "%g7",
  "%o0", "%o1", "%o2", "%o3",
  "%o4", "%o5", "%o6", "%o7",
  "%l0", "%l1", "%l2", "%l3",
  "%l4", "%l5", "%l6", "%l7",
  "%i0", "%i1", "%i2", "%i3",
  "%i4", "%i5", "%i6", "%i7",
};


/* leon2 (sparc v8) */
static char *leon2_disas(enum leon_type lt, uint32_t addr, uint32_t opcode)
{
  const char *op;
  const char *cond;
  const char *annul;
  char *val;
  const char *reg;

  cond = ""; annul = ""; val = ""; reg = "";

  switch (LDIS_OP(opcode)) {
    case 0: // format 2
      if (opcode == 0x01000000) {
        op = "nop";
      } else {
        op = op2_names[LDIS_OP2(opcode)];
        switch (LDIS_OP2(opcode)) {
          case 4:
            snprintf(leon_disas_aux_buffer, 20, "%%hi(0x%08X),", LDIS_OP2_IMM22(opcode)<<10);
            val = leon_disas_aux_buffer;
            reg = regs[LDIS_OP2_RD(opcode)];
            break;
          case 2:
          case 6:
          case 7:
            switch (LDIS_OP2(opcode)) {
              case 2: cond = op2_bcond[LDIS_OP2_COND(opcode)]; break;
              case 6: cond = op2_fbcond[LDIS_OP2_COND(opcode)]; break;
              case 7: cond = op2_cbcond[LDIS_OP2_COND(opcode)]; break;
            }
            if (LDIS_OP2_A(opcode)) annul = ",a";
            snprintf(leon_disas_aux_buffer, 20, "0x%08X", addr+LDIS_OP2_DISP22(opcode));
            val = leon_disas_aux_buffer;
            break;
        }
      }
      snprintf(leon_disas_buffer, LEON_DISAS_BUFFER_SIZE, "%s%s%s  %s %s", op, cond, annul, val, reg);
      break;

    case 1: // format 1
      snprintf(leon_disas_buffer, LEON_DISAS_BUFFER_SIZE, "call  0x%08X", addr + (LDIS_OP1_DISP30(opcode)<<2));
      break;

    case 2: // format 3
      op = op32_names[LDIS_OP3(opcode)];
      if (LDIS_OP3(opcode)==0x3A) {
        cond = op2_bcond[LDIS_OP2_COND(opcode)];
      }
//printf("op3 = 0x%X = '%s'\n", LDIS_OP3(opcode), op);
      snprintf(leon_disas_buffer, LEON_DISAS_BUFFER_SIZE, "%s%s", op, cond);
      break;
    case 3: // format 3
      op = op33_names[LDIS_OP3(opcode)];
      snprintf(leon_disas_buffer, LEON_DISAS_BUFFER_SIZE, "%s", op);
      break;
  }
  return leon_disas_buffer;
}


/* -------------------------------------------------------------------------- */
char *leon_disas(enum leon_type lt, uint32_t addr, uint32_t opcode)
{
  return leon2_disas(lt, addr, opcode);
}
