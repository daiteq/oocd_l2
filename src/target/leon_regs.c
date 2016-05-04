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
//static const leon_registers_t *leon_find_lreg(const leon_registers_t *lrgrp, unsigned rid)
//{
//	while (lrgrp && lrgrp->name) {
//		if (lrgrp->rid == rid) return lrgrp;
//		lrgrp++;
//  }
//	return NULL;
//}

static int leon_get_core_reg(struct reg *reg);
static int leon_set_core_reg(struct reg *reg, uint8_t *buf);

static const struct reg_arch_type leon_reg_type = {
	.get = leon_get_core_reg,
	.set = leon_set_core_reg,
};

static void leon_set_one_reg_cache(struct target *target, int rdi, const leon_registers_t *lr, enum leon_register_groups lgrp)
{
	struct leon_common *leon = target_to_leon(target);
	struct reg *r = leon->regdesc->reg_list + rdi;
	if (lr==NULL) return;
	r->name = lr->name;
//LOG_USER("--- Add reg %d '%s'", rdi, lr->name);
	r->number = rdi; // lr->rid;
	r->size = 32;
	r->value = &leon->regvals[lr->rid];
	r->type = &leon_reg_type;
	r->arch_info = target;
	r->exist = true;
	r->valid = 0;
	r->dirty = 0;
	r->reg_data_type = malloc(sizeof(struct reg_data_type));
	r->reg_data_type->type = lr->type;
	/* This really depends on the calling convention in use */
	r->caller_save = false;
	/* let GDB shows banked registers only in "info all-reg" */
	r->feature = malloc(sizeof(struct reg_feature));
	r->feature->name = leon_rgrp_names[lgrp].feature_name;
	r->group = leon_rgrp_names[lgrp].name;

	leon->rdi_to_rid[rdi] = lr->rid;
	leon->rid_to_rdi[lr->rid] = rdi;
	leon->rdi2hwaddr[rdi] = lr->maddr;
}


struct reg_cache *leon_build_reg_cache(struct target *target)
{
	struct reg_cache **cache_p = register_get_last_cache_p(&target->reg_cache);
	struct leon_common *leon = target_to_leon(target);
	
	struct reg_cache *cache = malloc(sizeof(struct reg_cache));
	struct reg *reg_list = calloc(LEON_RID_ALLSIZE, sizeof(struct reg)); // allocate space for maximum of available registers
	int i, cnt = 0;

	if (!cache || !reg_list) {
		free(cache);
		free(reg_list);
		return NULL;
	}
	/* reset for all unused/unavailable registers */
	for (i=0;i<LEON_RID_ALLSIZE; ++i) {
		leon->rid_to_rdi[i] = -1;
		leon->rdi_to_rid[i] = -1;
		leon->rdi2hwaddr[i] = LEON_MADDR_UNDEFINED;
	}

	cache->name = "LEON2MT registers";
	cache->next = NULL;
	cache->reg_list = reg_list;

	leon->regdesc = cache;
	(*cache_p) = cache;

	/* first we need LCFG */
//	leon_set_one_reg_cache(target, cnt++, leon_find_lreg(leon_peri_regs, LEON_RID_LCFG), LEON_RGRP_PERI);

	/* then we add IU registers */
	const leon_registers_t *plr = &leon_iu_regs[0];
	while (plr->name) {
		leon_set_one_reg_cache(target, cnt++, plr++, LEON_RGRP_IU);
	}
	/* then we add FPU registers if necessary */
/* we cannot use jtag operations at this moment */
	//uint32_t *plcfg = leon_get_ptrreg(target, LEON_RID_LCFG);
	//int retval = leon_read_register(target, LEON_RID_LCFG, 1);
	//int retval = leon_jtag_get_registers(target, LEON_REGS_LEON_CFG_REG, plcfg, 1);
	//if (retval==ERROR_OK && LEON_GET_VAL(plcfg, LEON_CFG_REG_FPU_TYPE)) {
		plr = &leon_fpu_regs[0];
		while (plr->name) {
			leon_set_one_reg_cache(target, cnt++, plr++, LEON_RGRP_FPU);
		}
	//}
	/* then we add CU registers */
	/* then we add DSU registers */
	//if (retval==ERROR_OK && LEON_GET_BIT(plcfg, LEON_CFG_REG_IS_DSU)) {
		plr = &leon_dsu_regs[0];
		while (plr->name) {
			leon_set_one_reg_cache(target, cnt++, plr++, LEON_RGRP_DSU);
		}
	//}
	/* and then, we add other PERIpheral registers */
		plr = &leon_peri_regs[0];
		while (plr->name) {
//			if (plr->rid!=LEON_RID_LCFG) {
				leon_set_one_reg_cache(target, cnt++, plr, LEON_RGRP_PERI);
//			}
			plr++;
		}

