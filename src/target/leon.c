/**
 * OOCD target for Leon (Sparc v8) CPUs
 * (C)2016 Roman Bartosinski, daiteq,s.r.o.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "leon.h"

/* -------------------------------------------------------------------------- */
//static void leon_deinit_target(struct target *target);

//static int leon_poll(struct target *target);
//static int leon_arch_state(struct target *target);

//static int leon_step(struct target *target, int current,
//                     uint32_t address, int handle_breakpoints);

//static int leon_assert_reset(struct target *target);
//static int leon_deassert_reset(struct target *target);


//static int leon_checksum_memory(struct target *t, uint32_t a, uint32_t s,
//                                uint32_t *checksum);
//static int leon_blank_check_memory(struct target *target, uint32_t address,
//                                   uint32_t count, uint32_t *blank);

//static int leon_run_algorithm(struct target *target, int num_mem_params,
//                        struct mem_param *mem_params, int num_reg_params,
//                        struct reg_param *reg_param, uint32_t entry_point,
//                        uint32_t exit_point, int timeout_ms, void *arch_info);

//static int leon_add_breakpoint(struct target *target,
//                               struct breakpoint *breakpoint);
//static int leon_remove_breakpoint(struct target *target,
//                                  struct breakpoint *breakpoint);
//static int leon_add_watchpoint(struct target *target,
//                               struct watchpoint *watchpoint);
//static int leon_remove_watchpoint(struct target *target,
//                                  struct watchpoint *watchpoint);
//// static int leon_hit_watchpoint(struct target *target, struct watchpoint **hit_watchpoint);

//int leon_reset_callback(struct target *target, enum target_reset_mode reset_mode, void *priv);

/* -------------------------------------------------------------------------- */
/* --- Auxiliary functions -------------------------------------------------- */
static char *leon_type_to_string(enum leon_type ltp)
{
	switch (ltp) {
		case LEON_TYPE_L2:   return "LEON2";
		case LEON_TYPE_L2FT: return "LEON2-FT";
		case LEON_TYPE_L2MT: return "LEON2-MT";
		default:             return "unknown";
	}
}

static char *leon_trctype_to_string(enum leon_trace_type lttp)
{
	switch (lttp) {
		case LEON_TRCTYPE_NONE: return "none";
		case LEON_TRCTYPE_CPU:  return "instructions";
		case LEON_TRCTYPE_AHB:  return "AHB transactions";
		case LEON_TRCTYPE_BOTH: return "mixed (inst and AHB)";
		default:                return "unknown";
	}
}

static void print_hwinfo(struct command_context *cmd_ctx, struct target *tgt)
{
	struct leon_common *leon = target_to_leon(tgt);
	uint32_t *plcfg = leon_get_ptrreg(tgt, LEON_RID_LCFG);
	uint32_t *ppsr = leon_get_ptrreg(tgt, LEON_RID_PSR);
	uint32_t *pfsr = leon_get_ptrreg(tgt, LEON_RID_FSR);
	uint32_t *piu1, *piu2;

	command_print(cmd_ctx, "Leon type : %s", leon_type_to_string(leon->ltype));
	if (leon->ltype==LEON_TYPE_L2MT && leon->mt_ctlblk_size>0)
		command_print(cmd_ctx, "     Size of MT block = %d bytes", leon->mt_ctlblk_size);
	if (plcfg) {
		command_print(cmd_ctx, "--- Leon Configuration Register (0x%08X) ---", *plcfg);
		command_print(cmd_ctx, " - Additional Instructions: %s%s%s",
			LEON_GET_BIT(plcfg, LEON_CFG_REG_INST_MUL) ? " <SMUL/UMUL>" : "",
			LEON_GET_BIT(plcfg, LEON_CFG_REG_INST_DIV) ? " <UDIV/SDIV>" : "",
			LEON_GET_BIT(plcfg, LEON_CFG_REG_INST_MAC) ? " <UMAC/SMAC>" : "");
		command_print(cmd_ctx, " - Additional Structures: %s%s%s%s%s%s%s",
			LEON_GET_BIT(plcfg, LEON_CFG_REG_IS_DSU) ? " <DSU>" : "",
			LEON_GET_BIT(plcfg, LEON_CFG_REG_IS_SDRAM) ? " <SDRAMctrl>" : "",
			LEON_GET_BIT(plcfg, LEON_CFG_REG_IS_WDOG) ? " <WATCHDOG>" : "",
			LEON_GET_BIT(plcfg, LEON_CFG_REG_IS_MEMSTAT) ? " <MEMORYstatus>" : "",
			LEON_GET_BIT(plcfg, LEON_CFG_REG_WRPR_TYPE_MASK) ? " <WriteProtection>" : "",
			LEON_GET_BIT(plcfg, LEON_CFG_REG_PCI_TYPE_MASK) ? " <PCIcore>" : "",
			LEON_GET_BIT(plcfg, LEON_CFG_REG_FPU_TYPE_MASK) ? " <FPU>" : "");
		command_print(cmd_ctx, " - Other Configurations:");
		command_print(cmd_ctx, "   - Number of watchpoints = %lu", LEON_GET_VAL(plcfg, LEON_CFG_REG_NUM_WPOIS));
		command_print(cmd_ctx, "   - Number of register windows = %lu", LEON_GET_VAL(plcfg, LEON_CFG_REG_NWINDOWS)+1);
		command_print(cmd_ctx, "   - ICache set size = %lu kB, line size = %lu bits",
		        1L<<LEON_GET_VAL(plcfg, LEON_CFG_REG_ICSET_SZ),
		        (1L<<LEON_GET_VAL(plcfg, LEON_CFG_REG_ICLINE_SZ))*32);
		command_print(cmd_ctx, "   - DCache set size = %lu kB, line size = %lu bits",
		        1L<<LEON_GET_VAL(plcfg, LEON_CFG_REG_DCSET_SZ),
		        (1L<<LEON_GET_VAL(plcfg, LEON_CFG_REG_DCLINE_SZ))*32);
	}
	if (ppsr) {
		command_print(cmd_ctx, "--- Sparc v8 PSR (0x%08X) ---", *ppsr);
		command_print(cmd_ctx, " - Implementation ID = %u, Version = %u",
		        LEON_GET_VAL(ppsr, SPARC_V8_PSR_IMPL), LEON_GET_VAL(ppsr, SPARC_V8_PSR_VER));
		command_print(cmd_ctx, " - CWP = %u", LEON_GET_VAL(ppsr, SPARC_V8_PSR_CWP));
		if (LEON_GET_BIT(ppsr, SPARC_V8_PSR_EF))
			command_print(cmd_ctx, " - Floating Point Unit enabled (type = %lu)",
			              (plcfg) ? LEON_GET_VAL(plcfg, LEON_CFG_REG_FPU_TYPE) : 0);
		if (LEON_GET_BIT(ppsr, SPARC_V8_PSR_EC))
			command_print(cmd_ctx, " - Coprocessor Unit enabled");
	}
	if (pfsr) {
		command_print(cmd_ctx, "--- Floating Point Unit FSR (0x%08X) ---", *pfsr);
		command_print(cmd_ctx, " - FPU Version = %u", LEON_GET_VAL(pfsr, SPARC_V8_FSR_VER));
	}
	command_print(cmd_ctx, "--- Sparc v8 IU registers ---");
	piu1 = leon_get_ptrreg(tgt, LEON_RID_PC);	piu2= leon_get_ptrreg(tgt, LEON_RID_NPC);
	command_print(cmd_ctx, " - PC = 0x%08X , nPC = 0x%08X", *piu1, *piu2);
	piu1 = leon_get_ptrreg(tgt, LEON_RID_SP);	piu2= leon_get_ptrreg(tgt, LEON_RID_FP);
	command_print(cmd_ctx, " - SP = 0x%08X , FP = 0x%08X", *piu1, *piu2);
	piu1 = leon_get_ptrreg(tgt, LEON_RID_TBR);	piu2= leon_get_ptrreg(tgt, LEON_RID_Y);
	command_print(cmd_ctx, " - TBR = 0x%08X , Y = 0x%08X", *piu1, *piu2);

	uint32_t *pdsu = leon_get_ptrreg(tgt, LEON_RID_DSUCTRL);
	if (pdsu) {
		command_print(cmd_ctx, "--- DSU Control Register (0x%08X)---", *pdsu);
		command_print(cmd_ctx, " - Break on :%s%s%s%s%s%s%s",
		              LEON_GET_BIT(pdsu, LEON_DSU_CTRL_BRK_TRC) ? " trace" : "",
		              LEON_GET_BIT(pdsu, LEON_DSU_CTRL_BRK_ERR) ? " error" : "",
		              LEON_GET_BIT(pdsu, LEON_DSU_CTRL_BRK_IU_WPT) ? " IU_watchpoint" : "",
		              LEON_GET_BIT(pdsu, LEON_DSU_CTRL_BRK_SW_BRK) ? " S/W_breakpoint" : "",
		              LEON_GET_BIT(pdsu, LEON_DSU_CTRL_BRK_DSU_BRK) ? " DSU_breakpoint" : "",
		              LEON_GET_BIT(pdsu, LEON_DSU_CTRL_BRK_TRAP) ? " trap" : "",
		              LEON_GET_BIT(pdsu, LEON_DSU_CTRL_BRK_ETRAP) ? " error_traps" : "");
	}
	pdsu = leon_get_ptrreg(tgt, LEON_RID_DSUTRAP);
	if (pdsu)
		command_print(cmd_ctx, "--- DSU Trap Register (0x%08X)---", *pdsu);
}

void leon_check_state_and_reason(struct target *tgt, enum target_state *pst, enum target_debug_reason *pdr)
{
	uint32_t tt;
	enum target_state st = TARGET_UNKNOWN;
	enum target_debug_reason dr = DBG_REASON_UNDEFINED;
	uint32_t *pdsuctrl = leon_get_ptrreg(tgt, LEON_RID_DSUCTRL);
	uint32_t *pdsutrp = leon_get_ptrreg(tgt, LEON_RID_DSUTRAP);
	do {
		if (!pdsuctrl) break;

		struct leon_common *leon = target_to_leon(tgt);
		if (leon->ltype==LEON_TYPE_L2MT) {
			st = TARGET_HALTED;
			dr = DBG_REASON_UNDEFINED;
		} else {
			if (LEON_GET_BIT(pdsuctrl, LEON_DSU_CTRL_DEBUG_MODE)) {
				st = TARGET_HALTED;
			} else {
				st = TARGET_RUNNING;
				dr = DBG_REASON_NOTHALTED;
				break;
			}
		}

		if (!pdsutrp) break;
		tt = LEON_GET_VAL(pdsutrp, LEON_DSU_TRAP_TT);
		if (tt == SPARC_V8_TT_WATCHPOINT) {
			if (LEON_GET_BIT(pdsuctrl, LEON_DSU_CTRL_DSUBRE | LEON_DSU_CTRL_BRK_NOW))
				dr = DBG_REASON_DBGRQ;
			else
				dr = DBG_REASON_WATCHPOINT;
			break;
		} else if (tt>=128) {
			dr = DBG_REASON_BREAKPOINT;
			break;
		} else {
			dr = DBG_REASON_DBGRQ;
			break;
		}

	//	DBG_REASON_WPTANDBKPT = 3,
	//	DBG_REASON_SINGLESTEP = 4,
	//	DBG_REASON_EXIT = 6,
	} while(0);

	if (tgt->state==TARGET_HALTED) {
		struct leon_common *leon = target_to_leon(tgt);
		// update all registers only if leon is not L2MT
		if (leon->ltype!=LEON_TYPE_UNKNOWN && leon->ltype!=LEON_TYPE_L2MT)
			leon_read_all_registers(tgt, 0);
	}

	if (pst) *pst = st;
	if (pdr) *pdr = dr;
}

