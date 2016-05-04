/**
 * OOCD target for Leon (Sparc v8) CPUs
 * (C)2016 Roman Bartosinski, daiteq,s.r.o.
 *
 */

#ifndef LEON_OOCD_TARGET_REGISTERS_HEADER_FILE
#define LEON_OOCD_TARGET_REGISTERS_HEADER_FILE

#include "target.h"
#include "register.h"
#include "leon_hwdesc.h"

enum leon_iu_register_indices {
	LEON_RID_R0 = 0,
	LEON_RID_R1, LEON_RID_R2, LEON_RID_R3,
	LEON_RID_R4, LEON_RID_R5, LEON_RID_R6, LEON_RID_R7,
	LEON_RID_R8, LEON_RID_R9, LEON_RID_R10, LEON_RID_R11,
	LEON_RID_R12, LEON_RID_R13, LEON_RID_R14, LEON_RID_R15,
	LEON_RID_R16, LEON_RID_R17, LEON_RID_R18, LEON_RID_R19,
	LEON_RID_R20, LEON_RID_R21, LEON_RID_R22, LEON_RID_R23,
	LEON_RID_R24, LEON_RID_R25, LEON_RID_R26, LEON_RID_R27,
	LEON_RID_R28, LEON_RID_R29, LEON_RID_R30, LEON_RID_R31,

	LEON_RID_Y, LEON_RID_PSR, LEON_RID_WIM, LEON_RID_TBR,
	LEON_RID_PC, LEON_RID_NPC,

	LEON_RID_ASR16, LEON_RID_ASR17, LEON_RID_ASR18, LEON_RID_ASR19,
	LEON_RID_ASR20, LEON_RID_ASR21, LEON_RID_ASR22, LEON_RID_ASR23,
	LEON_RID_ASR24, LEON_RID_ASR25, LEON_RID_ASR26, LEON_RID_ASR27,
	LEON_RID_ASR28, LEON_RID_ASR29, LEON_RID_ASR30, LEON_RID_ASR31,

	LEON_RID_SIZE_IU
};

enum leon_fpu_register_indices {
	LEON_RID_F0 = LEON_RID_SIZE_IU,
	LEON_RID_F1, LEON_RID_F2, LEON_RID_F3,
	LEON_RID_F4, LEON_RID_F5, LEON_RID_F6, LEON_RID_F7,
	LEON_RID_F8, LEON_RID_F9, LEON_RID_F10, LEON_RID_F11,
	LEON_RID_F12, LEON_RID_F13, LEON_RID_F14, LEON_RID_F15,
	LEON_RID_F16, LEON_RID_F17, LEON_RID_F18, LEON_RID_F19,
	LEON_RID_F20, LEON_RID_F21, LEON_RID_F22, LEON_RID_F23,
	LEON_RID_F24, LEON_RID_F25, LEON_RID_F26, LEON_RID_F27,
	LEON_RID_F28, LEON_RID_F29, LEON_RID_F30, LEON_RID_F31,

	LEON_RID_FSR,

	LEON_RID_SIZE_FPU
};

enum leon_cu_register_indices {
	LEON_RID_C0 = LEON_RID_SIZE_FPU,
	LEON_RID_C1, LEON_RID_C2, LEON_RID_C3,
	LEON_RID_C4, LEON_RID_C5, LEON_RID_C6, LEON_RID_C7,
	LEON_RID_C8, LEON_RID_C9, LEON_RID_C10, LEON_RID_C11,
	LEON_RID_C12, LEON_RID_C13, LEON_RID_C14, LEON_RID_C15,
	LEON_RID_C16, LEON_RID_C17, LEON_RID_C18, LEON_RID_C19,
	LEON_RID_C20, LEON_RID_C21, LEON_RID_C22, LEON_RID_C23,
	LEON_RID_C24, LEON_RID_C25, LEON_RID_C26, LEON_RID_C27,
	LEON_RID_C28, LEON_RID_C29, LEON_RID_C30, LEON_RID_C31,

