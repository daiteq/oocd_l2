/**
 * OOCD target for Leon (Sparc v8) CPUs
 * (C)2016 Roman Bartosinski, daiteq,s.r.o.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "leon.h"

//static uint8_t ir_ret_val[32];

int leon_jtag_set_instr(struct jtag_tap *tap, uint32_t new_instr, int forced)
{
	if (tap == NULL) return ERROR_FAIL;

	if (forced || buf_get_u32(tap->cur_instr, 0, tap->ir_length) != new_instr) {
		struct scan_field field;

		field.num_bits = tap->ir_length;
		void *t = calloc(DIV_ROUND_UP(field.num_bits, 8), 1);
		field.out_value = t;
		buf_set_u32(t, 0, field.num_bits, new_instr);
		field.in_value = NULL; // ir_ret_val;

		jtag_add_ir_scan(tap, &field, TAP_IDLE);

		free(t);
	}
	//return jtag_execute_queue();
	return ERROR_OK;
}

int leon_jtag_exchng_data(struct jtag_tap *tap, uint32_t *sdata,
                          int scnt, uint32_t *rdata, int rcnt, int rskip)
{
	struct scan_field field;
	uint8_t inval[4], outval[4];
	int retval;
	int cnt, maxcnt = (scnt>(rcnt+rskip)) ? scnt : (rcnt+rskip);
	uint32_t resp;

	field.num_bits = LEON_DRSCAN_NBITS;		// USER1 register
	field.out_value = outval;
	field.in_value = inval;

	for (cnt=0; cnt<maxcnt; ++cnt) {
		if (sdata && cnt<scnt) {
			buf_set_u32(outval, 0, LEON_DRSCAN_NBITS, sdata[cnt]);
		} else {
			buf_set_u32(outval, 0, LEON_DRSCAN_NBITS, LEON_PID_PAD | 0);
		}

// LOG_INFO("   - send 0x%02X,0x%02X,0x%02X,0x%02X.", outval[0], outval[1], outval[2], outval[3]);
		do {

			jtag_add_dr_scan(tap, 1, &field, TAP_IDLE); // TAP_DRPAUSE);
			retval = jtag_execute_queue();

	// LOG_INFO("   - returned 0x%02X,0x%02X,0x%02X,0x%02X.", inval[0], inval[1], inval[2], inval[3]);

			if (retval != ERROR_OK) {
				LOG_ERROR("LEON jtag exchange failed (%d).", cnt);
				return retval;
			}
			resp = buf_get_u32(inval, 0, LEON_DRSCAN_NBITS);
			if (((resp & LEON_PID_MASK)==LEON_PID_STATUS) && (resp & LEON_STAT_PROCESSING)) {
				LOG_INFO(" - status = x%08X.", resp);
				buf_set_u32(outval, 0, LEON_DRSCAN_NBITS, LEON_PID_PAD | 0);
				continue;
			} else
				break;
		} while (1);
		if (rdata && cnt>=rskip && cnt<(rskip+rcnt)) {
			rdata[cnt-rskip] = resp;
		} else {
//			uint32_t v = buf_get_u32(inval, 0, LEON_DRSCAN_NBITS);
// LOG_INFO(" - skipped x%08X.", v);
		}
	}

	return ERROR_OK;
}

/* -------------------------------------------------------------------------- */
int leon_jtag_get_registers(struct target *target, uint32_t addr,
                                   uint32_t *data, int cnt)
{
	int i, retval = ERROR_OK;
	struct leon_common *leon = target_to_leon(target);
	struct scan_field field;

	uint8_t outval[4];
	uint8_t inval[8];
	uint8_t *pinval = NULL; // buffer for incoming data

	/* sanitize arguments */
	if (cnt==0) return ERROR_OK;
	addr = addr & ~0x3u;
	if (cnt>LEON_BLOCK_MAX_LENGTH) {
		return ERROR_FAIL;
	}
	if (cnt>1) {
		pinval = calloc(2*cnt, 4);
		if (pinval==NULL) {
			LOG_ERROR("Unable to allocate memory buffer.");
			return ERROR_FAIL;
		}
	} else {
		pinval = inval;
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
		field.in_value = NULL;
// CONTROL WORD
		buf_set_u32(outval, 0, LEON_DRSCAN_NBITS, LEON_PID_CONTROL | LEON_CTRL_READ_DATA | LEON_CTRL_ADDR_INC | cnt);
		jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
// ADDRESS HI
		buf_set_u32(outval, 0, LEON_DRSCAN_NBITS, LEON_PID_ADDRESS | (addr>>16));
		jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
// ADDRESS LO
		buf_set_u32(outval, 0, LEON_DRSCAN_NBITS, LEON_PID_ADDRESS | (addr&0xffff));
		jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
// DATA
		buf_set_u32(outval, 0, LEON_DRSCAN_NBITS, LEON_PID_DATA | 0x0000);
		for (i=0;i<2*cnt-1;++i) {
			field.in_value = (uint8_t *) &pinval[i*4];
			jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
		}
		buf_set_u32(outval, 0, LEON_DRSCAN_NBITS, LEON_PID_PAD | 0x0000);
		field.in_value = (uint8_t *) &pinval[(2*cnt-1)*4];
		jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
// execute queue
		retval = jtag_execute_queue();
		if (retval!=ERROR_OK) {
			LOG_ERROR("Execute JTAG queue failed (%s, %d)", leon->tap->tapname, retval);
			break;
		}
// save data to output buffer
		uint32_t *pd = (uint32_t *)pinval;
		uint32_t oval;
		for (i=0;i<cnt;++i) {
			if ((*pd & LEON_PID_MASK) != LEON_PID_DATA) {
				LOG_ERROR("No returned data_hi(%d) (x%05X)", i, *pd);
				retval = ERROR_FAIL;
				break;
			}
			oval = (*pd++ & 0xffff)<<16;
			if ((*pd & LEON_PID_MASK) != LEON_PID_DATA) {
				LOG_ERROR("No returned data_lo(%d) (x%05X)", i, *pd);
				retval = ERROR_FAIL;
				break;
			}
			oval |= *pd++ & 0xffff;
			*data++ = oval;
		}
		keep_alive(); /* keep alive gdb connection */
	} while(0);

	LEON_TM_MEASURE(bench, leon->loptime_jtag);

	if (cnt>1) free(pinval);
	return retval;

}