void leon_update_crc16(uint16_t *crcval, uint16_t data)
{
	uint8_t s, t; // d = in_data
	uint16_t c, r; // c = crc before update, r = crc after update
	c = *crcval;
	s = (uint8_t)(data>>8) ^ (c >> 8);
	t = s ^ (s >> 4);
	r = (c << 8) ^ t ^ (t << 5) ^ (t << 12);

	s = (uint8_t)(data&0xff) ^ (r >> 8);
	t = s ^ (s >> 4);
	c = (r << 8) ^ t ^ (t << 5) ^ (t << 12);

	*crcval = c;
}

void leon_update_tmode(struct leon_common *leon, uint32_t dsu, uint32_t trc)
{
	if (dsu & LEON_DSU_CTRL_TRACE_EN) {
		switch (trc & (LEON_DSU_TRCTRL_TI | LEON_DSU_TRCTRL_TA)) {
			case LEON_DSU_TRCTRL_TI:
				leon->trctype = LEON_TRCTYPE_CPU;
				break;
			case LEON_DSU_TRCTRL_TA:
				leon->trctype = LEON_TRCTYPE_AHB;
				break;
			case LEON_DSU_TRCTRL_TI | LEON_DSU_TRCTRL_TA:
				leon->trctype = LEON_TRCTYPE_BOTH;
				break;
			default:
				leon->trctype = LEON_TRCTYPE_NONE;
				break;
		}
	} else
		leon->trctype = LEON_TRCTYPE_NONE;
}

struct target *leon_cmd_to_tgt(struct command_invocation *cmd)
{
	struct target *tgt = get_current_target(CMD_CTX);
	if (tgt==NULL) return NULL;
	if (strcmp(tgt->type->name, LEON_TYPE_NAME)==0) return tgt;
	// try find target from command
	struct command *pcmd = CMD_CURRENT;
	while (pcmd->parent) pcmd = pcmd->parent;
	tgt = get_target(pcmd->name);
	if (tgt==NULL) return NULL;
	if (strcmp(tgt->type->name, LEON_TYPE_NAME)==0) return tgt;
	return NULL;
}



/* TRACE BUFFER */
static uint32_t tbuf_to_time(enum leon_trace_type tt, uint32_t *ptb)
{
	if (tt==LEON_TRCTYPE_AHB)
		return (*(ptb+0) & LEON_DSU_TRCD_AHB_TIMETAG_MASK)>>LEON_DSU_TRCD_AHB_TIMETAG_SHIFT;
	else if (tt==LEON_TRCTYPE_CPU)
		return (*(ptb+0) & LEON_DSU_TRCD_INST_TIMETAG_MASK)>>LEON_DSU_TRCD_INST_TIMETAG_SHIFT;
	return 0xffffffff;
}
static inline uint32_t tbuf_to_inst_pc(uint32_t *ptb)
{
	return (*(ptb+2) & LEON_DSU_TRCB_INST_PC_MASK)>>LEON_DSU_TRCB_INST_PC_SHIFT;
}
static inline uint32_t tbuf_to_inst_opcode(uint32_t *ptb)
{
	return (*(ptb+3) & LEON_DSU_TRCA_INST_OPCODE_MASK)>>LEON_DSU_TRCA_INST_OPCODE_SHIFT;
}
static inline uint32_t tbuf_to_inst_param(uint32_t *ptb)
{
	return (*(ptb+1) & LEON_DSU_TRCC_INST_LDSTPAR_MASK)>>LEON_DSU_TRCC_INST_LDSTPAR_SHIFT;
}

static inline uint32_t tbuf_to_ahb_addr(uint32_t *ptb)
{
	return (*(ptb+3) & LEON_DSU_TRCA_AHB_ADDR_MASK)>>LEON_DSU_TRCA_AHB_ADDR_SHIFT;
}
static inline const char *tbuf_to_ahb_type(uint32_t *ptb)
{
	if (*(ptb+1) & LEON_DSU_TRCC_AHB_HWRITE) return "write";
	else return "read ";
}
static inline uint32_t tbuf_to_ahb_data(uint32_t *ptb)
{
	return (*(ptb+2) & LEON_DSU_TRCB_AHB_DATA_MASK)>>LEON_DSU_TRCB_AHB_DATA_SHIFT;
}
static inline int tbuf_to_ahb_trans(uint32_t *ptb)
{
	return (int)((*(ptb+1) & LEON_DSU_TRCC_AHB_HTRANS_MASK)>>LEON_DSU_TRCC_AHB_HTRANS_SHIFT);
}
static inline int tbuf_to_ahb_size(uint32_t *ptb)
{
	return (int)((*(ptb+1) & LEON_DSU_TRCC_AHB_HSIZE_MASK)>>LEON_DSU_TRCC_AHB_HSIZE_SHIFT);
}
static inline int tbuf_to_ahb_burst(uint32_t *ptb)
{
	return (int)((*(ptb+1) & LEON_DSU_TRCC_AHB_HBURST_MASK)>>LEON_DSU_TRCC_AHB_HBURST_SHIFT);
}
static inline int tbuf_to_ahb_mst(uint32_t *ptb)
{
	return (int)((*(ptb+1) & LEON_DSU_TRCC_AHB_HMASTER_MASK)>>LEON_DSU_TRCC_AHB_HMASTER_SHIFT);
}
static inline int tbuf_to_ahb_lock(uint32_t *ptb)
{
	return (int)((*(ptb+1) & LEON_DSU_TRCC_AHB_HMASTLOCK)>>LEON_DSU_TRCC_AHB_HMASTLOCK_SHIFT);
}
static inline int tbuf_to_ahb_resp(uint32_t *ptb)
{
	return (int)((*(ptb+1) & LEON_DSU_TRCC_AHB_HRESP_MASK)>>LEON_DSU_TRCC_AHB_HRESP_SHIFT);
}
static inline int tbuf_to_ahb_tt(uint32_t *ptb)
{
	return (int)((*(ptb+1) & LEON_DSU_TRCC_AHB_TT_MASK)>>LEON_DSU_TRCC_AHB_TT_SHIFT);
}
static inline int tbuf_to_ahb_pil(uint32_t *ptb)
{
	return (int)((*(ptb+1) & LEON_DSU_TRCC_AHB_PIL_MASK)>>LEON_DSU_TRCC_AHB_PIL_SHIFT);
}
static inline int tbuf_to_ahb_irl(uint32_t *ptb)
{
	return (int)((*(ptb+1) & LEON_DSU_TRCC_AHB_IRL_MASK)>>LEON_DSU_TRCC_AHB_IRL_SHIFT);
}

static void print_trace(struct command_context *cmd_ctx, struct target *ptgt,
                        enum leon_trace_type tt, uint32_t *tbuf) /* tbuf = [ 127-96, 95-64, 63-32, 31-0 ] */
{
	if (tt==LEON_TRCTYPE_CPU) {
		if (tbuf==NULL) {
			command_print(cmd_ctx, "  time     address     instruction      result");
		} else {
			int dasmt = 0;
			const char *das = leon_disas(ptgt, tbuf_to_inst_pc(tbuf), leon_swap_u32(tbuf_to_inst_opcode(tbuf)), &dasmt);
			command_print(cmd_ctx, " %10u  %08X %s %-30s  [%08X]",
			              tbuf_to_time(tt, tbuf), tbuf_to_inst_pc(tbuf), (dasmt) ? "S" : " ",
			              das, tbuf_to_inst_param(tbuf));
		}
	} else if (tt==LEON_TRCTYPE_AHB) {
		if (tbuf==NULL) {
			command_print(cmd_ctx, "  time     address     type      data  trans size burst mst lock resp tt pil irl");
		} else {
			command_print(cmd_ctx, " %10u  %08X  %s  %08X   %d    %d    %d    %d    %d    %d   %02d   %d   %d",
			              tbuf_to_time(tt, tbuf), tbuf_to_ahb_addr(tbuf),
			              tbuf_to_ahb_type(tbuf), tbuf_to_ahb_data(tbuf),
			              tbuf_to_ahb_trans(tbuf), tbuf_to_ahb_size(tbuf),
			              tbuf_to_ahb_burst(tbuf), tbuf_to_ahb_mst(tbuf),
			              tbuf_to_ahb_lock(tbuf), tbuf_to_ahb_resp(tbuf),
			              tbuf_to_ahb_tt(tbuf), tbuf_to_ahb_pil(tbuf),
			              tbuf_to_ahb_irl(tbuf));
		}
	} else { /* other type prints trace header */
		command_print(cmd_ctx, "-- unsupported trace type");
	}
}

/* ========================================================================== */

static int leon_target_create(struct target *target, Jim_Interp *interp)
{
	struct leon_common *pl = calloc(1, sizeof(struct leon_common));
	if (pl==NULL) return ERROR_TARGET_INIT_FAILED;

	pl->tap = target->tap;
	pl->loptime = 0;
	pl->ltype = LEON_TYPE_UNKNOWN;
	pl->mt_ctlblk_size = -1;
	pl->trctype = LEON_TRCTYPE_UNKNOWN;
	pl->lelf_epoint = 0xffffffff;
	target->arch_info = pl;

	return ERROR_OK;
}

static int leon_init_target(struct command_context *cmd_ctx,
		struct target *target)
{
	struct leon_common *pl = (struct leon_common *)target->arch_info;
	pl->rid_to_rdi = calloc(LEON_RID_ALLSIZE, sizeof(int));
	pl->rdi_to_rid = calloc(LEON_RID_ALLSIZE, sizeof(int));
	if (pl->rid_to_rdi==NULL || pl->rdi_to_rid==NULL)
		return ERROR_TARGET_INIT_FAILED;

//	target_register_reset_callback(leon_reset_callback, NULL);

	leon_build_reg_cache(target);
	return ERROR_OK;
}

static void leon_deinit_target(struct target *target)
{
	struct leon_common *pl = (struct leon_common *)target->arch_info;
//	target_unregister_reset_callback(leon_reset_callback, NULL);
	leon_elf_reset_sections_symbols(pl);
	free(pl->rid_to_rdi);
	free(pl->rdi_to_rid);
}