	LEON_RID_CSR,

	LEON_RID_SIZE_CPU
};

enum leon_peri_register_indices {
	LEON_RID_LCFG = LEON_RID_SIZE_CPU,
	LEON_RID_CCR,

	LEON_RID_SIZE_PERI
};

enum leon_dsu_register_indices {
	LEON_RID_DSUCTRL = LEON_RID_SIZE_PERI,
	LEON_RID_TRCCTRL,
	LEON_RID_DSUBADDR1,
	LEON_RID_DSUBMASK1,
	LEON_RID_DSUBADDR2,
	LEON_RID_DSUBMASK2,
	LEON_RID_DSUTRAP,

	LEON_RID_SIZE_DSU
};

#define LEON_RID_ALLSIZE     (LEON_RID_SIZE_DSU)

// shadow names of registers
enum leon_shadow_register_indices {
	LEON_RID_SP = (LEON_RID_R0+SPARC_V8_REG_SP),
	LEON_RID_FP = (LEON_RID_R0+SPARC_V8_REG_FP),
	LEON_RID_BRADR0 = LEON_RID_ASR24,
	LEON_RID_BRMSK0 = LEON_RID_ASR25,
	LEON_RID_BRADR1 = LEON_RID_ASR26,
	LEON_RID_BRMSK1 = LEON_RID_ASR27,
	LEON_RID_BRADR2 = LEON_RID_ASR28,
	LEON_RID_BRMSK2 = LEON_RID_ASR29,
	LEON_RID_BRADR3 = LEON_RID_ASR30,
	LEON_RID_BRMSK3 = LEON_RID_ASR31,
};

enum leon_register_groups {
	LEON_RGRP_IU     = 0,
	LEON_RGRP_FPU,
	LEON_RGRP_CU,
	LEON_RGRP_PERI,
	LEON_RGRP_DSU,
	LEON_RGRP_SHADOW,
};

typedef struct leon_rgroups
{
	const char   *name;
	const char   *feature_name;
} leon_rgroups_t;

typedef struct leon_registers
{
	unsigned      rid;
	const char   *name;
	enum reg_type type;
	uint32_t      maddr;
} leon_registers_t;

#define LEON_MADDR_UNDEFINED    0xffffffff
#define LEON_MADDR_DYNAMIC      0xfffffffe

extern const leon_registers_t leon_iu_regs[];
extern const leon_registers_t leon_fpu_regs[];
//extern const leon_registers_t leon_cu_regs[];
extern const leon_registers_t leon_peri_regs[];
extern const leon_registers_t leon_dsu_regs[];
extern const leon_rgroups_t leon_rgrp_names[];
/* -------------------------------------------------------------------------- */
typedef struct leon_reg_item {
	char        level;
	const char *sname;
	const char *lname;
	uint32_t    mask;
	int         shift;
} leon_reg_item_t;

typedef struct leon_reg_parsed {
	unsigned               rid;
	const char            *name;
	const char            *longname;
	const leon_reg_item_t *items;
} leon_reg_parsed_t;

extern const leon_reg_parsed_t leon_register_description[];

/* -------------------------------------------------------------------------- */
struct reg_cache *leon_build_reg_cache(struct target *target);
int leon_check_regcache(struct target *target);

int leon_get_all_reg_list(struct target *target,
                          struct reg **reg_list[], int *reg_list_size);
int leon_get_general_reg_list(struct target *target,
                              struct reg **reg_list[], int *reg_list_size);

uint32_t *leon_get_ptrreg(struct target *tgt, unsigned rid);

int leon_read_register(struct target *tgt, unsigned rid, bool force);
int leon_read_all_registers(struct target *tgt, bool force);
int leon_write_register(struct target *tgt, unsigned rid, uint32_t value);

int leon_reg_name2rid(struct target *tgt, const char *group, const char *name);

#endif /* LEON_OOCD_TARGET_REGISTERS_HEADER_FILE */