int leon_jtag_set_registers(struct target *target, uint32_t addr,
                                   uint32_t *data, int cnt)
{
//	return leon_write_memory(target, addr, 4, cnt, (uint8_t *)data);

	int i, retval = ERROR_OK;
	struct leon_common *leon = target_to_leon(target);
	struct scan_field field;
	uint8_t inval[4], outval[4];
/* sanitize arguments */
	if (cnt==0) return ERROR_OK;
	addr = addr & ~0x3u;
	if (cnt>LEON_BLOCK_MAX_LENGTH) {
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
		field.in_value = inval;
// CONTROL WORD
		buf_set_u32(&outval[0], 0, LEON_DRSCAN_NBITS, LEON_PID_CONTROL | LEON_CTRL_WRITE_DATA | LEON_CTRL_ADDR_INC | cnt);
		jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
// ADDRESS HI
		buf_set_u32(&outval[0], 0, LEON_DRSCAN_NBITS, LEON_PID_ADDRESS | (addr>>16));
		jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
// ADDRESS LO
		buf_set_u32(&outval[0], 0, LEON_DRSCAN_NBITS, LEON_PID_ADDRESS | (addr&0xffff));
		jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
// DATA - load data from output buffer
		for (i=0;i<cnt;++i) {
			uint32_t dval = *(data + i);
			buf_set_u32(&outval[0], 0, LEON_DRSCAN_NBITS, LEON_PID_DATA | (uint16_t)(dval>>16));
			jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
			buf_set_u32(&outval[0], 0, LEON_DRSCAN_NBITS, LEON_PID_DATA | (uint16_t)(dval&0xffff));
			jtag_add_dr_scan(leon->tap, 1, &field, TAP_IDLE);
		}
// execute queue
		retval = jtag_execute_queue();
		if (retval!=ERROR_OK) {
			LOG_ERROR("Execute JTAG queue failed (%s, %d)", leon->tap->tapname, retval);
			break;
		}
		keep_alive(); /* keep alive gdb connection */
	} while(0);

	LEON_TM_MEASURE(bench, leon->loptime_jtag);

	return retval;
}
