/**
 * OOCD target for Leon (Sparc v8) CPUs
 * (C)2016 Roman Bartosinski, daiteq,s.r.o.
 *
 */

#ifndef LEON_OOCD_TARGET_HEADER_FILE
#define LEON_OOCD_TARGET_HEADER_FILE

#include <elf.h>

#include <jtag/jtag.h>
#include <helper/time_support.h>
#include "target.h"
#include "target_type.h"
#include "breakpoints.h"

#include "leon_regs.h"

/* ========================================================================== */
/* LEON_BSCAN protocol */

#define LEON_IRINS_ENTER		0x02	/* use USER1 register */
#define LEON_IRINS_EXIT			0x3F	/* use BYPASS ??? */
#define LEON_IRINS_IDCODE		0x09	/* IDCODE */

#define LEON_DRSCAN_NBITS 18

#define LEON_PID_PAD      (0x00000)
#define LEON_PID_ADDRESS  (0x10000)
#define LEON_PID_DATA     (0x20000)
#define LEON_PID_CONTROL  (0x30000)
#define LEON_PID_STATUS   (0x30000)

#define LEON_PID_MASK     (0x30000)
#define LEON_VALUE_MASK   (0x0FFFF)

/* control word */
#define LEON_CTRL_WRITE_DATA   0x8000   /* write data */
#define LEON_CTRL_READ_DATA    0x4000   /* read data */
#define LEON_CTRL_RAT_DSU      0x2000   /* reset and return testing word as status */
#define LEON_CTRL_ADDR_INC     0x1000   /* auto-increment address */
#define LEON_CTRL_MASK_AI_LEN  0x0FFF   /* number of auto-incremented address */

/* status word */
#define LEON_STAT_TESTWORD     (LEON_PID_PAD | 0x4C32) /* LEON BSCAN test word */

#define LEON_STAT_INRESET      0x8000
#define LEON_STAT_PROCESSING   0x4000
#define LEON_STAT_IN_OVER      0x1000
#define LEON_STAT_IN_FULL      0x0800
#define LEON_STAT_IN_HALF      0x0400
#define LEON_STAT_DLEN_MASK    0x03FF

#define LEON_CRC_INIT          0xFFFF  /* CRC16-CCITT x16+x12+x5+1,init=0xffff */
#define LEON_BLOCK_MAX_LENGTH  0x03FE
#define LEON_BLOCK_UNLIMITED   0x03FF

/* -------------------------------------------------------------------------- */
/* OOCD target */
#define LEON_TYPE_NAME   "leon"

enum leon_type {
	LEON_TYPE_UNKNOWN,
	LEON_TYPE_L2,
	LEON_TYPE_L2FT,
	LEON_TYPE_L2MT,
};

enum leon_trace_type {
	LEON_TRCTYPE_UNKNOWN,
	LEON_TRCTYPE_NONE,
	LEON_TRCTYPE_CPU,
	LEON_TRCTYPE_AHB,
	LEON_TRCTYPE_BOTH,
};

struct leon_elf_section;
typedef struct leon_elf_section {
	uint8_t  index;
	char    *name;
	uint32_t type;
	uint32_t flags;
	struct leon_elf_section *next;
} leon_elf_section_t;

struct leon_elf_symbol;
typedef struct leon_elf_symbol {
	char      *name;
	uint32_t   value;
	uint32_t   size;
	uint8_t    info;
	uint8_t    other;
	uint8_t    secidx;
	struct leon_elf_symbol *next;
} leon_elf_symbol_t;


struct leon_common {
	struct jtag_tap *tap;
	uint32_t   loptime; // processing time [us] of the last operation
	uint32_t   loptime_jtag; // time [us] of the last jtag operation

	/* leon type */
	enum leon_type ltype;
	int            mt_ctlblk_size;

	/* tracing */
	enum leon_trace_type trctype;

	/* copy and desription of registers */
	struct reg_cache *regdesc;
	uint32_t          regvals[LEON_RID_ALLSIZE];    // cached register values (test rid2rdi[x]!=-1 if it is used)
	int               *rid_to_rdi; // [LEON_RID_ALLSIZE];    // map RID -> regdesc index
	int               *rdi_to_rid; // [LEON_RID_ALLSIZE];    // map regdesc index -> RID
//	uint32_t          rid2hwaddr[LEON_RID_ALLSIZE]; // map RID -> hw memory address
	uint32_t          rdi2hwaddr[LEON_RID_ALLSIZE]; // map regdesc index -> hw memory address

	/* loaded ELF (sections, symbols) */
	int                 lelf_nsecs;
	leon_elf_section_t *lelf_sects;
	int                 lelf_nsyms;
	leon_elf_symbol_t  *lelf_symbs;
	uint32_t            lelf_epoint;
};

static inline struct leon_common *
target_to_leon(struct target *target)
{
	return (struct leon_common *) target->arch_info;
}

static inline uint32_t leon_swap_u32(uint32_t in)
{
	return ((in>>24)&0xff) | ((in>>8)&0xff00) | ((in<<8)&0xff0000) | ((in<<24)&0xff000000);
}

static inline uint16_t leon_swap_u16(uint16_t in)
{
	return ((in>>8)&0xff) | ((in<<8)&0xff00);
}

#define LEON_DSU_TRAP_INSTR  0x91D02001    // Sparc v8 inst. TA 1 -> DSU break

/* -------------------------------------------------------------------------- */
/* leon_jtag.c */
int leon_jtag_set_instr(struct jtag_tap *tap, uint32_t new_instr, int forced);
int leon_jtag_exchng_data(struct jtag_tap *tap, uint32_t *sdata,
                          int scnt, uint32_t *rdata, int rcnt, int rskip);

int leon_jtag_get_registers(struct target *target, uint32_t addr,
                                   uint32_t *data, int cnt);
int leon_jtag_set_registers(struct target *target, uint32_t addr,
                                   uint32_t *data, int cnt);

/* -------------------------------------------------------------------------- */
//uint32_t leon_get_current_time(void);
#define LEON_TM_START(_tmvar_)  \
        struct duration _tmvar_; duration_start(&_tmvar_)
#define LEON_TM_MEASURE(_tmvar_, _loptm_)  \
        do { \
          if (duration_measure(&_tmvar_)==ERROR_OK) \
            _loptm_ = (uint32_t)(1000000*duration_elapsed(&_tmvar_)); \
          else \
            _loptm_ = 0; \
				} while(0)

char *leon_elf_val2sym(struct leon_common *pleon, const char *secname, uint32_t val);
int leon_elf_sym2val(struct leon_common *pleon, const char *secname, const char *sym, uint32_t *val);

/* -------------------------------------------------------------------------- */
/* leon_disas.c */
char *leon_disas(struct leon_common *pl, uint32_t addr, uint32_t opcode);

#endif /* LEON_OOCD_TARGET_HEADER_FILE */
