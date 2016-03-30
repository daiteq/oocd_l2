/**
 * OOCD target for Leon (Sparc v8) CPUs
 * (C)2016 Roman Bartosinski, daiteq,s.r.o.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "leon.h"

/*
 * GDB registers based on gdb-7.10.1/opcodes/sparc-dis.c
 * general registers are R0-R31
 */
const leon_registers_t leon_iu_regs[] = {
	{  LEON_RID_R0,  "r0", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{  LEON_RID_R1,  "r1", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{  LEON_RID_R2,  "r2", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{  LEON_RID_R3,  "r3", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{  LEON_RID_R4,  "r4", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{  LEON_RID_R5,  "r5", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{  LEON_RID_R6,  "r6", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{  LEON_RID_R7,  "r7", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{  LEON_RID_R8,  "r8", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{  LEON_RID_R9,  "r9", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R10, "r10", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R11, "r11", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R12, "r12", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R13, "r13", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R14, "r14", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R15, "r15", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R16, "r16", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R17, "r17", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R18, "r18", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R19, "r19", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R20, "r20", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R21, "r21", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R22, "r22", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R23, "r23", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R24, "r24", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R25, "r25", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R26, "r26", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R27, "r27", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R28, "r28", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R29, "r29", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R30, "r30", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},
	{ LEON_RID_R31, "r31", REG_TYPE_UINT32, LEON_MADDR_DYNAMIC},

	{ LEON_RID_Y,      "y", REG_TYPE_UINT32, LEON_DSU_SREG_Y},
	{ LEON_RID_PSR,  "psr", REG_TYPE_UINT32, LEON_DSU_SREG_PSR},
	{ LEON_RID_WIM,  "wim", REG_TYPE_UINT32, LEON_DSU_SREG_WIM},
	{ LEON_RID_TBR,  "tbr", REG_TYPE_UINT32, LEON_DSU_SREG_TBR},
	{ LEON_RID_PC,    "pc", REG_TYPE_CODE_PTR, LEON_DSU_SREG_PC},
	{ LEON_RID_NPC,  "npc", REG_TYPE_CODE_PTR, LEON_DSU_SREG_NPC},

	{ LEON_RID_ASR16, "asr16", REG_TYPE_UINT32, LEON_DSU_SREG_ASR(16)},
	{ LEON_RID_ASR17, "asr17", REG_TYPE_UINT32, LEON_DSU_SREG_ASR(17)},
	{ LEON_RID_ASR18, "asr18", REG_TYPE_UINT32, LEON_DSU_SREG_ASR(18)},
	{ LEON_RID_ASR19, "asr19", REG_TYPE_UINT32, LEON_DSU_SREG_ASR(19)},
	{ LEON_RID_ASR20, "asr20", REG_TYPE_UINT32, LEON_DSU_SREG_ASR(20)},
	{ LEON_RID_ASR21, "asr21", REG_TYPE_UINT32, LEON_DSU_SREG_ASR(21)},
	{ LEON_RID_ASR22, "asr22", REG_TYPE_UINT32, LEON_DSU_SREG_ASR(22)},
	{ LEON_RID_ASR23, "asr23", REG_TYPE_UINT32, LEON_DSU_SREG_ASR(23)},
	{ LEON_RID_ASR24, "asr24", REG_TYPE_UINT32, LEON_DSU_SREG_ASR(24)},
	{ LEON_RID_ASR25, "asr25", REG_TYPE_UINT32, LEON_DSU_SREG_ASR(25)},
	{ LEON_RID_ASR26, "asr26", REG_TYPE_UINT32, LEON_DSU_SREG_ASR(26)},
	{ LEON_RID_ASR27, "asr27", REG_TYPE_UINT32, LEON_DSU_SREG_ASR(27)},
	{ LEON_RID_ASR28, "asr28", REG_TYPE_UINT32, LEON_DSU_SREG_ASR(28)},
	{ LEON_RID_ASR29, "asr29", REG_TYPE_UINT32, LEON_DSU_SREG_ASR(29)},
	{ LEON_RID_ASR30, "asr30", REG_TYPE_UINT32, LEON_DSU_SREG_ASR(30)},
	{ LEON_RID_ASR31, "asr31", REG_TYPE_UINT32, LEON_DSU_SREG_ASR(31)},
	{ 0, NULL, 0, 0}
};

const leon_registers_t leon_fpu_regs[] = {
	{  LEON_RID_F0,  "f0", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{  LEON_RID_F1,  "f1", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{  LEON_RID_F2,  "f2", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{  LEON_RID_F3,  "f3", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{  LEON_RID_F4,  "f4", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{  LEON_RID_F5,  "f5", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{  LEON_RID_F6,  "f6", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{  LEON_RID_F7,  "f7", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{  LEON_RID_F8,  "f8", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{  LEON_RID_F9,  "f9", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F10, "f10", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F11, "f11", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F12, "f12", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F13, "f13", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F14, "f14", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F15, "f15", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F16, "f16", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F17, "f17", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F18, "f18", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F19, "f19", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F20, "f20", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F21, "f21", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F22, "f22", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F23, "f23", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F24, "f24", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F25, "f25", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F26, "f26", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F27, "f27", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F28, "f28", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F29, "f29", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F30, "f30", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},
	{ LEON_RID_F31, "f31", REG_TYPE_FLOAT, LEON_MADDR_DYNAMIC},

	{ LEON_RID_FSR,  "fsr", REG_TYPE_UINT32, LEON_DSU_SREG_FSR},
	{ 0, NULL, 0, 0}
};

//const leon_registers_t leon_cu_regs[] = {
//	{  LEON_RID_C0,  "c0", REG_TYPE_UINT32, xxx},
//	{ 0, NULL, 0, 0}
//};

const leon_registers_t leon_peri_regs[] = {
	{ LEON_RID_LCFG,  "lcfg", REG_TYPE_UINT32, LEON_REGS_LEON_CFG_REG},
	{ LEON_RID_CCR,    "ccr", REG_TYPE_UINT32, LEON_REGS_CACHE_CTRL_REG},
	{ 0, NULL, 0, 0}
};

const leon_registers_t leon_dsu_regs[] = {
	{   LEON_RID_DSUCTRL,   "dsuctrl", REG_TYPE_UINT32, LEON_DSU_CTRL_REG},
	{   LEON_RID_TRCCTRL,   "trcctrl", REG_TYPE_UINT32, LEON_DSU_TRACE_CTRL_REG},
	{ LEON_RID_DSUBADDR1, "dsubaddr1", REG_TYPE_UINT32, LEON_DSU_AHB_BRK_ADDR1},
	{ LEON_RID_DSUBMASK1, "dsubmask1", REG_TYPE_UINT32, LEON_DSU_AHB_BRK_MASK1},
	{ LEON_RID_DSUBADDR2, "dsubaddr2", REG_TYPE_UINT32, LEON_DSU_AHB_BRK_ADDR2},
	{ LEON_RID_DSUBMASK2, "dsubmask2", REG_TYPE_UINT32, LEON_DSU_AHB_BRK_MASK2},
	{   LEON_RID_DSUTRAP,   "dsutrap", REG_TYPE_UINT32, LEON_DSU_SREG_DSU_TRAP},
	{ 0, NULL, 0, 0}
};

/* name of each group has to have different first character */
const leon_rgroups_t leon_rgrp_names[] = {
	{     "iu",     "net.sourceforge.openocd.iu"},
	{    "fpu",    "net.sourceforge.openocd.fpu"},
	{     "cu",     "net.sourceforge.openocd.cu"},
	{   "peri",   "net.sourceforge.openocd.peri"},
	{    "dsu",    "net.sourceforge.openocd.dsu"},
	{ "shadow", "net.sourceforge.openocd.shadow"},
};


/* -------------------------------------------------------------------------- */
/* Register description */
const leon_reg_item_t leon_regdes_psr[] = {
	{ 1, "IMPL",         "Implementation", SPARC_V8_PSR_IMPL_MASK, SPARC_V8_PSR_IMPL_SHIFT},
	{ 1, "IVER", "Implementation version",  SPARC_V8_PSR_VER_MASK, SPARC_V8_PSR_VER_SHIFT},
	{ 1,   "EF",  "Enable Floating-Point",        SPARC_V8_PSR_EF, SPARC_V8_PSR_EF_SHIFT},
	{ 1,   "EC",     "Enable Coprocessor",        SPARC_V8_PSR_EC, SPARC_V8_PSR_EC_SHIFT},
	{ 1,   "ET",           "Enable Traps",        SPARC_V8_PSR_ET, SPARC_V8_PSR_ET_SHIFT},
	{ 1,  "PIL",   "Proc.Interrupt Level",  SPARC_V8_PSR_PIL_MASK, SPARC_V8_PSR_PIL_SHIFT},
	{ 1,    "S",             "Supervisor",         SPARC_V8_PSR_S, SPARC_V8_PSR_S_SHIFT},
	{ 1,   "PS",    "Previous Supervisor",        SPARC_V8_PSR_PS, SPARC_V8_PSR_PS_SHIFT},
	{ 1,  "CWP", "Current Window Pointer",  SPARC_V8_PSR_CWP_MASK, SPARC_V8_PSR_CWP_SHIFT},
	{ 1,  "ICC",    "Integer cond. codes",     SPARC_V8_PSR_ICC_C | SPARC_V8_PSR_ICC_V | SPARC_V8_PSR_ICC_Z | SPARC_V8_PSR_ICC_N, SPARC_V8_PSR_ICC_C_SHIFT},
	{ 2,    "C",              "icc Carry",     SPARC_V8_PSR_ICC_C, SPARC_V8_PSR_ICC_C_SHIFT},
	{ 2,    "V",           "icc Overflow",     SPARC_V8_PSR_ICC_V, SPARC_V8_PSR_ICC_V_SHIFT},
	{ 2,    "Z",               "icc Zero",     SPARC_V8_PSR_ICC_Z, SPARC_V8_PSR_ICC_Z_SHIFT},
	{ 2,    "N",           "icc Negative",     SPARC_V8_PSR_ICC_N, SPARC_V8_PSR_ICC_N_SHIFT},
	{ 0, NULL, NULL, 0, 0}
};

const leon_reg_item_t leon_regdes_tbr[] = {
	{ 1,  "TT", "Trap Type", SPARC_V8_TBR_TT_MASK, SPARC_V8_TBR_TT_SHIFT},
	{ 1, "TBA", "Trap Base Address", SPARC_V8_TBR_TBA_MASK, SPARC_V8_TBR_TBA_SHIFT},
	{ 0, NULL, NULL, 0, 0}
};

const leon_reg_item_t leon_regdes_dsuctrl[] = {
	{ 1,   "TE",            "Trace enable",   LEON_DSU_CTRL_TRACE_EN, LEON_DSU_CTRL_TRACE_EN_SHIFT},
	{ 1,  "DCM",      "Delay counter mode",    LEON_DSU_CTRL_DLY_CNT, LEON_DSU_CTRL_DLY_CNT_SHIFT},
	{ 1,   "BT",          "Break on trace",    LEON_DSU_CTRL_BRK_TRC, LEON_DSU_CTRL_BRK_TRC_SHIFT},
	{ 1,   "FT",           "Freeze timers", LEON_DSU_CTRL_FREEZE_TMR, LEON_DSU_CTRL_FREEZE_TMR_SHIFT},
	{ 1,   "BE",          "Break on error",    LEON_DSU_CTRL_BRK_ERR, LEON_DSU_CTRL_BRK_ERR_SHIFT},
	{ 1,   "BW",  "Break on IU watchpoint", LEON_DSU_CTRL_BRK_IU_WPT, LEON_DSU_CTRL_BRK_IU_WPT_SHIFT},
	{ 1,   "BS", "Break on S/W breakpoint", LEON_DSU_CTRL_BRK_SW_BRK, LEON_DSU_CTRL_BRK_SW_BRK_SHIFT},
	{ 1,   "BN",               "Break now",    LEON_DSU_CTRL_BRK_NOW, LEON_DSU_CTRL_BRK_NOW_SHIFT},
	{ 1,   "BD", "Break on DSU breakpoint", LEON_DSU_CTRL_BRK_DSU_BRK, LEON_DSU_CTRL_BRK_DSU_BRK_SHIFT},
	{ 1,   "BX",           "Break on trap",    LEON_DSU_CTRL_BRK_TRAP, LEON_DSU_CTRL_BRK_TRAP_SHIFT},
	{ 1,   "BZ",    "Break on error traps", LEON_DSU_CTRL_BRK_ETRAP, LEON_DSU_CTRL_BRK_ETRAP_SHIFT},
	{ 1,   "DE",    "Delay counter enable",  LEON_DSU_CTRL_DLY_CNT_EN, LEON_DSU_CTRL_DLY_CNT_EN_SHIFT},
	{ 1,   "DM",              "Debug mode", LEON_DSU_CTRL_DEBUG_MODE, LEON_DSU_CTRL_DEBUG_MODE_SHIFT},
	{ 1,   "EB",  "external DSUBRE signal", LEON_DSU_CTRL_DSUBRE, LEON_DSU_CTRL_DSUBRE_SHIFT},
	{ 1,   "EE",   "external DSUEN signal", LEON_DSU_CTRL_DSUEN, LEON_DSU_CTRL_DSUEN_SHIFT},
	{ 1,   "PE",    "Processor error mode", LEON_DSU_CTRL_CPU_ERR, LEON_DSU_CTRL_CPU_ERR_SHIFT},
	{ 1,   "SS",             "Single step", LEON_DSU_CTRL_SINGLE_STEP, LEON_DSU_CTRL_SINGLE_STEP_SHIFT},
	{ 1,   "LR",           "Link response", LEON_DSU_CTRL_LINK_RESP, LEON_DSU_CTRL_LINK_RESP_SHIFT},
	{ 1,   "DR",     "Debug mode response", LEON_DSU_CTRL_DEBUG_RESP, LEON_DSU_CTRL_DEBUG_RESP_SHIFT},
	{ 1,   "RE",        "Reset error mode", LEON_DSU_CTRL_RESET_CPU_ERR, LEON_DSU_CTRL_RESET_CPU_ERR_SHIFT},
	{ 1, "DCNT", "Trace buffer delay counter", LEON_DSU_CTRL_TRCDLYCNT_MASK, LEON_DSU_CTRL_TRCDLYCNT_SHIFT},
	{ 0, NULL, NULL, 0, 0}
};

const leon_reg_item_t leon_regdes_trcctrl[] = {
	{ 1,   "II", "Instruction trace index", LEON_DSU_TRCTRL_INST_IDX_MASK, LEON_DSU_TRCTRL_INST_IDX_SHIFT},
	{ 1,   "AI",         "AHB trace index",  LEON_DSU_TRCTRL_AHB_IDX_MASK, LEON_DSU_TRCTRL_AHB_IDX_SHIFT},
	{ 1,   "TI",     "Trace instr. enable",            LEON_DSU_TRCTRL_TI, LEON_DSU_TRCTRL_TI_SHIFT},
	{ 1,   "TA",        "Trace AHB enable",            LEON_DSU_TRCTRL_TA, LEON_DSU_TRCTRL_TA_SHIFT},
	{ 1,   "AF",        "AHB trace freeze",            LEON_DSU_TRCTRL_AF, LEON_DSU_TRCTRL_AF_SHIFT},
	{ 1, "SFLT",         "Slave filtering",    LEON_DSU_TRCTRL_SFILT_MASK, LEON_DSU_TRCTRL_SFILT_SHIFT},
	{ 1, "MFLT",        "Master filtering",    LEON_DSU_TRCTRL_MFILT_MASK, LEON_DSU_TRCTRL_MFILT_SHIFT},
	{ 0, NULL, NULL, 0, 0}
};

const leon_reg_item_t leon_regdes_dsubreakaddr[] = {
	{ 1,   "EX",     "Break on instruction",         LEON_DSU_BRKADDR_EX, LEON_DSU_BRKADDR_EX_SHIFT},
	{ 1, "BADR", "Breakpoint Address(31:2)", LEON_DSU_BRKADDR_BADDR_MASK, LEON_DSU_BRKADDR_BADDR_SHIFT},
	{ 0, NULL, NULL, 0, 0}
};

const leon_reg_item_t leon_regdes_dsubreakmask[] = {
	{ 1,   "ST", "Break on data store address",         LEON_DSU_BRKMASK_ST, LEON_DSU_BRKMASK_ST_SHIFT},
	{ 1,   "LD",  "Break on data load address",         LEON_DSU_BRKMASK_LD, LEON_DSU_BRKMASK_LD_SHIFT},
	{ 1, "BMSK",       "Breakpoint Mask(31:2)", LEON_DSU_BRKMASK_BMASK_MASK, LEON_DSU_BRKMASK_BMASK_SHIFT},
	{ 0, NULL, NULL, 0, 0}
};

const leon_reg_item_t leon_regdes_dsutrap[] = {
	{ 1, "TT", "8-bit SPARC trap type",         LEON_DSU_TRAP_TT_MASK, LEON_DSU_TRAP_TT_SHIFT},
	{ 1, "EM", "Error Mode",         LEON_DSU_TRAP_EM, LEON_DSU_TRAP_EM_SHIFT},
	{ 0, NULL, NULL, 0, 0}
};

const leon_reg_parsed_t leon_register_description[] = {
	{       LEON_RID_PSR,      "PSR", "Processor State Register", leon_regdes_psr},
	{       LEON_RID_TBR,      "TBR", "Trap Base Register", leon_regdes_tbr},
	{   LEON_RID_DSUCTRL,  "DSUCTRL", "DSU Control Register", leon_regdes_dsuctrl},
	{   LEON_RID_TRCCTRL,  "TRCCTRL", "Trace Buffer Control Register", leon_regdes_trcctrl},
	{ LEON_RID_DSUBADDR1, "BRKADDR1", "DSU Breakpoint Address 1", leon_regdes_dsubreakaddr},
	{ LEON_RID_DSUBMASK1, "BRKMASK1", "DSU Breakpoint Mask 1", leon_regdes_dsubreakmask},
	{ LEON_RID_DSUBADDR2, "BRKADDR2", "DSU Breakpoint Address 2", leon_regdes_dsubreakaddr},
	{ LEON_RID_DSUBMASK2, "BRKMASK2", "DSU Breakpoint Mask 2", leon_regdes_dsubreakmask},
	{   LEON_RID_DSUTRAP,  "DSUTRAP", "DSU Trap Register", leon_regdes_dsutrap},
	{0, NULL, NULL, NULL}
};