	/* in the end, we add shadow registers */
//LOG_USER("-- Added %d regs", cnt);
	cache->num_regs = cnt;
	return cache;
}


int leon_check_regcache(struct target *target)
{
	struct leon_common *leon = target_to_leon(target);
	struct reg *r;
	unsigned int i, retval;
	bool ex;

	retval = leon_read_register(target, LEON_RID_LCFG, 0);
	if (retval!=ERROR_OK) return retval;
	uint32_t *plcfg = leon_get_ptrreg(target, LEON_RID_LCFG);
	if (!plcfg) return ERROR_FAIL;
	/* check FPU */
	if (LEON_GET_VAL(plcfg, LEON_CFG_REG_FPU_TYPE) != LEON_CFG_FPU_TYPE_NONE)
		ex = true;
	else
		ex = false;
	r = leon->regdesc->reg_list;
	for (i = 0; i < leon->regdesc->num_regs; ++i) {
		if (!strcmp(r->group, leon_rgrp_names[LEON_RGRP_FPU].name)) {
			r->exist = ex;
		}
		r++;
	}

	/* check DSU */
	if (LEON_GET_BIT(plcfg, LEON_CFG_REG_IS_DSU))
		ex = true;
	else
		ex = false;
	r = leon->regdesc->reg_list;
	for (i = 0; i < leon->regdesc->num_regs; ++i) {
		if (!strcmp(r->group, leon_rgrp_names[LEON_RGRP_DSU].name)) {
			r->exist = ex;
		}
		r++;
	}

	return ERROR_OK;
}

/* -------------------------------------------------------------------------- */
int leon_get_all_reg_list(struct target *target,
                          struct reg **reg_list[], int *reg_list_size)
{
	struct leon_common *leon = target_to_leon(target);
	struct reg_cache *rc = leon->regdesc;
	unsigned int i, j = 0;

//	for (i = 0; i < rc->num_regs; ++i) {
//		if (rc->reg_list[i].exist) j++;
//	}
	j = rc->num_regs;

	*reg_list_size = j;
	*reg_list = malloc(sizeof(struct reg *) * j);

//	j = 0;
	for (i = 0; i < rc->num_regs; ++i) {
//		if (rc->reg_list[i].exist) {
//			(*reg_list)[j] = rc->reg_list + i;
//			j++;
			(*reg_list)[i] = rc->reg_list + i;
//		}
	}

	return ERROR_OK;
}

int leon_get_general_reg_list(struct target *target,
                              struct reg **reg_list[], int *reg_list_size)
{
	struct leon_common *leon = target_to_leon(target);
	int i;

	*reg_list_size = 35; /* r0-r31, pc, npc, psr */
	*reg_list = malloc(sizeof(struct reg *) * (*reg_list_size));

	for (i = 0; i < 32; ++i) {
		(*reg_list)[i] = &leon->regdesc->reg_list[leon->rid_to_rdi[LEON_RID_R0+i]];
	}
	(*reg_list)[32] = &leon->regdesc->reg_list[leon->rid_to_rdi[LEON_RID_PC]];
	(*reg_list)[33] = &leon->regdesc->reg_list[leon->rid_to_rdi[LEON_RID_NPC]];
	(*reg_list)[34] = &leon->regdesc->reg_list[leon->rid_to_rdi[LEON_RID_PSR]];

	return ERROR_OK;
}