static int leon_examine(struct target *target)
{
	int retval = ERROR_OK, rc;
	struct leon_common *leon = target_to_leon(target);
#define LEON_EXAMINE_WORDS 2
	//uint32_t dsnd[2] = {LEON_PID_CONTROL | LEON_CTRL_RAT_DSU, LEON_PID_PAD | 0x81};
	uint32_t dsnd[LEON_EXAMINE_WORDS] = {LEON_PID_CONTROL | LEON_CTRL_RAT_DSU, LEON_PID_CONTROL | LEON_CTRL_RAT_DSU};
	uint32_t drcv[LEON_EXAMINE_WORDS];

	LOG_DEBUG("LEONexam (%s)", __func__);

	LEON_TM_START(bench);

	/* Reestablish communication after target reset */
//{
//	struct scan_field field;
//	uint8_t inval[4], outval[4];

//	rc = leon_jtag_set_instr(leon->tap, LEON_IRINS_IDCODE, 1);

//	field.num_bits = 32;		// USER1 register
//	field.out_value = outval;
//	field.in_value = inval;
//	buf_set_u32(outval, 0, 32, 0x12345678);
//	jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
//	jtag_execute_queue();
//	buf_set_u32(outval, 0, 32, 0xaa66553c);
//	jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
//	jtag_execute_queue();
////	return ERROR_FAIL;
//}

	/* forced enter to BS USER1 register */
	rc = leon_jtag_set_instr(leon->tap, LEON_IRINS_ENTER, 1);
	/* reset internal fsm */
	retval = ERROR_TARGET_NOT_EXAMINED;
	do {
		if (rc!=ERROR_OK) {
			LOG_ERROR("set IR to UCD failed (%d)", rc);
			break;
		} 
		//rc = leon_jtag_exchng_data(leon->tap, dsnd, LEON_EXAMINE_WORDS, &drcv, 1, 1); // reset L2 DSU
		rc = leon_jtag_exchng_data(leon->tap, dsnd, LEON_EXAMINE_WORDS, drcv, LEON_EXAMINE_WORDS, 0); // reset L2 DSU
		if (rc!=ERROR_OK) {
			LOG_ERROR("LEONexam (%s): exchange data failed (0x%04X)", leon->tap->dotted_name, rc);
			break;
		}
		if (drcv[1]!=LEON_STAT_TESTWORD) {
			LOG_WARNING("LEONexam (%s): bad TESTWORD (0x%04X)", leon->tap->dotted_name, drcv[1]);
			break;
		} else {
			LOG_DEBUG("LEONexam (%s): TESTWORD is OK (0x%04X)", leon->tap->dotted_name, drcv[1]);
		}

//		rc = leon_read_all_registers(target, 1);
//		if (rc!=ERROR_OK) {
//			LOG_ERROR("Reading control registers failed");
//			break;
//		}
		if (leon->ltype==LEON_TYPE_L2MT) {
			LOG_USER("LEONexam (%s) MT mode - skip testing of L2 registers", leon->tap->dotted_name);
			target->state = TARGET_HALTED;
		} else {
			uint32_t *plcfg = leon_get_ptrreg(target, LEON_RID_LCFG);
			if (!plcfg) break;
			retval = leon_jtag_get_registers(target, LEON_REGS_LEON_CFG_REG, plcfg, 1);
			if (retval!=ERROR_OK) break;
			uint32_t *ppsr = leon_get_ptrreg(target, LEON_RID_PSR);
			if (!ppsr) break;
			retval = leon_jtag_get_registers(target, LEON_DSU_SREG_PSR, ppsr, 1);
			if (retval!=ERROR_OK) break;
			LOG_USER("LEONexam (%s) Configuration = 0x%08x", leon->tap->dotted_name, *plcfg);
			LOG_USER(" - Sparc IMPL=x%X, VER=x%X",
								LEON_GET_VAL(ppsr, SPARC_V8_PSR_IMPL),
								LEON_GET_VAL(ppsr, SPARC_V8_PSR_VER));
			if (LEON_GET_BIT(ppsr, SPARC_V8_PSR_EF)) {
				uint32_t *pfsr = leon_get_ptrreg(target, LEON_RID_FSR);
				if (!pfsr) break;
				retval = leon_jtag_get_registers(target, LEON_DSU_SREG_FSR, pfsr, 1);
				if (retval!=ERROR_OK) break;
				LOG_USER(" - Sparc FPU (type=%lu) VER=x%X",
									LEON_GET_VAL(plcfg, LEON_CFG_REG_FPU_TYPE),
									LEON_GET_VAL(pfsr, SPARC_V8_FSR_VER));
			}
			if (LEON_GET_BIT(plcfg, LEON_CFG_REG_IS_DSU)) {
				uint32_t rdsu;
				uint32_t *preg = leon_get_ptrreg(target, LEON_RID_DSUCTRL);
				if (!preg) break;
				retval = leon_jtag_get_registers(target, LEON_DSU_CTRL_REG, preg, 1);
				if (retval!=ERROR_OK) break;
				LOG_USER(" - DSU Control Register = 0x%X", *preg);
				rdsu = *preg;
				preg = leon_get_ptrreg(target, LEON_RID_DSUTRAP);
				if (!preg) break;
				retval = leon_jtag_get_registers(target, LEON_DSU_SREG_DSU_TRAP, preg, 1);
				if (retval!=ERROR_OK) break;
				LOG_USER(" - DSU Trap Register    = 0x%X", *preg);
				preg = leon_get_ptrreg(target, LEON_RID_TRCCTRL);
				if (!preg) break;
				retval = leon_jtag_get_registers(target, LEON_DSU_TRACE_CTRL_REG, preg, 1);
				if (retval!=ERROR_OK) break;
				LOG_USER(" - DSU Trace Ctrl Reg.  = 0x%X", *preg);
				leon_update_tmode(leon, rdsu, *preg);
			} else {
				LOG_WARNING("The target doesn't contain DSU");
			}
		}

		leon_check_regcache(target);

		target_set_examined(target);
//target->state = TARGET_HALTED;
		retval = ERROR_OK;
	} while(0);
	LEON_TM_MEASURE(bench, leon->loptime);
	if (leon->ltype==LEON_TYPE_UNKNOWN)
		LOG_WARNING("Type of LEON processor hasn't been detected - use command 'leontype' for manual selection.");
	return retval;
}


static int leon_arch_state(struct target *target)
{
	//struct leon_common *leon = target_to_leon(target);
	uint32_t *ppc = leon_get_ptrreg(target, LEON_RID_PC);
	if (ppc) {
		LOG_USER("target in state '%s' due to %s, pc: 0x%8.8" PRIx32 "",
		         target_state_name(target), debug_reason_name(target), *ppc);
	}
	return ERROR_OK;
}


static int leon_poll(struct target *target)
{
	int retval;
//	struct leon_common *leon = target_to_leon(target);
//	uint32_t *pdsu = leon_get_ptrreg(target, LEON_RID_DSUCTRL);

	if (!target_was_examined(target)) {
		LOG_WARNING("Target hasn't been examined yet\n");
		return ERROR_OK;
	}

	struct leon_common *pl = target_to_leon(target);

	if (pl->ltype==LEON_TYPE_L2MT) {
		retval = leon_read_register(target, LEON_RID_DSUCTRL, 1);
		target->state = TARGET_HALTED;
		target->debug_reason = DBG_REASON_UNDEFINED;
	} else {
		retval = leon_read_register(target, LEON_RID_DSUCTRL, 1);
		if (retval==ERROR_OK) {
			retval = leon_read_register(target, LEON_RID_DSUTRAP, 1);
		}
		if (retval!=ERROR_OK) {
			target->state = TARGET_UNKNOWN;
			target->debug_reason = DBG_REASON_UNDEFINED;
			return ERROR_FAIL;
		}
	}
enum target_state lst = target->state;
	leon_check_state_and_reason(target, &target->state, &target->debug_reason);
if (lst!=target->state) {
	LOG_INFO("Target state changed from %d to %d\n", lst, target->state);
}
	return ERROR_OK;
}


static int leon_halt(struct target *target)
{
	int retval;
	struct leon_common *leon = target_to_leon(target);

	if (target->state == TARGET_HALTED) {
		LOG_DEBUG("target is already halted.");
		return ERROR_OK;
	}
	do {
		LEON_TM_START(bench);

		if (leon->ltype==LEON_TYPE_L2MT) {
			target->state = TARGET_HALTED;
		} else {
			uint32_t *pdsu = leon_get_ptrreg(target, LEON_RID_DSUCTRL);
			if (!pdsu) break;
			retval = leon_read_register(target, LEON_RID_DSUCTRL, 1);
	//LOG_INFO(" TgtHALT RD DSUctrl x%08X", *pdsu);
			if (retval!=ERROR_OK) break;
			leon_check_state_and_reason(target, &target->state, &target->debug_reason);
			if (target->state == TARGET_HALTED) return ERROR_OK;
			/* halt processor - break now */
			retval = leon_write_register(target, LEON_RID_DSUCTRL,
									(*pdsu & LEON_DSU_CTRL_RW_MASK) | LEON_DSU_CTRL_BRK_NOW |
										LEON_DSU_CTRL_BRK_IU_WPT);
			if (retval!=ERROR_OK) break;

			retval = leon_read_register(target, LEON_RID_DSUCTRL, 1);
	//LOG_INFO(" TgtHALT reRD DSUctrl x%08X", *pdsu);
			if (retval!=ERROR_OK) break;
			leon_check_state_and_reason(target, &target->state, &target->debug_reason);
			if (target->state!=TARGET_HALTED) {
				LOG_WARNING("Target is not in the debug mode");
				return ERROR_TARGET_NOT_HALTED;
			}

			register_cache_invalidate(leon->regdesc);
			retval = leon_read_all_registers(target, 0);

			retval = target_call_event_callbacks(target, TARGET_EVENT_HALTED);
			if (retval!=ERROR_OK) {
				LOG_ERROR("error while calling callback 'TARGET_EVENT_HALTED'");
				return retval;
			}
		}

		LEON_TM_MEASURE(bench, leon->loptime);

		return ERROR_OK;
	} while(0);

	LOG_ERROR("Entering to debug mode failed");
	target->state = TARGET_UNKNOWN;
	target->debug_reason = DBG_REASON_UNDEFINED;
	return ERROR_TARGET_NOT_HALTED;
}


static int leon_resume(struct target *target, int current, uint32_t address,
		int handle_breakpoints, int debug_execution)
{
	int retval;
	struct leon_common *leon = target_to_leon(target);

	if (target->state != TARGET_HALTED) {
		LOG_ERROR("Target not halted");
		return ERROR_TARGET_NOT_HALTED;
	}

	do {
		LEON_TM_START(bench);

		uint32_t *pdsu = leon_get_ptrreg(target, LEON_RID_DSUCTRL);
		if (!pdsu) break;
		retval = leon_read_register(target, LEON_RID_DSUCTRL, 1);
		if (retval!=ERROR_OK) break;
		leon_check_state_and_reason(target, &target->state, &target->debug_reason);
		if (target->state != TARGET_HALTED) return ERROR_OK;

		/* resume processor - unbreak now, if CPU error is detected try reset it */
		if (LEON_GET_BIT(pdsu, LEON_DSU_CTRL_CPU_ERR)) {
			LOG_INFO("Target '%s' - Processor is in ERROR MODE", leon->tap->dotted_name);
			retval = leon_write_register(target, LEON_RID_DSUCTRL, *pdsu | LEON_DSU_CTRL_RESET_CPU_ERR);
			if (retval!=ERROR_OK) break;
			retval = leon_read_register(target, LEON_RID_DSUCTRL, 1);
			if (retval!=ERROR_OK) break;
			if (LEON_GET_BIT(pdsu, LEON_DSU_CTRL_CPU_ERR)) {
				LOG_ERROR("Target '%s' - CPU is still in ERROR MODE", leon->tap->dotted_name);
				break;
			}
		}
		LOG_WARNING("try resume from 0x%08X(%d) (0x%08X)", address, current, *pdsu);
		/* set PC to address if not current */
		if (!current) {
			leon_write_register(target, LEON_RID_NPC, address);
		} else {
			uint32_t *pa = leon_get_ptrreg(target, LEON_RID_NPC);
			leon_read_register(target, LEON_RID_NPC, 1);
			address = *pa;
		}

		target->debug_reason = DBG_REASON_NOTHALTED;
		/* Registers are now invalid */
		register_cache_invalidate(leon->regdesc);

		/* disable break */
		retval = leon_write_register(target, LEON_RID_DSUCTRL, *pdsu & ~LEON_DSU_CTRL_BRK_NOW);
		if (retval!=ERROR_OK) break;

		retval = leon_read_register(target, LEON_RID_DSUCTRL, 1);
		if (retval!=ERROR_OK) break;
		leon_check_state_and_reason(target, &target->state, &target->debug_reason);
// program in the target can be finished immediately - poll can check this later
//		if (target->state==TARGET_HALTED) {
//			LOG_WARNING("Target is still in the debug mode (0x%08X)", *pdsu);
//			return ERROR_TARGET_FAILURE;
//		}

		if (!debug_execution) {
			target->state = TARGET_RUNNING;
			target_call_event_callbacks(target, TARGET_EVENT_RESUMED);
			LOG_DEBUG("Target resumed at 0x%08" PRIx32, address);
		} else {
			target->state = TARGET_DEBUG_RUNNING;
			target_call_event_callbacks(target, TARGET_EVENT_DEBUG_RESUMED);
			LOG_DEBUG("Target debug resumed at 0x%08" PRIx32, address);
		}

		LEON_TM_MEASURE(bench, leon->loptime);

		return ERROR_OK;
	} while(0);
	LOG_ERROR("Resume from debug mode failed");
	return ERROR_TARGET_FAILURE;
}


