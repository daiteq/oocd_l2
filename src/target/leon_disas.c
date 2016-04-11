/**
 * OOCD target for Leon (Sparc v8) CPUs
 * (C)2016 Roman Bartosinski, daiteq,s.r.o.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "leon.h"


#define LEON_DISAS_BUFFER_SIZE     40
static char leon_disas_buffer[LEON_DISAS_BUFFER_SIZE];
#define LEON_DISAS_AUXBUF_SIZE     25
static char leon_disas_aux_buffer[LEON_DISAS_AUXBUF_SIZE];

#define LDIS_OP(op)          (((op) >> 30) & 0x00000003)
#define LDIS_OP1_DISP30(op)  ((op & 0x20000000) ? (0xC0000000 | (op & 0x3FFFFFFF)) : (((op) >>  0) & 0x1FFFFFFF))
#define LDIS_OP2(op)         (((op) >> 22) & 0x00000007)
#define LDIS_OP2_RD(op)      (((op) >> 25) & 0x0000001F)
#define LDIS_OP2_IMM22(op)   (((op) >>  0) & 0x003FFFFF)
#define LDIS_OP2_A(op)       (((op) >> 29) & 0x00000001)
#define LDIS_OP2_COND(op)    (((op) >> 25) & 0x0000000F)
#define LDIS_OP2_DISP22(op)  ((op & 0x00200000) ? (((0xFFC00000 | (op & 0x003FFFFF)))<<2) : ((((op) >>  0) & 0x001FFFFF)<<2))
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
  "add", "and", "or", "xor", "sub", "andn", "orn", "xnor",
  "addx", "", "umul", "smul", "subx", "", "udiv", "sdiv",
  "addcc", "andcc", "orcc", "xorcc", "subcc", "andncc", "orncc", "xnorcc",
  "addxcc", "", "umulcc", "smulcc", "subxcc", "", "udivcc", "sdivcc",
  "taddcc", "tsubcc", "taddcctv", "tsubcctv", "mulscc", "sll", "srl", "sra",
  "rd", "rdpsr", "rdwim", "rdtbr", "", "", "", "",
  "wr", "wr", "wr", "wr", "FPop1", "FPop2", "CPop1", "CPop2",
  "jmpl", "rett", "t", "flush", "save", "restore", "", ""
};

static const int op32_spec_reg[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 0, 0, 0, 0,
  1, 2, 3, 4, 0, 0, 0, 0, 0, -1, -1, -1, 0, 0, 0, 0
};

static const char *op33_names[] = {
  "ld", "ldub", "lduh", "ldd", "st", "stb", "sth", "std", "", "ldsb", "ldsh", "", "", "ldstub", "", "swap",
  "lda", "lduba", "lduha", "ldda", "sta", "stba", "stha", "stda", "", "ldsba", "ldsha", "", "", "ldstuba", "", "swapa",
  "ldf", "ldfsr", "", "lddf", "stf", "stfsr", "stdfq", "stdf", "", "", "", "", "", "", "", "",
  "ldc", "ldcsr", "", "lddc", "stc", "stcsr", "stdcq", "stdc", "", "", "", "", "", "", "", ""
};
static const int op33_swap_rd[] = {
  0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0
};

static const char *reg_spec[] = {
  "%y", "%asr1", "%asr2", "%asr3", "%asr4", "%asr5", "%asr6", "%asr7",
  "%asr8", "%asr9", "%asr10", "%asr11", "%asr12", "%asr13", "%asr14", "%asr15",
  "%asr16", "%asr17", "%asr18", "%asr19", "%asr20", "%asr21", "%asr22", "%asr23",
  "%asr24", "%asr25", "%asr26", "%asr27", "%asr28", "%asr29", "%asr30", "%asr31",
  "%psr", "%wim", "%tbr"
};

static const char *regs_l2[] = {
  "%g0", "%g1", "%g2", "%g3",
  "%g4", "%g5", "%g6", "%g7",
  "%o0", "%o1", "%o2", "%o3",
  "%o4", "%o5", "%o6", "%o7",
  "%l0", "%l1", "%l2", "%l3",
  "%l4", "%l5", "%l6", "%l7",
  "%i0", "%i1", "%i2", "%i3",
  "%i4", "%i5", "%i6", "%i7",
};

static const char *regs_l2mt[] = {
  "%r0", "%r1", "%r2", "%r3",
  "%r4", "%r5", "%r6", "%r7",
  "%r8", "%r9", "%r10", "%r11",
  "%r12", "%r13", "%r14", "%r15",
  "%r16", "%r17", "%r18", "%r19",
  "%r20", "%r21", "%r22", "%r23",
  "%r24", "%r25", "%r26", "%r27",
  "%r28", "%r29", "%r30", "%r31",
};

typedef struct leon_disas_params {
  enum leon_type lt;
  uint32_t       addr;
  uint32_t       opcode;
  const char    **regset;
} leon_disas_params_t;

/* leon2 (sparc v8) */
static char *leon2_disas(leon_disas_params_t *pldp)
{
  uint32_t o = pldp->opcode;
  const char *op;
  const char *cond;
  const char *annul;
  char *val;
  const char *reg;
  int done = 0, swaprd=0;

  op = ""; cond = ""; annul = ""; val = ""; reg = "";

  switch (LDIS_OP(o)) {
    case 0: // format 2
      if (o == 0x01000000) {
        op = "nop";
      } else {
        op = op2_names[LDIS_OP2(o)];
        switch (LDIS_OP2(o)) {
          case 4:
            snprintf(leon_disas_aux_buffer, LEON_DISAS_AUXBUF_SIZE,
                     "%%hi(0x%08X),", LDIS_OP2_IMM22(o)<<10);
            val = leon_disas_aux_buffer;
            reg = pldp->regset[LDIS_OP2_RD(o)];
            break;
          case 2:
          case 6:
          case 7:
            switch (LDIS_OP2(o)) {
              case 2: cond = op2_bcond[LDIS_OP2_COND(o)]; break;
              case 6: cond = op2_fbcond[LDIS_OP2_COND(o)]; break;
              case 7: cond = op2_cbcond[LDIS_OP2_COND(o)]; break;
            }
            if (LDIS_OP2_A(o)) annul = ",a";
            snprintf(leon_disas_aux_buffer, LEON_DISAS_AUXBUF_SIZE,
                     "0x%08X", pldp->addr + LDIS_OP2_DISP22(o));
            val = leon_disas_aux_buffer;
            break;
        }
      }
      snprintf(leon_disas_buffer, LEON_DISAS_BUFFER_SIZE, "%s%s%s  %s %s",
                op, cond, annul, val, reg);
      break;

    case 1: // format 1
      op = "call";
      snprintf(leon_disas_buffer, LEON_DISAS_BUFFER_SIZE, "%s  0x%08X",
                op, pldp->addr + (LDIS_OP1_DISP30(o)<<2));
      break;

    case 2: // format 3
      if (LDIS_OP3(o)==0x38 && LDIS_OP3_I(o) && LDIS_OP3_RD(o)==0) { // jmpl
        if (LDIS_OP3_RS1(o)==0) { // rs1=g0
          op = "jmp";
          snprintf(leon_disas_aux_buffer, LEON_DISAS_AUXBUF_SIZE,
                   "0x%08X", LDIS_OP3_SIMM13(o));
          val = leon_disas_aux_buffer;
          done = 1;
        } else if (LDIS_OP3_RS1(o)==31 && LDIS_OP3_SIMM13(o)==8) { // rs1=i7 && simm13=8
          op = "ret";
          done = 1;
        } else if (LDIS_OP3_RS1(o)==15 && LDIS_OP3_SIMM13(o)==8) { // rs1=i7 && simm13=8
          op = "retl";
          done = 1;
        }
      }
      if (LDIS_OP3(o)==0x02) { // or
        if (LDIS_OP3_RS1(o)==0) { // rs1=g0
          if (!LDIS_OP3_I(o) && LDIS_OP3_RS2(o)==0) { // rs2=g0 -> clr
            op = "clr";
            reg = pldp->regset[LDIS_OP3_RD(o)];
            done = 1;
          } else {
            op = "mov";
            if (LDIS_OP3_I(o)) {
              snprintf(leon_disas_aux_buffer, LEON_DISAS_AUXBUF_SIZE,
                       "0x%04X,", LDIS_OP3_SIMM13(o));
            } else {
              snprintf(leon_disas_aux_buffer, LEON_DISAS_AUXBUF_SIZE,
                       "%s,", pldp->regset[LDIS_OP3_RS2(o)]);
            }
            val = leon_disas_aux_buffer;
            reg = pldp->regset[LDIS_OP3_RD(o)];
            done = 1;
          }
        }
      }
      if (!done) {
        op = op32_names[LDIS_OP3(o)];
        int s = op32_spec_reg[LDIS_OP3(o)];
        int plus = 0 ;
        if (LDIS_OP3(o)==0x3A) {
          cond = op2_bcond[LDIS_OP2_COND(o)];
          plus = 1;
        }
        if (LDIS_OP3_I(o)) {
          if (plus) {
            if (LDIS_OP3_RS1(o)==0) { /* RS1 == r0 == 0 */
              snprintf(leon_disas_aux_buffer, LEON_DISAS_AUXBUF_SIZE,
                      "0x%08X%s", LDIS_OP3_SIMM13(o), (s<0) ? "" : "," );
            } else {
              snprintf(leon_disas_aux_buffer, LEON_DISAS_AUXBUF_SIZE,
                      "%s + 0x%08X%s", pldp->regset[LDIS_OP3_RS1(o)],
                      LDIS_OP3_SIMM13(o), (s<0) ? "" : "," );
            }
          } else {
            snprintf(leon_disas_aux_buffer, LEON_DISAS_AUXBUF_SIZE,
                     "%s, 0x%08X%s", pldp->regset[LDIS_OP3_RS1(o)],
                     LDIS_OP3_SIMM13(o), (s<0) ? "" : "," );
          }
        } else {
          if (plus) {
            if (LDIS_OP3_RS1(o)==0 || LDIS_OP3_RS2(o)==0) { /* (RS1 | RS2) == r0 == 0 */
              snprintf(leon_disas_aux_buffer, LEON_DISAS_AUXBUF_SIZE,
                       "%s%s",
                       (LDIS_OP3_RS1(o)) ? pldp->regset[LDIS_OP3_RS1(o)]
                                         : pldp->regset[LDIS_OP3_RS2(o)],
                       (s<0) ? "" : "," );
            } else {
              snprintf(leon_disas_aux_buffer, LEON_DISAS_AUXBUF_SIZE,
                       "%s + %s%s", pldp->regset[LDIS_OP3_RS1(o)],
                       pldp->regset[LDIS_OP3_RS2(o)], (s<0) ? "" : "," );
            }
          } else {
            snprintf(leon_disas_aux_buffer, LEON_DISAS_AUXBUF_SIZE,
                     "%s, %s%s", pldp->regset[LDIS_OP3_RS1(o)],
                     pldp->regset[LDIS_OP3_RS2(o)], (s<0) ? "" : "," );
          }
        }
        val = leon_disas_aux_buffer;
        switch (s) {
          case 1:
            reg = reg_spec[LDIS_OP3_RD(o)];
            break;
          case 2: case 3: case 4:
            reg = reg_spec[30 + s];
            break;
          case -1:
            reg = "";
            break;
          default:
            reg = pldp->regset[LDIS_OP3_RD(o)];
            break;
        }
      }
//printf("op3 = 0x%X = '%s'\n", LDIS_OP3(o), op);
      snprintf(leon_disas_buffer, LEON_DISAS_BUFFER_SIZE, "%s%s  %s %s", op, cond, val, reg);
      break;

    case 3: // format 3
      op = op33_names[LDIS_OP3(o)];
      swaprd = op33_swap_rd[LDIS_OP3(o)];
      if (LDIS_OP3_I(o)) {
        snprintf(leon_disas_aux_buffer, LEON_DISAS_AUXBUF_SIZE,
                 "%s[%s + 0x%08X]%s", swaprd ? ", " : "",
                 pldp->regset[LDIS_OP3_RS1(o)], LDIS_OP3_SIMM13(o),
                 swaprd ? "" : ", ");
      } else {
        snprintf(leon_disas_aux_buffer, LEON_DISAS_AUXBUF_SIZE, "%s[%s + %s]%s",
                 swaprd ? ", " : "", pldp->regset[LDIS_OP3_RS1(o)],
                 pldp->regset[LDIS_OP3_RS2(o)], swaprd ? "" : ", ");
      }
      val = leon_disas_aux_buffer;
      reg = pldp->regset[LDIS_OP3_RD(o)];
      if (swaprd)
        snprintf(leon_disas_buffer, LEON_DISAS_BUFFER_SIZE, "%s  %s%s", op, reg, val);
      else
        snprintf(leon_disas_buffer, LEON_DISAS_BUFFER_SIZE, "%s  %s%s", op, val, reg);
      break;
  }
  if (*op==0)
    snprintf(leon_disas_buffer, LEON_DISAS_BUFFER_SIZE, "<unknown>");

  return leon_disas_buffer;
}


/* -------------------------------------------------------------------------- */
/* switch according to leon type (L2,L2FT,L2MT,...) */
char *leon_disas(enum leon_type lt, uint32_t addr, uint32_t opcode)
{
  leon_disas_params_t ldp = {
    .lt = lt, .addr = addr, .opcode = opcode,
  };
  switch (lt) {
    case LEON_TYPE_L2MT:
      ldp.regset = regs_l2mt;
      break;
    default:
      ldp.regset = regs_l2;
      break;
  };
  return leon2_disas(&ldp);
}