/* -------------------------------------------------------------------------- */
static uint32_t leon_get_addr_from_rid(struct target *tgt, uint32_t rid)
{
	struct leon_common *leon = target_to_leon(tgt);
	uint32_t outval;
	uint32_t *plcfg, *ppsr;
	int nwp, cwp, fputp, i, rofs;
	int retval;

	retval = leon->rid_to_rdi[rid];
	if (retval<0) {
		return LEON_MADDR_DYNAMIC;
	}
	outval = leon->rdi2hwaddr[retval];
	if (outval==LEON_MADDR_UNDEFINED) return LEON_MADDR_DYNAMIC;
	if (outval!=LEON_MADDR_DYNAMIC) return outval;

	// read LCFG for NWINDOWS, FPUtype
	retval = leon_read_register(tgt, LEON_RID_LCFG, 0);
	if (retval!=ERROR_OK) return LEON_MADDR_DYNAMIC;
	// read PSR for CWP,
	retval = leon_read_register(tgt, LEON_RID_PSR, 0);
	if (retval!=ERROR_OK) return LEON_MADDR_DYNAMIC;
	plcfg = leon_get_ptrreg(tgt, LEON_RID_LCFG);
	ppsr = leon_get_ptrreg(tgt, LEON_RID_PSR);

	nwp = LEON_GET_VAL(plcfg, LEON_CFG_REG_NWINDOWS)+1;
	cwp = LEON_GET_VAL(ppsr, SPARC_V8_PSR_CWP);
	fputp = LEON_GET_VAL(plcfg, LEON_CFG_REG_FPU_TYPE);

	if (/* rid>=LEON_RID_R0 && */ rid<=LEON_RID_R31) { // r0-r31
		i = rid - LEON_RID_R0;
		switch (leon->ltype) {
			case LEON_TYPE_L2:
			case LEON_TYPE_L2FT:
			case LEON_TYPE_L2MT: /* TODO: move to standalone case */
				if (i<8) {			// read global registers
					if (fputp) // fpu is enabled
						rofs = (nwp * 64)+128 + 4*i;
					else
						rofs = (nwp * 64) + 4*i;
//LOG_INFO("rofs @ %d (%u) = 0x%X (%d)", i, rid, rofs, nwp);
					return LEON_DSU_IU_FPU_RFILE_BASE+rofs;
				} else if (i<18) {		// read output registers
					rofs = ((cwp * 64)+32 + 4*i)%(nwp*64);
					return LEON_DSU_IU_FPU_RFILE_BASE+rofs;
				} else if (i<24) { 		// read local registers
					rofs = ((cwp * 64)+64 + 4*i)%(nwp*64);
					return LEON_DSU_IU_FPU_RFILE_BASE+rofs;
				} else {			// read input registers
					rofs = ((cwp * 64)+96+4*i)%(nwp*64);
					return LEON_DSU_IU_FPU_RFILE_BASE+rofs;
				}
				break;
			default:
				LOG_WARNING("unsupported type of Leon CPU.");
				break;
		}
	}
	if (rid>=LEON_RID_F0 && rid<=LEON_RID_F31) { // f0-f31
		i = rid - LEON_RID_F0;
		switch (fputp) {
			case LEON_CFG_FPU_TYPE_NONE:  break;
			case LEON_CFG_FPU_TYPE_MEIKO: return LEON_DSU_IU_FPU_RFILE_BASE + (nwp * 64) + 4*i; break;
			case LEON_CFG_FPU_TYPE_GRFPU: return LEON_DSU_IU_FPU_RFILE_BASE + 0x00010000 + 4*i; break;
		}
	}
	return LEON_MADDR_DYNAMIC;
}

/* -------------------------------------------------------------------------- */
static int leon_get_core_reg(struct reg *reg)
{
	int retval;
	uint32_t addr;
	struct target *tgt = reg->arch_info;
	struct leon_common *leon = target_to_leon(tgt);
	int rid = leon->rdi_to_rid[reg->number];

	if ((rid != LEON_RID_LCFG) &&
	    (reg->group[0] != leon_rgrp_names[LEON_RGRP_DSU].name[0])) { /* do not check halted target for DSU registers and LeonConfig register */
		if (tgt->state != TARGET_HALTED) {
			LOG_ERROR("Target not halted");
			return ERROR_TARGET_NOT_HALTED;
		}
	}

	addr = leon_get_addr_from_rid(tgt, rid);
//	LOG_INFO("reg addr @%u = 0x%08X", rid, addr);
	if (addr == LEON_MADDR_DYNAMIC) return ERROR_FAIL;


	retval = leon_jtag_get_registers(tgt, addr, reg->value, 1);
	if (retval==ERROR_OK) {
//LOG_INFO("rd reg @ 0x%08X = 0x%08X", addr, (uint32_t)reg->value);
		reg->valid = 1;
		reg->dirty = 0;
	}
	return retval;
}