static int leon_step(struct target *target, int current,
		uint32_t address, int handle_breakpoints)
{
	int retval;
	struct leon_common *leon = target_to_leon(target);
	uint32_t *pdsu = leon_get_ptrreg(target, LEON_RID_DSUCTRL);
	if (!pdsu) return ERROR_FAIL;

	if (target->state != TARGET_HALTED) {
		LOG_ERROR("Target not halted");
		return ERROR_TARGET_NOT_HALTED;
	}

	do {
		/* current = 1: continue on current pc, otherwise continue at <address> */
		if (!current) {
			retval = leon_write_register(target, LEON_RID_PC, address);
			if (retval==ERROR_OK)
				retval = leon_write_register(target, LEON_RID_NPC, address+4);
			if (retval!=ERROR_OK) {
				LOG_ERROR(" - Set PC failed");
				break;
			}
		}

		LEON_TM_START(bench);

		retval = leon_read_register(target, LEON_RID_DSUCTRL, 1);
		if (retval!=ERROR_OK) break;
		leon_check_state_and_reason(target, &target->state, &target->debug_reason);
		if (target->state != TARGET_HALTED) return ERROR_OK;

		target->debug_reason = DBG_REASON_SINGLESTEP;

		retval = leon_write_register(target, LEON_RID_DSUCTRL, (*pdsu & ~LEON_DSU_CTRL_BRK_NOW) | LEON_DSU_CTRL_SINGLE_STEP);
		if (retval!=ERROR_OK) break;

		retval = target_call_event_callbacks(target, TARGET_EVENT_RESUMED);
		if (retval != ERROR_OK) break;

		retval = leon_read_register(target, LEON_RID_DSUCTRL, 1);
		if (retval!=ERROR_OK) break;
		leon_check_state_and_reason(target, &target->state, &target->debug_reason);

		/* registers are now invalid */
		register_cache_invalidate(leon->regdesc);
//LOG_USER(">>> TARGET state = %s", debug_reason_name(target));
		if (target->state == TARGET_HALTED) {
			retval = leon_read_all_registers(target, 1);
			retval = leon_write_register(target, LEON_RID_DSUCTRL, *pdsu & ~LEON_DSU_CTRL_SINGLE_STEP);
			if (retval!=ERROR_OK) break;
			retval = target_call_event_callbacks(target, TARGET_EVENT_HALTED);
			if (retval != ERROR_OK) break;
			LOG_DEBUG("target stepped");
		}

		LEON_TM_MEASURE(bench, leon->loptime);

		return ERROR_OK;
	} while(0);

	LOG_ERROR("Step failed");
	return retval;
}

/* LEON2 with DSU doesn't support software controlled HW reset */
//static int leon_assert_reset(struct target *target)
//{
//	target->state = TARGET_RESET;
//	LOG_INFO("%s", __func__);
//	return ERROR_OK;
//}
//static int leon_deassert_reset(struct target *target)
//{
//	target->state = TARGET_RUNNING;
//	LOG_INFO("%s", __func__);
//	return ERROR_OK;
//}

int leon_soft_reset_halt(struct target *target)
{
	int retval;
	struct leon_common *leon = target_to_leon(target);
	uint32_t *pdsu = leon_get_ptrreg(target, LEON_RID_DSUCTRL);

	if (leon->ltype==LEON_TYPE_UNKNOWN) {
		LOG_ERROR("Type of Leon CPU is not set");
		return ERROR_FAIL;
	}

	retval = target_halt(target);
	if (retval != ERROR_OK)
		return retval;

	LEON_TM_START(bench);

	int timeout = 0;
	while (1) {
		LEON_TM_MEASURE(bench, leon->loptime);
		if (leon->loptime>1000000) { timeout = 1; break; }

		retval = leon_read_register(target, LEON_RID_DSUCTRL, 1);
		if (retval != ERROR_OK) return retval;
		if (LEON_GET_BIT(pdsu, LEON_DSU_CTRL_DEBUG_MODE)) break; /* halted */
		if (debug_level >= 3)
			alive_sleep(100);
		else
			keep_alive();
	}
	if (timeout) {
		LOG_ERROR("Failed to halt CPU after 1 sec");
		return ERROR_TARGET_TIMEOUT;
	}
	target->state = TARGET_HALTED;

	/* set all registers to default state */
	retval = leon_write_register(target, LEON_RID_PC, 0);
	retval = leon_write_register(target, LEON_RID_NPC, 4);
	if (leon->ltype==LEON_TYPE_L2 || leon->ltype==LEON_TYPE_L2FT) {
		uint32_t *ppsr = leon_get_ptrreg(target, LEON_RID_PSR);
		retval = leon_write_register(target, LEON_RID_PSR, (*ppsr & ~SPARC_V8_PSR_ET) | SPARC_V8_PSR_S);
		retval = leon_write_register(target, LEON_RID_CCR, 0);
	} else {
		LOG_WARNING("The current LeonType is not fully supported.");
	}
//	int i;
//	for (i = LEON_RID_R0; i <= LEON_RID_R31; ++i) {
//		retval = leon_write_register(target, i, 0xffffffff);
//	}

	register_cache_invalidate(leon->regdesc);

	retval = target_call_event_callbacks(target, TARGET_EVENT_HALTED);
	if (retval != ERROR_OK)
		return retval;

	return ERROR_OK;
}

//int leon_reset_callback(struct target *target, enum target_reset_mode reset_mode, void *priv)
//{
//	//struct leon_common *leon = target_to_leon(target);
//	LOG_INFO("leon reset callback t=%d", reset_mode);
//	return ERROR_OK;
//}

/* -------------------------------------------------------------------------- */
int leon_get_gdb_reg_list(struct target *target,
		struct reg **reg_list[], int *reg_list_size,
		enum target_register_class reg_class)
{
	//struct leon_common *leon = target_to_leon(target);

	switch (reg_class) {
		case REG_CLASS_ALL:
			return leon_get_all_reg_list(target, reg_list, reg_list_size);
		case REG_CLASS_GENERAL:
			return leon_get_general_reg_list(target, reg_list, reg_list_size);
		default:
			break;
	}

	return ERROR_FAIL;
}

// -----------------------------------------------------------------------------
static int leon_read_memory(struct target *target, uint32_t addr,
	uint32_t size, uint32_t count, uint8_t *buf)
{
	int retval = ERROR_OK;
	struct leon_common *leon = target_to_leon(target);
	struct scan_field field;

	uint32_t addr_first, addr_last;
	uint32_t cnt_all, cnt_done, cnt_batch, cnt_errors;
	uint32_t size_batch;
	uint32_t ofst_first;

	uint8_t outval[4];
	uint8_t *pinval = NULL; // buffer for incoming data
	uint32_t res_ctrl, res_ahi, res_alo; // auxiliary space

	LOG_DEBUG("rd_mem address: 0x%8.8" PRIx32 ", size: 0x%8.8" PRIx32 ", count: 0x%8.8" PRIx32 "",
	          addr, size, count);

	if (target->state != TARGET_HALTED) {
		LOG_ERROR("Target not halted");
		return ERROR_TARGET_NOT_HALTED;
	}

	/* sanitize arguments */
	if (((size != 4) && (size != 2) && (size != 1)) || (count == 0) || !(buf)) {
		LOG_ERROR("Bad arguments");
		return ERROR_COMMAND_SYNTAX_ERROR;
	}
	if (((size == 4) && (addr & 0x3u)) || ((size == 2) && (addr & 0x1u))) {
		LOG_ERROR("Can't handle unaligned memory access");
		return ERROR_TARGET_UNALIGNED_ACCESS;
	}

	/* init variables */
	addr_first = addr & (~0x3u);
	ofst_first = (addr & 0x3u)/size;
	addr_last = (addr + (count-1)*size) & (~0x3u);
	cnt_all = ((addr_last-addr_first)/4)+1;
	cnt_done = 0;
	cnt_batch = 0;
	cnt_errors = 0;
	size_batch = (cnt_all>LEON_BLOCK_MAX_LENGTH) ? LEON_BLOCK_MAX_LENGTH : cnt_all;
//	LOG_INFO("Params: af=x%X , al=x%X (of=%u) cnt_all = %u, bsz = %u",
//	         addr_first, addr_last, ofst_first, cnt_all, size_batch);

	pinval = calloc(2*size_batch + 1, 4); /* we need 2 data words for each U32 and one CRC for one batch */
	if (pinval==NULL) {
		LOG_ERROR("Unable to allocate memory buffer.");
		return ERROR_FAIL;
	}

	LEON_TM_START(bench);

	do {
		retval = leon_jtag_set_instr(leon->tap, LEON_IRINS_ENTER, 1);
		if (retval!=ERROR_OK) {
			LOG_ERROR("LEON set instruction failed (%s)", leon->tap->tapname);
			break;
		}

		field.num_bits = LEON_DRSCAN_NBITS;
		field.out_value = outval;

		while (cnt_done < cnt_all) { /* number of transfers of U32 words */
			int iserr = 0;
			uint32_t i, curcnt;
			curcnt = ((cnt_all-cnt_done) >= size_batch) ? size_batch : (cnt_all-cnt_done);
			//LOG_INFO("block %d %u (%u/%u) @ x%X [err=%d]", cnt_batch, curcnt, cnt_done, cnt_all, addr_first, cnt_errors);
// CONTROL WORD
			buf_set_u32(&outval[0], 0, LEON_DRSCAN_NBITS, LEON_PID_CONTROL | LEON_CTRL_READ_DATA | LEON_CTRL_ADDR_INC | curcnt);
			field.in_value = (uint8_t *) &res_ctrl;
			jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
// ADDRESS HI
			buf_set_u32(&outval[0], 0, LEON_DRSCAN_NBITS, LEON_PID_ADDRESS | (addr_first>>16));
			field.in_value = (uint8_t *) &res_ahi;
			jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
// ADDRESS LO
			buf_set_u32(&outval[0], 0, LEON_DRSCAN_NBITS, LEON_PID_ADDRESS | (addr_first&0xffff));
			field.in_value = (uint8_t *) &res_alo;
			jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
// DATA
			buf_set_u32(&outval[0], 0, LEON_DRSCAN_NBITS, LEON_PID_DATA | 0x0000);
			for (i=0;i<2*curcnt-1;++i) {
				field.in_value = (uint8_t *) &pinval[i*4];
				jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
			}
			buf_set_u32(&outval[0], 0, LEON_DRSCAN_NBITS, LEON_PID_PAD | 0x0000);
			field.in_value = (uint8_t *) &pinval[(2*curcnt-1)*4];
			jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
// CRC
			field.in_value = (uint8_t *) &pinval[2*curcnt*4];
			jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);

// execute queue
			retval = jtag_execute_queue();
			if (retval!=ERROR_OK) {
				LOG_ERROR("Execute JTAG queue failed (%s, %d)", leon->tap->tapname, retval);
				break;
			}

// check CRC and save data to output buffer
			uint16_t crcval = LEON_CRC_INIT;
			uint32_t *pd = (uint32_t *)pinval;
			uint32_t oval;
			uint32_t chpid = 0;
			uint8_t *curbuf = buf;
			for (i=0;i<curcnt;++i) {
				leon_update_crc16(&crcval, (uint16_t)*pd);
				oval = (*pd & 0xffff)<<16;
				chpid += (*pd & LEON_PID_MASK)>>16;
				pd++;
				leon_update_crc16(&crcval, (uint16_t)*pd);
				oval |= *pd & 0xffff;
				chpid += (*pd & LEON_PID_MASK)>>16;
				pd++;
				switch (size) {
					case 4:
						if (target->endianness == TARGET_BIG_ENDIAN)
							oval = leon_swap_u32(oval);
						*((uint32_t *)curbuf) = oval;
						curbuf += 4;
						break;
					case 2:
						if (count && ofst_first==0) {
							if (target->endianness == TARGET_BIG_ENDIAN)
								*((uint16_t *)curbuf) = leon_swap_u16((uint16_t)(oval>>16));
							else
								*((uint16_t *)curbuf) = (uint16_t)(oval>>16);
							curbuf += 2;
							count--;
						}
						if (count) {
							if (target->endianness == TARGET_BIG_ENDIAN)
								*((uint16_t *)curbuf) = leon_swap_u16((uint16_t)(oval&0xffff));
							else
								*((uint16_t *)curbuf) = (uint16_t)(oval & 0xffff);
							curbuf += 2;
							count--;
							ofst_first = 0;
						}
						break;
					case 1:
						if (count && ofst_first==0) {
							*((uint8_t *)curbuf) = (uint8_t)(oval>>24);
							curbuf++;
							count--;
						}
						if (count && ofst_first<2) {
							*((uint8_t *)curbuf) = (uint8_t)(oval>>16);
							curbuf++;
							count--;
						}
						if (count && ofst_first<3) {
							*((uint8_t *)curbuf) = (uint8_t)(oval>>8);
							curbuf++;
							count--;
						}
						if (count) {
							*((uint8_t *)curbuf) = (uint8_t)(oval>>0);
							curbuf++;
							count--;
							ofst_first = 0;
						}
						break;
				}
			}
			if (chpid!=2*curcnt*(LEON_PID_DATA>>16)) {
				LOG_WARNING("Read block (%u) - returned bad responses (x%04X,x%04X).",
				            cnt_batch, chpid, 2*curcnt*(LEON_PID_DATA>>16));
				iserr = 1;
			}
			if (crcval != *pd) {
				LOG_WARNING("Read block (%u) - bad CRC (x%04X,x%04X).",
				            cnt_batch, *pd, crcval);
				iserr = 1;
			}
			if (iserr) {
				if (++cnt_errors>=5) {
					LOG_ERROR("Read block of memory failed 5 times.");
					retval = ERROR_FAIL;
					break;
				}
			} else {
				buf = curbuf;
				cnt_done += curcnt;
				addr_first += curcnt*4;
				cnt_batch++;
			}
			keep_alive(); /* keep alive gdb connection */
		}
	} while(0);

	LEON_TM_MEASURE(bench, leon->loptime);
//	LOG_INFO("rd loptime = %u (%f)\n", leon->loptime, duration_elapsed(&bench));

	free(pinval);
	return retval;
}


static int leon_write_memory(struct target *target, uint32_t addr,
	uint32_t size, uint32_t count, const uint8_t *buf)
{
//	/* ?TODO: switch according type of memory (RAM, registers, FLASH, ...) */
	int retval = ERROR_OK;
	struct leon_common *leon = target_to_leon(target);
	struct scan_field field;
	void *t = NULL;

	uint32_t addr_first, addr_last;
	uint32_t cnt_all, cnt_done, cnt_batch, cnt_errors;
	uint32_t size_batch;
	uint32_t ofst_first, ofst_last;

	uint8_t inval[4], outval[4];

	uint16_t crcval;

	LOG_DEBUG("wr_mem address: 0x%8.8" PRIx32 ", size: 0x%8.8" PRIx32 ", count: 0x%8.8" PRIx32 "",
	          addr, size, count);

	if (target->state != TARGET_HALTED) {
		LOG_ERROR("Target not halted");
		return ERROR_TARGET_NOT_HALTED;
	}

	/* sanitize arguments */
	if (((size != 4) && (size != 2) && (size != 1)) || (count == 0) || !(buf)) {
		LOG_ERROR("Bad arguments");
		return ERROR_COMMAND_SYNTAX_ERROR;
	}
	if (((size == 4) && (addr & 0x3u)) || ((size == 2) && (addr & 0x1u))) {
		LOG_ERROR("Can't handle unaligned memory access");
		return ERROR_TARGET_UNALIGNED_ACCESS;
	}

	/* init variables */
	addr_first = addr & (~0x3u);
	ofst_first = (addr & 0x3u)/size;
	addr_last = (addr + (count-1)*size) & (~0x3u);
	ofst_last = ((addr + (count-1)*size) & 0x3u)/size;
	cnt_all = ((addr_last-addr_first)/4)+1;
	cnt_done = 0;
	cnt_batch = 0;
	cnt_errors = 0;
	size_batch = (cnt_all>LEON_BLOCK_MAX_LENGTH) ? LEON_BLOCK_MAX_LENGTH : cnt_all;
//	LOG_INFO("Params: af=x%X , al=x%X (of=%u) cnt_all = %u, bsz = %u",
//	         addr_first, addr_last, ofst_first, cnt_all, size_batch);

	/* The current implementation writes only to full 32bit words */
	if (ofst_first!=0 || (ofst_last!=(4/size)-1)) {
		LOG_ERROR("Writing to memory (address and size) must be aligned to 32bit.");
		return ERROR_TARGET_UNALIGNED_ACCESS;
	}

	/* endianness */
	if ((target->endianness == TARGET_BIG_ENDIAN) && (size != 1)) {
		t = malloc(count * size * sizeof(uint8_t));
		if (t == NULL) {
			LOG_ERROR("Out of memory");
			return ERROR_FAIL;
		}
		switch (size) {
			case 4:
				buf_bswap32(t, buf, size * count);
				break;
			case 2:
				buf_bswap16(t, buf, size * count);
				break;
		}
		buf = t;
	}

	LEON_TM_START(bench);

	do {
		retval = leon_jtag_set_instr(leon->tap, LEON_IRINS_ENTER, 1);
		if (retval!=ERROR_OK) {
			LOG_ERROR("LEON set instruction failed (%s)", leon->tap->tapname);
			break;
		}

		field.num_bits = LEON_DRSCAN_NBITS;
		field.out_value = outval;
		field.in_value = inval;

		while (cnt_done < cnt_all) { /* number of transfers of U32 words */
			int iserr = 0;
			uint32_t i, curcnt;
			curcnt = ((cnt_all-cnt_done) >= size_batch) ? size_batch : (cnt_all-cnt_done);
			//LOG_INFO("block %d %u (%u/%u) @ x%X [err=%d]", cnt_batch, curcnt, cnt_done, cnt_all, addr_first, cnt_errors);
// CONTROL WORD
			buf_set_u32(&outval[0], 0, LEON_DRSCAN_NBITS, LEON_PID_CONTROL | LEON_CTRL_WRITE_DATA | LEON_CTRL_ADDR_INC | curcnt);
			jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
// ADDRESS HI
			buf_set_u32(&outval[0], 0, LEON_DRSCAN_NBITS, LEON_PID_ADDRESS | (addr_first>>16));
			jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
// ADDRESS LO
			buf_set_u32(&outval[0], 0, LEON_DRSCAN_NBITS, LEON_PID_ADDRESS | (addr_first&0xffff));
			jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
// DATA - load data from output buffer and compute CRC
			crcval = LEON_CRC_INIT;
			for (i=0;i<curcnt;++i) {
// TODO: switch size 4,2,1
				uint32_t dval = *(uint32_t*)(buf + (cnt_done+i)*4);
				leon_update_crc16(&crcval, (uint16_t)(dval>>16));
				buf_set_u32(&outval[0], 0, LEON_DRSCAN_NBITS, LEON_PID_DATA | (uint16_t)(dval>>16));
				jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
				leon_update_crc16(&crcval, (uint16_t)(dval&0xffff));
				buf_set_u32(&outval[0], 0, LEON_DRSCAN_NBITS, LEON_PID_DATA | (uint16_t)(dval&0xffff));
				jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
			}
// CRC
			buf_set_u32(&outval[0], 0, LEON_DRSCAN_NBITS, LEON_PID_PAD | 0);
			jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);

// execute queue
			retval = jtag_execute_queue();
			if (retval!=ERROR_OK) {
				LOG_ERROR("Execute JTAG queue failed (%s, %d)", leon->tap->tapname, retval);
				break;
			}

// check CRC
			uint32_t incrc = buf_get_u32(inval, 0, LEON_DRSCAN_NBITS);
			if ((incrc & LEON_PID_MASK)!=LEON_PID_PAD) {
				LOG_WARNING("Write block (%u) - returned bad responses (x%05X).",
				            cnt_batch, incrc);
				iserr = 1;
			}
			if (crcval != (incrc & LEON_VALUE_MASK)) {
				LOG_WARNING("Write block (%u) - bad CRC (x%04X,x%04X).",
				            cnt_batch, (incrc & LEON_VALUE_MASK), crcval);
				iserr = 1;
			}
			if (iserr) {
				if (++cnt_errors>=5) {
					LOG_ERROR("Write block of memory failed 5 times.");
					retval = ERROR_FAIL;
					break;
				}
			} else {
				cnt_done += curcnt;
				addr_first += curcnt*4;
				cnt_batch++;
			}
			keep_alive(); /* keep alive gdb connection */
		}
	} while(0);

	LEON_TM_MEASURE(bench, leon->loptime);

	if (t!=NULL) free(t);
	return retval;
}


static int leon_checksum_memory(struct target *t, uint32_t a, uint32_t s,
		uint32_t *checksum)
{
	LOG_WARNING("Not implemented: %s", __func__);
	return ERROR_FAIL;
}

static int leon_blank_check_memory(struct target *target, uint32_t address,
		uint32_t count, uint32_t *blank, uint8_t erased_value)
{
	LOG_WARNING("Not implemented: %s", __func__);
	return ERROR_OK;
}


static int leon_run_algorithm(struct target *target, int num_mem_params,
		struct mem_param *mem_params, int num_reg_params,
		struct reg_param *reg_param, uint32_t entry_point,
		uint32_t exit_point, int timeout_ms, void *arch_info)
{
	LOG_WARNING("Not implemented: %s", __func__);
	return ERROR_OK;
}


static int leon_add_breakpoint(struct target *target, struct breakpoint *breakpoint)
{
//	struct leon_common *leon = target_to_leon(target);
	uint32_t data;
	int retval;

	LOG_DEBUG("Adding breakpoint: addr 0x%08" PRIx32 ", len %d, type %d, set: %d, id: %" PRId32,
	          breakpoint->address, breakpoint->length, breakpoint->type,
	          breakpoint->set, breakpoint->unique_id);

	/* Only support SW breakpoints for now. */
	if (breakpoint->type == BKPT_HARD) {
		LOG_INFO("... add HW breakpoint ... use DSU hw breakpoints ... TODO");
		// count number of HW breakpoints -> too many/one/none
		struct breakpoint *pbp = target->breakpoints;
		int hwcnt = 0;
		while (pbp) {
			LOG_INFO("brk = %p", pbp);
			if (pbp->type == BKPT_HARD) {
				hwcnt++;
			}
			pbp = pbp->next;
		}
		if (hwcnt==2) {
			return ERROR_TARGET_RESOURCE_NOT_AVAILABLE;
		} else {
			// TODO
			return ERROR_FAIL;
		}

	} else {
		LOG_ERROR("... add SW breakpoint");

		/* Read and save the instruction */
		retval = leon_read_memory(target, breakpoint->address, 4, 1, (uint8_t *)&data);
		if (retval != ERROR_OK) {
			LOG_ERROR("Error while reading the instruction at 0x%08" PRIx32,
			          breakpoint->address);
			return retval;
		}

		if (breakpoint->orig_instr != NULL)
			free(breakpoint->orig_instr);

		breakpoint->orig_instr = malloc(breakpoint->length);
		memcpy(breakpoint->orig_instr, &data, breakpoint->length);

		/* Sub in the OR1K trap instruction */
		target_buffer_set_u32(target, (uint8_t *)&data, LEON_DSU_TRAP_INSTR);
		retval = leon_write_memory(target, breakpoint->address, 4, 1, (uint8_t *)&data);
		if (retval != ERROR_OK) {
			LOG_ERROR("Error while writing SW BREAK at 0x%08" PRIx32,
			          breakpoint->address);
			return retval;
		}
		/* TODO: invalidate instruction cache */
//		retval = leon_invalidate_icache(target, 1, &breakpoint->address);
//		if (retval != ERROR_OK) {
//			LOG_ERROR("Error while invalidating the ICACHE");
//			return retval;
//		}
	}
	return ERROR_OK;
}