static int leon_set_core_reg(struct reg *reg, uint8_t *buf)
{
	int retval;
	uint32_t addr;
	struct target *tgt = reg->arch_info;
	uint32_t value = buf_get_u32(buf, 0, 32);
	struct leon_common *leon = target_to_leon(tgt);
	int rid = leon->rdi_to_rid[reg->number];

	if (rid != LEON_RID_DSUCTRL) { /* do not check halted target for DSU register */
		if (tgt->state != TARGET_HALTED) {
			LOG_ERROR("Target not halted");
			return ERROR_TARGET_NOT_HALTED;
		}
	}

	addr = leon_get_addr_from_rid(tgt, rid);
	if (addr == LEON_MADDR_DYNAMIC) return ERROR_FAIL;

	*((uint32_t *)reg->value) = value;
	reg->dirty = 1;
	reg->valid = 1;

	retval = leon_jtag_set_registers(tgt, addr, reg->value, 1);
	if (retval==ERROR_OK)
		reg->dirty = 0;
	return retval;
}

/* -------------------------------------------------------------------------- */
uint32_t *leon_get_ptrreg(struct target *tgt, unsigned rid)
{
	struct leon_common *leon = target_to_leon(tgt);
	int ri = leon->rid_to_rdi[rid];
	if (ri<0) return NULL;
	if (leon->regdesc->reg_list[ri].exist==false) return NULL;
	return &leon->regvals[rid];
}

/* -------------------------------------------------------------------------- */
int leon_read_register(struct target *tgt, unsigned rid, bool force)
{
	struct leon_common *leon = target_to_leon(tgt);
	struct reg *r;
	int ri = leon->rid_to_rdi[rid];
	if (ri<0) return ERROR_FAIL; /* register not defined */
	r = leon->regdesc->reg_list + ri;
	if (r->exist && (force || r->valid==0)) {
		return leon_get_core_reg(r);
	}
	return ERROR_OK;
}

int leon_read_all_registers(struct target *tgt, bool force)
{
	struct leon_common *leon = target_to_leon(tgt);
	struct reg *r;
	unsigned i;
	int retval, rc = ERROR_OK;
	r = leon->regdesc->reg_list;
	for (i=0;i<leon->regdesc->num_regs;++i) {
		if (r->exist && (force || r->valid==0)) {
			retval = leon_get_core_reg(r);
			if (retval!=ERROR_OK) {
				rc = ERROR_FAIL; /* should be something like ERROR_PARTIALLY_FAIL */
				break;
			}
		}
		r++;
	}
	return rc;
}

int leon_write_register(struct target *tgt, unsigned rid, uint32_t value)
{
	struct leon_common *leon = target_to_leon(tgt);
	struct reg *r;
	uint8_t bval[4];

	int ri = leon->rid_to_rdi[rid];
	if (ri<0) return ERROR_FAIL; /* register not defined */
	r = leon->regdesc->reg_list + ri;
	buf_set_u32(bval, 0, 32, value);

	return leon_set_core_reg(r, bval);
}


/* -------------------------------------------------------------------------- */
int leon_reg_name2rid(struct target *tgt, const char *group, const char *name)
{
	struct leon_common *leon = target_to_leon(tgt);
	struct reg *r = leon->regdesc->reg_list;
	unsigned int i = 0;
	if (!name) return -2;
	while (i<leon->regdesc->num_regs) {
		if (!group || (group && !strcmp(group, r->group))) { // no group or required group
			if (!strcmp(name, r->name)) {
				//LOG_INFO(">>> reg: i=%u, rdi=%d, rid=%d, nm=%s", i, r->number, leon->rdi_to_rid[r->number], r->name);
				return leon->rdi_to_rid[r->number];
			}
		}
		i++;
		r++;
	}
	return -1;
}