static int leon_remove_breakpoint(struct target *target, struct breakpoint *breakpoint)
{
//	struct leon_common *leon = target_to_leon(target);
	int retval;

	LOG_DEBUG("Removing breakpoint: addr 0x%08" PRIx32 ", len %d, type %d, set: %d, id: %" PRId32,
	          breakpoint->address, breakpoint->length, breakpoint->type,
	          breakpoint->set, breakpoint->unique_id);

	if (breakpoint->type == BKPT_HARD) {
		LOG_INFO("... add HW breakpoint ... TODO");
		
	} else {
		/* Replace the removed instruction */
		retval = leon_write_memory(target, breakpoint->address,
		                           4, 1, breakpoint->orig_instr);
		if (retval != ERROR_OK) {
			LOG_ERROR("Error while writing back the instruction at 0x%08" PRIx32,
			          breakpoint->address);
			return retval;
		}

		/* TODO: invalidate instruction cache */
//		retval = leon_invalidate_icache(target, 1, &breakpoint->address);
//		if (retval != ERROR_OK) {
//			LOG_ERROR("Error while invalidating the ICACHE");
//			return retval;
//		}
	}
	return ERROR_OK;
}


static int leon_add_watchpoint(struct target *target, struct watchpoint *watchpoint)
{
	LOG_DEBUG("%s", __func__);
	return ERROR_OK;
}


static int leon_remove_watchpoint(struct target *target, struct watchpoint *watchpoint)
{
	LOG_DEBUG("%s", __func__);
	return ERROR_OK;
}

/* ========================================================================== */
COMMAND_HANDLER(leon_handle_loptime_command)
{
	struct target *tgt = leon_cmd_to_tgt(cmd);
	struct leon_common *leon;

	if (tgt==NULL) {
		LOG_ERROR("Target type is not '" LEON_TYPE_NAME "'");
		return ERROR_FAIL;
	}
	leon = target_to_leon(tgt);
	command_print(CMD_CTX, "Time of the last LEON operation : %u us.", leon->loptime);
	return ERROR_OK;
}


COMMAND_HANDLER(leon_handle_hwinfo_command)
{
	struct target *tgt = leon_cmd_to_tgt(cmd);
	int retval = 0;

	if (tgt==NULL) {
		LOG_ERROR("Target type is not '" LEON_TYPE_NAME "'");
		return ERROR_FAIL;
	}
	do {
		retval = leon_read_all_registers(tgt, 1);
		if (retval!=ERROR_OK) {
			LOG_WARNING("Not all regs have been read");
			break;
		}
		print_hwinfo(CMD_CTX, tgt);
	} while (0);
	return retval;
}


COMMAND_HANDLER(leon_handle_leontype_command)
{
	struct target *tgt = leon_cmd_to_tgt(cmd);
	struct leon_common *leon;

	if (tgt==NULL) {
		LOG_ERROR("Target type is not '" LEON_TYPE_NAME "'");
		return ERROR_FAIL;
	}
	leon = target_to_leon(tgt);

	if (CMD_ARGC > 2) {
		return ERROR_COMMAND_SYNTAX_ERROR;
	}

	if (CMD_ARGC > 0) {
		enum leon_type prevtype = leon->ltype;
		if (!strcmp(CMD_ARGV[0], "l2"))
			leon->ltype = LEON_TYPE_L2;
		else if (!strcmp(CMD_ARGV[0], "l2ft"))
			leon->ltype = LEON_TYPE_L2FT;
		else if (!strcmp(CMD_ARGV[0], "l2mt")) {
			leon->ltype = LEON_TYPE_L2MT;
			if (CMD_ARGC > 1) {
				unsigned int nl = strtoul(CMD_ARGV[1], NULL, 0);
				if (__builtin_popcount(nl)>1) {
					return ERROR_COMMAND_SYNTAX_ERROR;
				}
				if (nl<4)
					leon->mt_ctlblk_size = -1;
				else
					leon->mt_ctlblk_size = nl;
			}
		} else
//			leon->ltype = LEON_TYPE_UNKNOWN;
			return ERROR_COMMAND_SYNTAX_ERROR;
		if (prevtype!=leon->ltype) {
			leon_rename_reg_cache(tgt, leon->ltype);
		}
	}

	if (leon->ltype==LEON_TYPE_L2MT && leon->mt_ctlblk_size>0)
		command_print(CMD_CTX, "LEON processor type = %s, ctlblock = %d bytes",
		              leon_type_to_string(leon->ltype), leon->mt_ctlblk_size);
	else
		command_print(CMD_CTX, "LEON processor type = %s",
		              leon_type_to_string(leon->ltype));
	return ERROR_OK;
}

static int leon_disp_parsed_register(struct command_invocation *cmd, const leon_reg_parsed_t *pr)
{
	struct target *tgt = leon_cmd_to_tgt(cmd);
	uint32_t *preg = leon_get_ptrreg(tgt, pr->rid);
	const char *parregfmts[] = {
		"",
		"  - %4s = %-10u (0x%08X,0x%08X) - (%s)",
		"    - %4s = %-8u (0x%08X,0x%08X) - (%s)",
		"      - %4s = %-6u (0x%08X,0x%08X) - (%s)",
	};
	if (preg==NULL) return ERROR_FAIL;
	if (leon_read_register(tgt, pr->rid, 0)!=ERROR_OK) return ERROR_FAIL;
	command_print(CMD_CTX, "--- %s (0x%08X) - %s", pr->name, *preg, pr->longname);
	const leon_reg_item_t *pi = pr->items;
	while (pi && pi->level) {
		command_print(CMD_CTX, parregfmts[pi->level<4 ? pi->level : 3],
		                       pi->sname, (*preg & pi->mask) >> pi->shift,
		                       *preg & pi->mask, pi->mask, pi->lname);
		pi++;
	}
	return ERROR_OK;
}

COMMAND_HANDLER(leon_handle_parsereg_command)
{
	struct target *tgt = leon_cmd_to_tgt(cmd);
	struct leon_common *leon;
	int rid = -1;
	uint32_t *preg;

	if (tgt==NULL) {
		LOG_ERROR("Target type is not '" LEON_TYPE_NAME "'");
		return ERROR_FAIL;
	}
	leon = target_to_leon(tgt);

	if (CMD_ARGC > 1) {
		return ERROR_COMMAND_SYNTAX_ERROR;
	}

	if (CMD_ARGC==1) {
		unsigned i;
		unsigned la = strlen(CMD_ARGV[0]);
		for (i=0;i<leon->regdesc->num_regs;++i) {
			if (strlen(leon->regdesc->reg_list[i].name)==la &&
			    !strcmp(leon->regdesc->reg_list[i].name, CMD_ARGV[0])) {
				rid = leon->rdi_to_rid[leon->regdesc->reg_list[i].number];
				break;
			}
		}
		if (rid<0) {
			LOG_ERROR("Unknown register %s", CMD_ARGV[0]);
			return ERROR_FAIL;
		}
	}
/* parse registers */
	if (rid<0) { /* show all known registers */
		const leon_reg_parsed_t *pr = leon_register_description;
		while (pr && pr->items) {
			if (leon_disp_parsed_register(cmd, pr) != ERROR_OK)
				return ERROR_FAIL;
			pr++;
		}
	} else {
		const leon_reg_parsed_t *pr = leon_register_description;
		while (pr && pr->items) {
			if (pr->rid == (unsigned)rid) { /* show known register */
				return leon_disp_parsed_register(cmd, pr);
			}
			pr++;
		}
		// unsupported reg - show its value only
		preg = leon_get_ptrreg(tgt, rid);
		if (preg) {
			int rc = leon_read_register(tgt, rid, 0);
			command_print(CMD_CTX, "--- Register '%s' = 0x%08X", CMD_ARGV[0], *preg);
			return rc;
		}
	}

	return ERROR_OK;
}

COMMAND_HANDLER(leon_handle_tmode_command)
{
	struct target *tgt = leon_cmd_to_tgt(cmd);
	struct leon_common *leon;

	if (tgt==NULL) {
		LOG_ERROR("Target type is not '" LEON_TYPE_NAME "'");
		return ERROR_FAIL;
	}
	leon = target_to_leon(tgt);

	if (CMD_ARGC > 1) {
		return ERROR_COMMAND_SYNTAX_ERROR;
	}

	if (CMD_ARGC == 1) {
		int rc = ERROR_FAIL;
		if (tgt->state != TARGET_HALTED) {
			LOG_ERROR("Target not halted");
			return ERROR_TARGET_NOT_HALTED;
		}
		uint32_t *pdsuc = leon_get_ptrreg(tgt, LEON_RID_DSUCTRL);
		uint32_t *ptrcc = leon_get_ptrreg(tgt, LEON_RID_TRCCTRL);
		if (leon_read_register(tgt, LEON_RID_DSUCTRL, 0)!=ERROR_OK ||
		    leon_read_register(tgt, LEON_RID_TRCCTRL, 0)!=ERROR_OK)
			return ERROR_FAIL;

		if (!strcmp(CMD_ARGV[0], "none")) {
			leon->trctype = LEON_TRCTYPE_NONE;
			rc = leon_write_register(tgt, LEON_RID_TRCCTRL, *ptrcc & ~(LEON_DSU_TRCTRL_TI | LEON_DSU_TRCTRL_TA));
			rc = leon_write_register(tgt, LEON_RID_DSUCTRL, *pdsuc & ~LEON_DSU_CTRL_TRACE_EN);
		} else if (!strcmp(CMD_ARGV[0], "proc")) {
			leon->trctype = LEON_TRCTYPE_CPU;
			rc = leon_write_register(tgt, LEON_RID_TRCCTRL, (*ptrcc | LEON_DSU_TRCTRL_TI) & ~(LEON_DSU_TRCTRL_TA | LEON_DSU_TRCTRL_AHB_IDX_MASK | LEON_DSU_TRCTRL_INST_IDX_MASK));
			rc |= leon_write_register(tgt, LEON_RID_DSUCTRL, *pdsuc | LEON_DSU_CTRL_TRACE_EN);
		} else if (!strcmp(CMD_ARGV[0], "ahb")) {
			leon->trctype = LEON_TRCTYPE_AHB;
			rc = leon_write_register(tgt, LEON_RID_TRCCTRL, (*ptrcc | LEON_DSU_TRCTRL_TA) & ~(LEON_DSU_TRCTRL_TI | LEON_DSU_TRCTRL_AHB_IDX_MASK | LEON_DSU_TRCTRL_INST_IDX_MASK));
			rc |= leon_write_register(tgt, LEON_RID_DSUCTRL, *pdsuc | LEON_DSU_CTRL_TRACE_EN);
		} else if (!strcmp(CMD_ARGV[0], "both")) {
			leon->trctype = LEON_TRCTYPE_BOTH;
			rc = leon_write_register(tgt, LEON_RID_TRCCTRL, (*ptrcc | LEON_DSU_TRCTRL_TI | LEON_DSU_TRCTRL_TA) & ~(LEON_DSU_TRCTRL_AHB_IDX_MASK | LEON_DSU_TRCTRL_INST_IDX_MASK));
			rc |= leon_write_register(tgt, LEON_RID_DSUCTRL, *pdsuc | LEON_DSU_CTRL_TRACE_EN);
		} else {
			return ERROR_COMMAND_SYNTAX_ERROR;
		}
	}

	command_print(CMD_CTX, "Trace mode = %s", leon_trctype_to_string(leon->trctype));

	return ERROR_OK;
}

COMMAND_HANDLER(leon_handle_hist_command)
{
	int n = 10, rc;
	struct target *tgt = leon_cmd_to_tgt(cmd);
	struct leon_common *leon;
	uint32_t tbuf[4];

	if (tgt==NULL) {
		LOG_ERROR("Target type is not '" LEON_TYPE_NAME "'");
		return ERROR_FAIL;
	}
	leon = target_to_leon(tgt);
	if (CMD_ARGC > 1) return ERROR_COMMAND_SYNTAX_ERROR;
	if (CMD_ARGC == 1) n = atoi(CMD_ARGV[0]);

	uint32_t *pdsuc = leon_get_ptrreg(tgt, LEON_RID_DSUCTRL);
	uint32_t *ptrcc = leon_get_ptrreg(tgt, LEON_RID_TRCCTRL);
	if (!pdsuc || !ptrcc) return ERROR_FAIL;
	if (leon_read_register(tgt, LEON_RID_DSUCTRL, 1)!=ERROR_OK) return ERROR_FAIL;
	if (leon_read_register(tgt, LEON_RID_TRCCTRL, 1)!=ERROR_OK) return ERROR_FAIL;
	leon_update_tmode(leon, *pdsuc, *ptrcc);
	if (leon->trctype==LEON_TRCTYPE_BOTH) {
		command_print(CMD_CTX, "--- Mixed mode trace NI ---");
	} else {
		if (leon->trctype==LEON_TRCTYPE_AHB) {
			rc = LEON_GET_VAL(ptrcc, LEON_DSU_TRCTRL_AHB_IDX);
		} else {
			rc = LEON_GET_VAL(ptrcc, LEON_DSU_TRCTRL_INST_IDX);
		}
		if (rc<n) n=rc;
		print_trace(CMD_CTX, tgt, leon->trctype, NULL);
		while (n>0) {
			uint32_t addr = LEON_DSU_TRACE_BUFFER_BASE+(rc-n)*16;
//	printf("hist addr = 0x%08X\n", addr);
			if (leon_jtag_get_registers(tgt, addr, tbuf, 4) != ERROR_OK) break;
			print_trace(CMD_CTX, tgt, leon->trctype, tbuf);
			n--;
		}
	}
	return ERROR_OK;
}

COMMAND_HANDLER(leon_handle_ahb_command)
{
	return ERROR_FAIL;
}
COMMAND_HANDLER(leon_handle_inst_command)
{
	return ERROR_FAIL;
}


COMMAND_HANDLER(leon_handle_disas_command)
{
	uint32_t addr = 0xffffffff, opcode, n = 1;
	struct target *tgt = leon_cmd_to_tgt(cmd);
	struct leon_common *leon;

//	uint32_t curctlbits = 0;
//	int rdctlbits; /* pre-read control bits for the current block */

	if (tgt==NULL) {
		LOG_ERROR("Target type is not '" LEON_TYPE_NAME "'");
		return ERROR_FAIL;
	}
	leon = target_to_leon(tgt);
	if (CMD_ARGC > 2) return ERROR_COMMAND_SYNTAX_ERROR;
	if (CMD_ARGC > 0) {
		if (*(CMD_ARGV[0])>='0' && *(CMD_ARGV[0])<='9') {
			addr = strtoul(CMD_ARGV[0], NULL, 0);
		} else if (*(CMD_ARGV[0])=='%') { // value from register name
			int rid = leon_reg_name2rid(tgt, "iu", CMD_ARGV[0]+1);
			if (rid<0) {
				LOG_ERROR("Unknown IU register '%s'", CMD_ARGV[0]+1);
				return ERROR_FAIL;
			}
			if (leon_read_register(tgt, rid, 0)!=ERROR_OK) {
				LOG_ERROR("Read register '%s' failed", CMD_ARGV[0]+1);
				return ERROR_FAIL;
			}
			uint32_t *prv = leon_get_ptrreg(tgt, rid);
			addr = *prv;
			//LOG_INFO(" >> rid=%d => addr=0x%08X", rid, addr);
		} else {
			leon_elf_symbol_t *ps = leon_elf_sym2val(leon, ".text", CMD_ARGV[0], &addr);
			if (ps==NULL) {
				LOG_ERROR("Unknown symbol '%s'", CMD_ARGV[0]);
				return ERROR_FAIL;
			}
		}
	}
	if (CMD_ARGC > 1) {
		n = strtoul(CMD_ARGV[1], NULL, 0);
	}
	if (addr == 0xffffffff) { /* get addr from PC */
		uint32_t *ppc = leon_get_ptrreg(tgt, LEON_RID_PC);
		if (leon_read_register(tgt, LEON_RID_PC, 1)!=ERROR_OK) return ERROR_FAIL;
		addr = *ppc;
	}

//	if (leon->ltype==LEON_TYPE_L2MT && leon->mt_ctlblk_size>0)
//		rdctlbits = 1;

	while(n>0) {
		if (leon_jtag_get_registers(tgt, addr, &opcode, 1)!=ERROR_OK) {
			break;
		}

		char *symaddr = leon_elf_val2sym(leon, ".text", addr);
		if (symaddr)
			command_print(CMD_CTX, "  %s:", symaddr);
		int mtdas = 0;
		const char *das = leon_disas(tgt, addr, opcode, &mtdas);
		command_print(CMD_CTX, "%08X  %08X %s %-30s", addr, opcode,
		              mtdas ? "S" : " ", das);
		addr += 4;
		n--;
	}

	return ERROR_OK;
}

COMMAND_HANDLER(leon_mem_command)
{
	uint32_t addr = 0xffffffff, size = 1;
	struct target *tgt = leon_cmd_to_tgt(cmd);
	struct leon_common *leon;

	if (tgt==NULL) {
		LOG_ERROR("Target type is not '" LEON_TYPE_NAME "'");
		return ERROR_FAIL;
	}
	leon = target_to_leon(tgt);
	if (CMD_ARGC > 2) return ERROR_COMMAND_SYNTAX_ERROR;
	if (CMD_ARGC > 0) {
		if (*(CMD_ARGV[0])>='0' && *(CMD_ARGV[0])<='9') { // direct value
			addr = strtoul(CMD_ARGV[0], NULL, 0);
		} else if (*(CMD_ARGV[0])=='%') { // value from register name
			int rid = leon_reg_name2rid(tgt, "iu", CMD_ARGV[0]+1);
			if (rid<0) {
				LOG_ERROR("Unknown IU register '%s'", CMD_ARGV[0]+1);
				return ERROR_FAIL;
			}
			if (leon_read_register(tgt, rid, 0)!=ERROR_OK) {
				LOG_ERROR("Read register '%s' failed", CMD_ARGV[0]+1);
				return ERROR_FAIL;
			}
			uint32_t *prv = leon_get_ptrreg(tgt, rid);
			addr = *prv;
			//LOG_INFO(" >> rid=%d => addr=0x%08X", rid, addr);
		} else { // value from symbol
			leon_elf_symbol_t *ps = leon_elf_sym2val(leon, NULL, CMD_ARGV[0], &addr);
			if (ps) {
				size = (ps->size + 3) / 4; /* display the whole words */
			} else {
				LOG_ERROR("Unknown symbol '%s'", CMD_ARGV[0]);
				return ERROR_FAIL;
			}
		}

		if (CMD_ARGC > 1) {
			size = strtoul(CMD_ARGV[1], NULL, 0);
		}
	} else { /* get addr from PC */
		return ERROR_COMMAND_SYNTAX_ERROR;
	}

	//command_print(CMD_CTX,"%X %x\n", addr, size);
	while (size) {
		uint32_t buf[4];
		char strbuf[4][10];
		char chbuf[17] = "................";
		const char *spacebuf = "        ";
		int cnt = (size>4) ? 4 : size;

		int rc = leon_read_memory(tgt, addr, 4, cnt, (uint8_t *)buf);
		if (rc!=ERROR_OK) {
			command_print(CMD_CTX, "! Read from target memory at 0x%08X failed (%d)",
			              addr, rc);
			return ERROR_FAIL;
		}
		snprintf(&strbuf[0][0], 10, "%08X", leon_swap_u32(buf[0]));
		if (cnt>1) snprintf(&strbuf[1][0], 10, "%08X", leon_swap_u32(buf[1]));
		if (cnt>2) snprintf(&strbuf[2][0], 10, "%08X", leon_swap_u32(buf[2]));
		if (cnt>3) snprintf(&strbuf[3][0], 10, "%08X", leon_swap_u32(buf[3]));
		for(int i = 0; i<4*cnt;++i) {
			uint8_t c = (buf[i>>2] >> (8*(i&3))) & 0xff;
printf("%d : %08X %d(%c)\n", i, buf[i>>2], c, c);
			if (c>=' ' && c<127) chbuf[i]=c;
		}
		command_print(CMD_CTX, "%08X   %s  %s  %s  %s    %s", addr, strbuf[0],
		            (cnt>1) ? strbuf[1] : spacebuf, (cnt>2) ? strbuf[2] : spacebuf,
		            (cnt>3) ? strbuf[3] : spacebuf, chbuf);
		addr += cnt*4;
		size -= cnt;
	}

	return ERROR_OK;
}

/* -------------------------------------------------------------------------- */
COMMAND_HANDLER(leon_symbols_command)
{
	int rc = 0;
	struct target *tgt = leon_cmd_to_tgt(cmd);
	struct leon_common *leon;

	if (tgt==NULL) {
		LOG_ERROR("Target type is not '" LEON_TYPE_NAME "'");
		return ERROR_FAIL;
	}
	leon = target_to_leon(tgt);

	if (CMD_ARGC>0) {
		struct image image;
		memset(&image, 0, sizeof(struct image));
		if (image_open(&image, CMD_ARGV[0], "elf") != ERROR_OK) {
			LOG_ERROR("Load ELF file with symbols failed. Try common command 'load_image'.");
			return ERROR_OK;
		}
		rc = leon_load_elf_symbols(leon, &image);

		if ((rc==ERROR_OK) && (leon->lelf_nsyms>0))
			command_print(CMD_CTX, " read %u symbols", leon->lelf_nsyms);
		image_close(&image);
	} else {
		leon_elf_symbol_t *ps = leon->lelf_symbs;
		while (ps) {
			if (ps->name && *ps->name) {
				char sbuf[4] = {0,0,0,0};
				switch (ELF32_ST_BIND(ps->info)) {
					case STB_LOCAL:  sbuf[0]='l'; break;
					case STB_GLOBAL: sbuf[0]='g'; break;
					case STB_WEAK:   sbuf[0]='w'; break;
					case STB_GNU_UNIQUE: sbuf[0]='u'; break;
					case STB_NUM:    sbuf[0]='!'; break;
					default: sbuf[0]=' '; break;
				}
				switch (ELF32_ST_TYPE(ps->info)) {
					case STT_OBJECT:  sbuf[1]='O'; break;
					case STT_FUNC:    sbuf[1]='F'; break;
					case STT_SECTION: sbuf[1]='s'; break;
					case STT_FILE:    sbuf[1]='f'; break;
					case STT_COMMON:  sbuf[1]='C'; break;
					case STT_TLS:     sbuf[1]='T'; break;
					//case STT_GNU_IFUNC: sbuf[1]=' '; break;
					case STT_NOTYPE:
					default:          sbuf[1]=' '; break;
				}
				//command_print(CMD_CTX, "0x%08X %x,%x %s", ps->value, ps->info, ps->other, ps->name);
				command_print(CMD_CTX, "0x%08X %s %s", ps->value, sbuf, ps->name);
			}
			ps = ps->next;
		}
	}
	return rc;
}

COMMAND_HANDLER(leon_load_elf_command)
{
	uint8_t *buffer;
	size_t buf_cnt;
	uint32_t image_size;
	uint32_t min_address = 0;
	uint32_t max_address = 0xffffffff;
	int i;
	int retval;
	struct image image;

	struct target *tgt = leon_cmd_to_tgt(cmd);
	struct leon_common *leon;

	if (tgt==NULL) {
		LOG_ERROR("Target type is not '" LEON_TYPE_NAME "'");
		return ERROR_FAIL;
	}
	leon = target_to_leon(tgt);

	LEON_TM_START(bench);

	memset(&image, 0, sizeof(struct image));
	if (image_open(&image, CMD_ARGV[0], "elf") != ERROR_OK) {
		LOG_ERROR("Load ELF file with symbols failed. Try common command 'load_image'.");
		return ERROR_OK;
	}

	retval = leon_load_elf_symbols(leon, &image);

	image_size = 0x0;
	retval = ERROR_OK;
	for (i = 0; i < image.num_sections; i++) {
		buffer = malloc(image.sections[i].size);
		if (buffer == NULL) {
			command_print(CMD_CTX,
						  "error allocating buffer for section (%d bytes)",
						  (int)(image.sections[i].size));
			break;
		}

		retval = image_read_section(&image, i, 0x0, image.sections[i].size, buffer, &buf_cnt);
		if (retval != ERROR_OK) {
			free(buffer);
			break;
		}

		uint32_t offset = 0;
		uint32_t length = buf_cnt;

		/* DANGER!!! beware of unsigned comparision here!!! */

		if ((image.sections[i].base_address + buf_cnt >= min_address) &&
				(image.sections[i].base_address < max_address)) {

			if (image.sections[i].base_address < min_address) {
				/* clip addresses below */
				offset += min_address-image.sections[i].base_address;
				length -= offset;
			}

			if (image.sections[i].base_address + buf_cnt > max_address)
				length -= (image.sections[i].base_address + buf_cnt)-max_address;
			retval = target_write_buffer(tgt,
					image.sections[i].base_address + offset, length, buffer + offset);
			if (retval != ERROR_OK) {
				free(buffer);
				break;
			}
			image_size += length;
//			command_print(CMD_CTX, "%u bytes written at address 0x%8.8" PRIx32 "",
//					(unsigned int)length,
//					image.sections[i].base_address + offset);
		}
		free(buffer);
	}

	uint32_t tmtime;
	LEON_TM_MEASURE(bench, tmtime);
	if ((retval==ERROR_OK) && (tmtime>0)) {
		command_print(CMD_CTX, " total size: %" PRIu32 " bytes (%0.3f KiB/s)",
		              image_size, duration_kbps(&bench, image_size));
		if (leon->lelf_nsyms)
			command_print(CMD_CTX, " read %u symbols", leon->lelf_nsyms);
		if (leon->lelf_epoint!=0xffffffff)
			command_print(CMD_CTX, " entry point: 0x%08X", leon->lelf_epoint);
	}

	image_close(&image);

	return retval;
}

COMMAND_HANDLER(leon_run_command)
{
	uint32_t addr = 0, cur = 1;
	struct target *tgt = leon_cmd_to_tgt(cmd);
	struct leon_common *leon;

	if (tgt==NULL) {
		LOG_ERROR("Target type is not '" LEON_TYPE_NAME "'");
		return ERROR_FAIL;
	}
	leon = target_to_leon(tgt);
	if (CMD_ARGC > 1) return ERROR_COMMAND_SYNTAX_ERROR;
	if (CMD_ARGC > 0) {
		if (*(CMD_ARGV[0])>='0' && *(CMD_ARGV[0])<='9') {
			addr = strtoul(CMD_ARGV[0], NULL, 0);
			cur = 0;
		} else if (*(CMD_ARGV[0])=='%') { // value from register name
			int rid = leon_reg_name2rid(tgt, "iu", CMD_ARGV[0]+1);
			if (rid<0) {
				LOG_ERROR("Unknown IU register '%s'", CMD_ARGV[0]+1);
				return ERROR_FAIL;
			}
			if (leon_read_register(tgt, rid, 0)!=ERROR_OK) {
				LOG_ERROR("Read register '%s' failed", CMD_ARGV[0]+1);
				return ERROR_FAIL;
			}
			uint32_t *prv = leon_get_ptrreg(tgt, rid);
			addr = *prv;
			//LOG_INFO(" >> rid=%d => addr=0x%08X", rid, addr);
			cur = 0;
		} else {
			leon_elf_symbol_t *ps = leon_elf_sym2val(leon, ".text", CMD_ARGV[0], &addr);
			if (ps==NULL) {
				LOG_ERROR("Unknown symbol '%s'", CMD_ARGV[0]);
				return ERROR_FAIL;
			}
			cur = 0;
		}
	}
	if (cur) { /* get addr from PC */
		uint32_t *ppc = leon_get_ptrreg(tgt, LEON_RID_PC);
		if (leon_read_register(tgt, LEON_RID_PC, 1)!=ERROR_OK) return ERROR_FAIL;
		addr = *ppc;
	}

	return target_resume(tgt, cur, addr, 1, 0);
}

COMMAND_HANDLER(leon_restart_command)
{
	struct target *tgt = leon_cmd_to_tgt(cmd);
	struct leon_common *leon;
	int halt = 0, retval;

	if (tgt==NULL) {
		LOG_ERROR("Target type is not '" LEON_TYPE_NAME "'");
		return ERROR_FAIL;
	}
	leon = target_to_leon(tgt);
	if (leon->ltype!=LEON_TYPE_L2MT) {
		LOG_ERROR("Target has to be L2MT");
		return ERROR_FAIL;
	}

	if (CMD_ARGC > 1) return ERROR_COMMAND_SYNTAX_ERROR;
	if (CMD_ARGC > 0) {
		if (!strcmp(CMD_ARGV[0], "halt"))
			halt = 1;
	}
	do {
		uint32_t *pdsu = leon_get_ptrreg(tgt, LEON_RID_DSUMTCTRL);
		retval = leon_write_register(tgt, LEON_RID_DSUMTCTRL, LEON_DSU_MTCTRL_RST | (halt ? LEON_DSU_MTCTRL_BRK : 0));
		if (retval!=ERROR_OK) break;
		retval = leon_read_register(tgt, LEON_RID_DSUMTCTRL, 1);
		if (retval!=ERROR_OK) break;
		if (!(*pdsu & LEON_DSU_MTCTRL_RST)) {
			LOG_ERROR("SW Reset failed");
			return ERROR_FAIL;
		}
		usleep(1);
		if (!halt) {
			retval = leon_write_register(tgt, LEON_RID_DSUCTRL, 0); // for safety, reset breaks in DSU control registers
		} else {
			retval = leon_write_register(tgt, LEON_RID_DSUCTRL, LEON_DSU_CTRL_BRK_NOW); // for safety, reset breaks in DSU control registers
		}
		if (retval!=ERROR_OK) break;
		retval = leon_write_register(tgt, LEON_RID_DSUMTCTRL, halt ? LEON_DSU_MTCTRL_BRK : 0);
		if (retval!=ERROR_OK) break;
		retval = leon_read_register(tgt, LEON_RID_DSUMTCTRL, 1);
		if (retval!=ERROR_OK) break;
		if (*pdsu & LEON_DSU_MTCTRL_RST) {
			LOG_ERROR("SW Reset failed (2)");
			return ERROR_FAIL;
		}
		// set register dsumtctrl as dirty to force re-read from hardware target
		leon_invalidate_register(tgt, LEON_RID_DSUMTCTRL);
		return ERROR_OK;
	} while(0);
	return ERROR_FAIL;
}

/* ========================================================================== */

static const struct command_registration leon_command_handlers[] = {
	{
		.name = "loptime",
		.handler = leon_handle_loptime_command,
		.mode = COMMAND_EXEC,
		.help = "get processing time [ms] of the last LEON operation",
		.usage = NULL,
	},
	{
		.name = "hwinfo",
		.handler = leon_handle_hwinfo_command,
		.mode = COMMAND_EXEC,
		.help = "get and print HW information about LEON target",
		.usage = NULL,
	},
	{
		.name = "leontype",
		.handler = leon_handle_leontype_command,
		.mode = COMMAND_CONFIG,
		.help = "With an argument, set type of LEON processor by hand (can be dangerous)."
		        "The second argument for 'l2mt' processor sets size of MT control block in bytes."
		        "With or without argument, display current setting.",
		.usage = "[unknown | l2 | l2ft | l2mt] [mtblk]",
	},
	{
		.name = "preg",
		.handler = leon_handle_parsereg_command,
		.mode = COMMAND_EXEC,
		.help = "With argument, name of register, get and parse this register."
		        "Without argument, parse all non-trivial registers.",
		.usage = NULL,
	},

/* --- grlib compatible commands --- */
	{
		.name = "tmode",
		.handler = leon_handle_tmode_command,
		.mode = COMMAND_EXEC,
		.help = "Select tracing mode between none, processor-only, AHB only or both.",
		.usage = "[none | proc | ahb | both]",
	},

	{
		.name = "hist",
		.handler = leon_handle_hist_command,
		.mode = COMMAND_EXEC,
		.help = "Display trace history (mixed mode)",
		.usage = "[number of lines]",
	},
	{
		.name = "ahb",
		.handler = leon_handle_ahb_command,
		.mode = COMMAND_EXEC,
		.help = "Display history of AHB transactions",
		.usage = "[number of lines]",
	},
	{
		.name = "inst",
		.handler = leon_handle_inst_command,
		.mode = COMMAND_EXEC,
		.help = "Display trace history of CPU instructions",
		.usage = "[number of lines]",
	},
	{
		.name = "disas",
		.handler = leon_handle_disas_command,
		.mode = COMMAND_EXEC,
		.help = "Disassemble memory (symbol can be used)",
		.usage = "<address> <length>",
	},
	{
		.name = "mem",
		.handler = leon_mem_command,
		.mode = COMMAND_EXEC,
		.help = "Display content of the memory (symbol can be used)",
		.usage = "<address> <length> or <symbol>",
	},

//	{
//		.name = "wmem",
//		.handler = leon_handle_parsereg_command,
//		.mode = COMMAND_EXEC,
//		/* prefer using less error-prone "arm mcr" or "arm mrc" */
//		.help = "With argument, name of register, get and parse this register."
//		        "Without argument, parse all non-trivial registers.",
//		.usage = NULL,
//	},

	{
		.name = "load",
		.handler = leon_load_elf_command,
		.mode = COMMAND_EXEC,
		.help = "Load ELF file with symbols",
		.usage = "<filename>",
	},

	{
		.name = "symbols",
		.handler = leon_symbols_command,
		.mode = COMMAND_EXEC,
		.help = "With argument, load symbols from alternate ELF file."
		        "Without argument, display all symbols.",
		.usage = "<filename>",
	},

	{
		.name = "run",
		.handler = leon_run_command,
		.mode = COMMAND_EXEC,
		.help = "Run (resume) CPU from the current PC or address",
		.usage = "[address]",
	},
/* --- */

	{
		.name = "restart",
		.handler = leon_restart_command,
		.mode = COMMAND_EXEC,
		.help = "Restart L2MT target",
		.usage = "[halt]",
	},

	COMMAND_REGISTRATION_DONE
};

/* -------------------------------------------------------------------------- */
/* --- Target structure and functions --------------------------------------- */
struct target_type leon_target = {
	.name = LEON_TYPE_NAME,

	.target_create = leon_target_create,
	.init_target = leon_init_target,
	.deinit_target = leon_deinit_target,
	.examine = leon_examine,
	.commands = leon_command_handlers,

	.poll = leon_poll,
	.arch_state = leon_arch_state,

	.halt = leon_halt,
	.resume = leon_resume,
	.step = leon_step,

//	.assert_reset = leon_assert_reset,
//	.deassert_reset = leon_deassert_reset,

	.get_gdb_reg_list = leon_get_gdb_reg_list,

	.read_memory = leon_read_memory,
	.write_memory = leon_write_memory,
	.checksum_memory = leon_checksum_memory,
	.blank_check_memory = leon_blank_check_memory,

	.run_algorithm = leon_run_algorithm,

	.add_breakpoint = leon_add_breakpoint,
	.remove_breakpoint = leon_remove_breakpoint,
	.add_watchpoint = leon_add_watchpoint,
	.remove_watchpoint = leon_remove_watchpoint,
	.soft_reset_halt = leon_soft_reset_halt,
};


