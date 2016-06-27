/**
 * xpcu2 - driver for Xilinx Platform Cable USB II
 * (template from dummy.c)
 * RB 2016
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <jtag/interface.h>
#include <jtag/commands.h>
#include "libusb_common.h"
#include <libusb.h> /* get config descriptor */
//#include <string.h>
//#include <time.h>

#define QUEUE_BUF_CAP   512  /* TODO: more only with adding queue fragmentation for USB BULK OUT/IN transactions */
#define QUEUE_SCAN_CAP  10   /* TODO: dynamic allocation of scan buffers */

typedef struct xpcu2_scan {
	int length;
	struct scan_command *command; /* Corresponding scan command */
} xpcu2_scan_t;

struct xpcu2_private {
	struct jtag_libusb_device_handle *udev;
	int epbout, epbin; /* EPidx of bulk output and bulk input endpoints */

	int     queue_length;      /* size of queue [in bits] */
	int     queue_outsz;       /* size of outbuf [in bytes] */
	uint8_t queue_outbuf[QUEUE_BUF_CAP]; /* maximum size of BULK buffer */
	uint8_t queue_inbuf[QUEUE_BUF_CAP];  /* maximum size of BULK buffer */
	int     queue_insz;        /* required size of inbuf [in bits !!!] number of bits required from TDO */
	int     queue_read;        /* size of inbuf from last read [in bytes] */

	xpcu2_scan_t queue_scans[10];
	int     queue_scansz;

	int     tap_last_tms;      /* save the most recently TMS in queue */
	int     tap_last_tdi;      /* save the most recently TDI in queue */

	int cur_speed_idx;
};
typedef struct xpcu2_private xpcu2_private_t;

static xpcu2_private_t *xpcu2_data = NULL;


void xpcu_tap_init(xpcu2_private_t *pxd);
void xpcu_debug_buffer(uint8_t *pb, int sz);

/* -------------------------------------------------------------------------- */
#define XPCU_USB_VID      0x03FD  // Xilinx
#define XPCU_USB_PID      0x0008  // Platform Cable USB II
#define XPCU_USB_CFGIDX   0 /* 2 */      // XPCU2 has only cfg #2 -- !!! je to 0, protoze OOCD/libusb1_common.c pouziva index konfigurace a ne ConfigNum z USB deskriptoru
#define XPCU_USB_IFACE    0       // XPCU2 cfg #2 has only iface #0
#define XPCU_USB_TIMEOUT  500     // transfer timeout 500ms

#define XPCU_USB_REQ_EZUSB 0xA0
#define XPCU_USB_REQ_JTAG  0xB0

/* values of wValue for vendor request 0xB0 */
#define XPCU_FNC_GET_U16REG 0x0050 /* ??? get/set U16 register/memory,  wIdx is regidx or address */
#define XPCU_FNC_GET_U8REG  0x0020 /* ??? get/set U8 register/memory, wIdx is regidx or address */
#define XPCU_FNC_START      0x0018 /* start jtag transfer ??? something on the beginning of each transfer, wIdx=0 */
#define XPCU_FNC_SET_SPEED  0x0028
#define XPCU_FNC_GET_STATE  0x0038 /* status of ???  bit 0 (0x01) is the last/current level of TDO, wIdx = speed index (supported speeds: 20=750kHz, 19=1.5MHz, 18=3MHz, 17=6MHz, 16=12MHz) */
#define XPCU_FNC_FINISH     0x0010 /* finish jtag transfer */
#define XPCU_FNC_NUMTICKS   0x00A6 /* number of time moments in bulk data (wIdx = number of moments - 1 ; i.e. 0=1 tick, 1=2 ticks, 2=3 ticks, ...) */

#define XPCU_U16REG_FWVER   0
#define XPCU_U16REG_PLDVER  1
#define XPCU_U16REG_TYPE    258

/* supported speeds [khz,idx] */
static const int xpcu_speeds[][2] = {{750,20},{1500,19},{3000,18},{6000,17},{12000,16},{0,0}};

/* warnings - check versions */
#define XPCU_FWVER          0x0517
#define XPCU_PLDVER         0x0012
#define XPCU_TYPE           0x0004

/* -------------------------------------------------------------------------- */
static int xpcu2_usb_get_u16reg(struct jtag_libusb_device_handle *udev, uint16_t idx, uint16_t *val)
{
	int rc;
	unsigned char bbuf[2];
	rc =jtag_libusb_control_transfer(udev,
				LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN,
				XPCU_USB_REQ_JTAG, XPCU_FNC_GET_U16REG, idx, (char *)bbuf, 2, XPCU_USB_TIMEOUT);
	if (rc!=2) return ERROR_FAIL;
	if (val) {
		*val = ((uint16_t)bbuf[1]<<8)|bbuf[0];
	}
	return ERROR_OK;
}

static int xpcu2_usb_get_state(struct jtag_libusb_device_handle *udev, uint8_t *val)
{
	int rc;
	unsigned char bbuf[1];
	rc =jtag_libusb_control_transfer(udev,
				LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN,
				XPCU_USB_REQ_JTAG, XPCU_FNC_GET_STATE, 0, (char *)bbuf, 1, XPCU_USB_TIMEOUT);
	if (rc!=1) return ERROR_FAIL;
	if (val)
		*val = bbuf[0];
	return ERROR_OK;
}

static int xpcu2_usb_set_speed(struct jtag_libusb_device_handle *udev, uint16_t spidx)
{
	int rc;
	rc =jtag_libusb_control_transfer(udev,
				LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT,
				XPCU_USB_REQ_JTAG, XPCU_FNC_SET_SPEED, spidx, NULL, 0, XPCU_USB_TIMEOUT);
	if (rc!=0) return ERROR_FAIL;
	return ERROR_OK;
}

static int xpcu2_usb_start_queue(struct jtag_libusb_device_handle *udev) //, uint16_t spidx)
{
	int rc;
	rc =jtag_libusb_control_transfer(udev,
				LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT,
				XPCU_USB_REQ_JTAG, XPCU_FNC_START, 0, NULL, 0, XPCU_USB_TIMEOUT);
	if (rc!=0) return ERROR_FAIL;

//	rc =jtag_libusb_control_transfer(udev,
//				LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT,
//				XPCU_USB_REQ_JTAG, XPCU_FNC_SET_SPEED, spidx, NULL, 0, XPCU_USB_TIMEOUT);
//	if (rc!=0) return ERROR_FAIL;

	return ERROR_OK;
}

static int xpcu2_usb_finish_queue(struct jtag_libusb_device_handle *udev)
{
	int rc;
	rc =jtag_libusb_control_transfer(udev,
				LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT,
				XPCU_USB_REQ_JTAG, XPCU_FNC_FINISH, 0, NULL, 0, XPCU_USB_TIMEOUT);
	if (rc!=0) return ERROR_FAIL;
	return ERROR_OK;
}

static int xpcu2_usb_open(xpcu2_private_t *pxd)
{
	uint16_t val;
	const uint16_t vids[] = {XPCU_USB_VID, 0 };
	const uint16_t pids[] = {XPCU_USB_PID, 0 };
	struct jtag_libusb_device_handle *devh = NULL;
	struct libusb_config_descriptor *ucfg = NULL;

	if (!pxd) return ERROR_FAIL;
	if (pxd->udev) return ERROR_OK;

	do {
		LOG_DEBUG("usb open");
		if (jtag_libusb_open(vids, pids, NULL, &devh) != ERROR_OK) {
			LOG_ERROR("Xilinx Platform Cable USB II hasn't been detected.");
			break;
		}

		LOG_DEBUG("set cfg");
		if (jtag_libusb_set_configuration(devh, XPCU_USB_CFGIDX)) {
			LOG_ERROR("set cfg failed.");
			break;
		}
		LOG_DEBUG("claim iface");
		if (jtag_libusb_claim_interface(devh, XPCU_USB_IFACE)) {
			LOG_ERROR("claim iface failed.");
			break;
		}
		// determine EPidx of bulk endpoints
		int rc = libusb_get_config_descriptor(libusb_get_device(devh), XPCU_USB_CFGIDX, &ucfg);
		if (rc || ucfg==NULL) {
			LOG_ERROR("read cfg failed (%d).", rc);
			break;
		}
		if (ucfg && ucfg->interface && ucfg->interface->altsetting &&
		    ucfg->interface->altsetting->endpoint &&
		    ucfg->interface->altsetting->bNumEndpoints) {
			int i;
			// find first BULK OUT ep
			for (i=0;i<ucfg->interface->altsetting->bNumEndpoints;++i) {
				if (!(ucfg->interface->altsetting->endpoint[i].bEndpointAddress & 0x80) &&
				    (ucfg->interface->altsetting->endpoint[i].bmAttributes & 0x03)==0x02) {
					pxd->epbout = ucfg->interface->altsetting->endpoint[i].bEndpointAddress;
					break;
				}
			}
			// find first BULK IN ep
			for (i=0;i<ucfg->interface->altsetting->bNumEndpoints;++i) {
				if ((ucfg->interface->altsetting->endpoint[i].bEndpointAddress & 0x80) &&
				    (ucfg->interface->altsetting->endpoint[i].bmAttributes & 0x03)==0x02) {
					pxd->epbin = ucfg->interface->altsetting->endpoint[i].bEndpointAddress;
					break;
				}
			}
		}
		libusb_free_config_descriptor(ucfg);
		if (!pxd->epbin || !pxd->epbout) {
			LOG_ERROR("find BULK endpoints failed (%u,%u).", pxd->epbin, pxd->epbout);
			break;
		}

		// check FW version
		if (xpcu2_usb_get_u16reg(devh, XPCU_U16REG_FWVER, &val)!=ERROR_OK) {
			LOG_ERROR("read FW version failed.");
			break;
		}
		if (val!=XPCU_FWVER) {
			LOG_WARNING("XPCU2 FW version not supported (x%04X)", val);
		} else {
			LOG_INFO("  FW version %u", val);
		}
		// check PLD version
		if (xpcu2_usb_get_u16reg(devh, XPCU_U16REG_PLDVER, &val)!=ERROR_OK) {
			LOG_ERROR("read PLD version failed.");
			break;
		}
		if (val!=XPCU_PLDVER) {
			LOG_WARNING("XPCU2 PLD version not supported (x%04X)", val);
		} else {
			LOG_INFO("  PLD version x%04X", val);
		}
		// check Type
		if (xpcu2_usb_get_u16reg(devh, XPCU_U16REG_TYPE, &val)!=ERROR_OK) {
			LOG_ERROR("read Type failed.");
			break;
		}
		if (val!=XPCU_TYPE) {
			LOG_WARNING("XPCU2 Type not supported (x%04X)", val);
		} else {
			LOG_INFO("  Type x%04X", val);
		}

		pxd->udev = devh;
		return ERROR_OK;
	} while (0);

	if (devh)
		jtag_libusb_close(devh);
	return ERROR_FAIL;
}

static int xpcu2_usb_close(xpcu2_private_t *pxd)
{
	LOG_DEBUG("usb close");
	if (!pxd) return ERROR_FAIL;
	// send finish cmd
	int rc = xpcu2_usb_finish_queue(pxd->udev);
	if (rc!=0) {
		LOG_ERROR("send finish cmd %d", rc);
	}
	jtag_libusb_close(pxd->udev);
	pxd->udev = NULL;
	return ERROR_OK;
}

/* -------------------------------------------------------------------------- */
static int xpcu2_usb_flush_queue(xpcu2_private_t *pxd)
{
	int rc = ERROR_OK;
	// start JTAG transactions
	xpcu2_usb_start_queue(xpcu2_data->udev); // , xpcu2_data->cur_speed_idx);
	// set length of data (bits-1)
	if (pxd->queue_length>0) do {
		rc = jtag_libusb_control_transfer(pxd->udev,
					LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT,
					XPCU_USB_REQ_JTAG, XPCU_FNC_NUMTICKS, pxd->queue_length-1, NULL, 0, XPCU_USB_TIMEOUT);
		if (rc!=0) {
			LOG_ERROR("flush queue (l1 - %d)", rc);
			break;
		} else {
			rc = ERROR_OK;
		}
		rc = jtag_libusb_bulk_write(pxd->udev, pxd->epbout, (char *) pxd->queue_outbuf, pxd->queue_outsz, XPCU_USB_TIMEOUT);
		if (rc!=pxd->queue_outsz) {
			LOG_ERROR("flush queue (w2 - %d)", rc);
			break;
		} else {
			rc = ERROR_OK;
		}
		if (pxd->queue_insz>0) {
			int sz = (pxd->queue_insz + 15)/15; // TDO is returned in 16bit words
//			LOG_INFO("XPCU-jtag rd %d bits ... %d words", pxd->queue_insz, sz);
			rc = jtag_libusb_bulk_read(pxd->udev, pxd->epbin, (char *) pxd->queue_inbuf, QUEUE_BUF_CAP, XPCU_USB_TIMEOUT);
			pxd->queue_read = rc;
			if (rc<sz || (rc & 1)) { // read back from XPCU by 16bit only
				LOG_WARNING("flush queue (r3 - %d , %d->%d)", rc, pxd->queue_insz, sz);
//				break;
			} else {
//				xpcu_debug_buffer(pxd->queue_inbuf, pxd->queue_read);
				if (pxd->queue_insz&0x1f) {
					if (!(pxd->queue_insz&0x10)) {
						int n = 16-(pxd->queue_insz&0x0f);
						uint16_t *pw = ((uint16_t *)pxd->queue_inbuf) + (pxd->queue_insz>>4);
//						LOG_INFO(" - The MSB(16) must be shifted to right position (%d - %d)", pxd->queue_insz, n);
						*pw = *pw >> n;
					} else {
						if (pxd->queue_insz&0x1F) {
							int n = 32-(pxd->queue_insz&0x1f);
							uint32_t *pw = ((uint32_t *)pxd->queue_inbuf) + (pxd->queue_insz>>5);
//							LOG_INFO(" - The MSB(32) must be shifted to right position (%d - %d)", pxd->queue_insz, n);
							*pw = *pw >> n;
						}
					}
				}
				rc = ERROR_OK;
			}
		}
	} while(0);

	/* check status */
	if (rc==ERROR_OK) {
		uint8_t st;
		rc = xpcu2_usb_get_state(pxd->udev, &st);
		/* TODO: check state */
//		LOG_INFO("check state 0x%02X", st);

		/* copy TDO from buffers */
		/* TODO */
		if (pxd->queue_scansz) {
			int i, ofst = 0; /* first bit in response is the MSB */
//			xpcu_debug_buffer(pxd->queue_inbuf, pxd->queue_read);
			uint8_t *pbcur;
			if (pxd->queue_scansz>1) {
				int maxsz = 0;
				for (int j=0; j<pxd->queue_scansz;++j) {
					if (pxd->queue_scans[j].length>maxsz) maxsz=pxd->queue_scans[j].length;
				}
				pbcur = calloc(DIV_ROUND_UP(maxsz, 8), 1);
			} else {
				pbcur = pxd->queue_inbuf;
			}
			// divide input data according to queue
			for (i=0;i<pxd->queue_scansz;++i) {
				if (ofst<0) {
					rc = ERROR_JTAG_QUEUE_FAILED;
					break;
				}
				if (pxd->queue_scansz>1) {
					buf_set_buf(pxd->queue_inbuf, ofst, pbcur, 0, pxd->queue_scans[i].length);
				}
//				LOG_INFO("return scan #%d from %d, l=%d (x%08X)", i, ofst, pxd->queue_scans[i].length, *(uint32_t *)pbcur & (((uint32_t)1<<pxd->queue_scans[i].length)-1) );
				if (jtag_read_buffer(pbcur, pxd->queue_scans[i].command) != ERROR_OK) {
					xpcu_tap_init(pxd);
					rc = ERROR_JTAG_QUEUE_FAILED;
					break;
				}
				ofst += pxd->queue_scans[i].length; /* go to next returned stream */
			}
			if (pxd->queue_scansz>1) {
				free(pbcur);
			}
		}
	}

	return rc;
}

/* -------------------------------------------------------------------------- */
static int tapcnt = 0;
void xpcu_tap_init(xpcu2_private_t *pxd)
{
	pxd->queue_length = 0;
	pxd->queue_outsz = 0;
	pxd->queue_insz = 0;
	pxd->queue_scansz = 0;
	tapcnt = 0;
}

/* add one step to queue tms,tdi is value of TMS, TDI;
 *     rd is TCK step for reading TDO and wr is TCK step for writing TMS,TDI
 *     if rd and wr are 0 - the function adds one step padding */
void xpcu_tap_append_step(xpcu2_private_t *pxd, int tms, int tdi, int rd, int wr)
{
	uint8_t *pqo, msk;
	int nextbit = pxd->queue_length & 0x03; /* four bits of TMS,TDI,TCKrd,TCKwr in one byte */
	int nextword = 2*(pxd->queue_length >> 2);

	/* if tms/tdi==-1 use the latest value of TMS/TDI */
	if (tms>=0)
		pxd->tap_last_tms = tms;
	else
		tms = pxd->tap_last_tms;
	if (tdi>=0)
		pxd->tap_last_tdi = tdi;
	else
		tdi = pxd->tap_last_tdi;

	pqo = &pxd->queue_outbuf[nextword];
	if (nextbit==0) *pqo = 0; /* reset buffer */
	msk = (1<<nextbit);
	if (tdi) *pqo |= msk;
	if (tms) *pqo |= (msk<<4);
	pqo++;
	if (nextbit==0) *pqo = 0; /* reset buffer */
	if (wr) *pqo |= msk;
	if (rd) *pqo |= (msk<<4);
	if (nextbit==0) pxd->queue_outsz += 2; /* increment output bytes */
	if (rd) pxd->queue_insz++; /* increment required input bits */
	pxd->queue_length++; /* increment bits in queue */
//LOG_INFO("STEP: %d - %d (m=%u) %d , %d - tms=%d, tdi=%d, rd=%d, wr=%d", nextword, nextbit, msk, pxd->queue_length, pxd->queue_insz, tms, tdi, rd, wr);
}

void xpcu_tap_append_scan(xpcu2_private_t *pxd, int rd, int wr, int length, uint8_t *buffer)
{
	int i = 0; // pocitadlo celeho retezce
	int j = 0; // pocitadlo bitu v bajtu
	int w = 0; // pocitadlo bitu ve slove pri cteni pro zarovnani kazdeho slova na cely bajt
	while (i<length) {
		j = pxd->queue_length & 0x03;
		if (rd && (w==31 || (i==length-1)) && j!=3) {
			xpcu_tap_append_step(pxd, 0, 1, 0, 0);
//			LOG_INFO("Add pad i=%d/%d,w=%d,j=%d", i, length, w, j);
		} else {
			int tms = (i==length-1) ? 1 : 0; // konec retezce
			xpcu_tap_append_step(pxd, tms, (buffer[i / 8] >> (i % 8)) & 1, rd, wr);
//			LOG_INFO("Add tdo i=%d/%d,w=%d,j=%d - tms=%d", i, length, w, j, tms);
			i++;
			if (++w==32) w = 0;
		}
		//if (++j==4) j = 0;
	}

	tapcnt += length;
//	LOG_INFO("Appended scan (rd=%d,wr=%d,l=%d)", rd, wr, length);
//	xpcu_debug_buffer(pxd->queue_outbuf, pxd->queue_outsz);
}

/* -------------------------------------------------------------------------- */
/* add reset sequence to queue */
static void xpcu_reset(xpcu2_private_t *pxd, int trst, int srst)
{
	uint8_t *pqo = &pxd->queue_outbuf[pxd->queue_outsz];
	const uint8_t rstseq[4] = {0xc0,0x08,0xf0,0x0f};
	memcpy(pqo, rstseq, 4);
	pxd->queue_outsz += 4;
	pxd->queue_length += 8;
}

static int xpcu_end_state(tap_state_t state)
{
	if (tap_is_state_stable(state))
		tap_set_end_state(state);
	else {
		LOG_ERROR("BUG: %i is not a valid end state", state);
		return ERROR_JTAG_NOT_STABLE_STATE;
	}
	return ERROR_OK;
}

static void xpcu_state_move(xpcu2_private_t *pxd)
{
	int i;
	int tms = 0;
	uint8_t tms_scan = tap_get_tms_path(tap_get_state(), tap_get_end_state());
	uint8_t tms_scan_bits = tap_get_tms_path_len(tap_get_state(), tap_get_end_state());

	for (i = 0; i < tms_scan_bits; i++) {
		tms = (tms_scan >> i) & 1;
		xpcu_tap_append_step(pxd, tms, 0, 0, 1);
	}
	tap_set_state(tap_get_end_state());
}


static void xpcu_runtest(xpcu2_private_t *pxd, int num_cycles)
{
	int i;
	tap_state_t saved_end_state = tap_get_end_state();

	/* only do a state_move when we're not already in IDLE */
	if (tap_get_state() != TAP_IDLE) {
		xpcu_end_state(TAP_IDLE);
		xpcu_state_move(pxd);
	}
	/* execute num_cycles */
	for (i = 0; i < num_cycles; i++)
		xpcu_tap_append_step(pxd, 0, 0, 0, 1);
	/* finish in end_state */
	xpcu_end_state(saved_end_state);
	if (tap_get_state() != tap_get_end_state())
		xpcu_state_move(pxd);
}

int xpcu_path_move(xpcu2_private_t *pxd, int num_states, tap_state_t *path)
{
	int i;
	for (i = 0; i < num_states; i++) {
		if (path[i] == tap_state_transition(tap_get_state(), false))
			xpcu_tap_append_step(pxd, 0, 0, 0, 1);
		else if (path[i] == tap_state_transition(tap_get_state(), true))
			xpcu_tap_append_step(pxd, 1, 0, 0, 1);
		else {
			LOG_ERROR("BUG: %s -> %s isn't a valid TAP transition",
					tap_state_name(tap_get_state()), tap_state_name(path[i]));
			return ERROR_JTAG_TRANSITION_INVALID;
		}
		tap_set_state(path[i]);
	}
	tap_set_end_state(tap_get_state());
	return ERROR_OK;
}

int xpcu_tms(xpcu2_private_t *pxd, struct tms_command *tcmd)
{
	unsigned num_bits = tcmd->num_bits;
	const uint8_t *bits = tcmd->bits;
	uint8_t msk = 1;

	while (num_bits) {
		if (pxd->queue_outsz>=QUEUE_BUF_CAP) return ERROR_JTAG_QUEUE_FAILED;
		if (*bits & msk)
			xpcu_tap_append_step(pxd, 1, -1, 0, 1);
		else
			xpcu_tap_append_step(pxd, 0, -1, 0, 1);
		msk <<= 1;
		if (msk==0) {
			msk = 1;
			bits++;
		}
		num_bits--;
	}
	return ERROR_OK;
}

int xpcu_scan(xpcu2_private_t *pxd, struct scan_command *scmd)
{
	tap_state_t saved_end_state;
	int scan_size, rd, wr;
	uint8_t *buffer = NULL;

//	LOG_INFO("scan nfld = %d", scmd->num_fields);
	for (rd=0;rd<scmd->num_fields;++rd) {
//		LOG_INFO("  #%d: nbits=%d", rd, scmd->fields[rd].num_bits);
	}

	scan_size = jtag_build_buffer(scmd, &buffer);
//	LOG_INFO("scan input, length = %d", scan_size);
//	xpcu_debug_buffer(buffer, (scan_size + 7) / 8);
	saved_end_state = tap_get_end_state();

	switch (jtag_scan_type(scmd)) {
		case SCAN_IN:  rd = 1; wr = 0; break;
		case SCAN_OUT: rd = 0; wr = 1; break;
		case SCAN_IO:  rd = 1; wr = 1; break;
		default:       rd = 0; wr = 0; break;
	}

	if (rd) {
		if (pxd->queue_scansz >= QUEUE_SCAN_CAP)
			return ERROR_JTAG_QUEUE_FAILED;
	}

	/* Move to appropriate scan state */
	xpcu_end_state(scmd->ir_scan ? TAP_IRSHIFT : TAP_DRSHIFT);
	if (tap_get_state() != tap_get_end_state())
		xpcu_state_move(pxd);
	/* add scan - the last tick for reading TDO is with TMS=1 to change state to TAP_DR/IREXIT1 */
	xpcu_tap_append_scan(pxd, rd, wr, scan_size, buffer);

	/* We are in Exit1, go to Pause */
	xpcu_tap_append_step(pxd, 0, 0, 0, 1);
	tap_set_state(scmd->ir_scan ? TAP_IRPAUSE : TAP_DRPAUSE);

	xpcu_end_state(saved_end_state);
	if (tap_get_state() != tap_get_end_state())
		xpcu_state_move(pxd);

	if (buffer)
		free(buffer);

	if (rd) {
		pxd->queue_scans[pxd->queue_scansz].length = scan_size;
		pxd->queue_scans[pxd->queue_scansz].command = scmd;
		pxd->queue_scansz++;
	}
	return ERROR_OK;
}

/* ================================== */
static int xpcu_execute_queue(void)
{
	struct jtag_command *cmd;
	int ret = ERROR_OK;

//LOG_INFO("execQ: l=%d, osz=%d, isz=%d, rd=%d, ssz=%d, TAPst = %s",
//           xpcu2_data->queue_length, xpcu2_data->queue_outsz,
//           xpcu2_data->queue_insz, xpcu2_data->queue_read,
//           xpcu2_data->queue_scansz, tap_state_name(tap_get_state()));

	if (jtag_command_queue) {
		// prepare transactions from cmd_queue
		for (cmd=jtag_command_queue; ret==ERROR_OK && cmd!=NULL; cmd=cmd->next) {
			switch (cmd->type) {
				case JTAG_RESET:
					LOG_DEBUG("reset trst: %i srst %i", cmd->cmd.reset->trst, cmd->cmd.reset->srst);
					xpcu_reset(xpcu2_data, cmd->cmd.reset->trst, cmd->cmd.reset->srst);
					/* TODO: tady se asi jen nastavuji signaly trst a srst na hodnotu, nemelo by se jit do TLR v BSCANu */
					break;
				case JTAG_RUNTEST:
					LOG_DEBUG("runtest %i cycles, end in %i", cmd->cmd.runtest->num_cycles, cmd->cmd.runtest->end_state);
					if (cmd->cmd.runtest->end_state != -1)
						xpcu_end_state(cmd->cmd.runtest->end_state);
					xpcu_runtest(xpcu2_data, cmd->cmd.runtest->num_cycles);
					break;
				case JTAG_TLR_RESET:
					LOG_DEBUG("statemove end in %i", cmd->cmd.statemove->end_state);
					if (cmd->cmd.statemove->end_state != -1) {
						if (xpcu_end_state(cmd->cmd.statemove->end_state)) {
							ret = ERROR_JTAG_NOT_STABLE_STATE;
							break;
						}
					}
					xpcu_state_move(xpcu2_data);
					break;
				case JTAG_PATHMOVE:
					LOG_DEBUG("pathmove: %i states, end in %i",
										cmd->cmd.pathmove->num_states,
										cmd->cmd.pathmove->path[cmd->cmd.pathmove->num_states - 1]);
					if (xpcu_path_move(xpcu2_data, cmd->cmd.pathmove->num_states, cmd->cmd.pathmove->path)) {
						ret = ERROR_JTAG_TRANSITION_INVALID;
						break;
					}
					break;
				case JTAG_SLEEP:
					LOG_DEBUG("sleep %i", cmd->cmd.sleep->us);
					jtag_sleep(cmd->cmd.sleep->us);
					break;
				case JTAG_TMS:
					LOG_DEBUG("add %d jtag tms", cmd->cmd.tms->num_bits);
					xpcu_tms(xpcu2_data, cmd->cmd.tms);
					break;
				case JTAG_SCAN:
					LOG_DEBUG("%s scan end in %i", (cmd->cmd.scan->ir_scan) ? "IR" : "DR", cmd->cmd.scan->end_state);
					if (cmd->cmd.scan->end_state != -1)
						xpcu_end_state(cmd->cmd.scan->end_state);
					xpcu_scan(xpcu2_data, cmd->cmd.scan);
					break;

				default:
					LOG_ERROR("BUG: unknown JTAG command type encountered (%d)", cmd->type);
					ret = ERROR_JTAG_NOT_IMPLEMENTED;
					break;
			}
			//
			xpcu2_usb_flush_queue(xpcu2_data);
			/* reset queue */
			xpcu_tap_init(xpcu2_data);
		}

		// finish JTAG transactions
		//xpcu2_usb_finish_queue(xpcu2_data->udev);
	}
	return ret;
}


static int xpcu_khz(int khz, int *jtag_speed)
{
	int i = 0;
	while (xpcu_speeds[i][0]) {
		if (khz==xpcu_speeds[i][0]) {
			*jtag_speed = xpcu_speeds[i][1];
			return ERROR_OK;
		}
		i++;
	}
	LOG_ERROR("%d khz not supported", khz);
	return ERROR_FAIL;
}

static int xpcu_speed_div(int speed, int *khz)
{
	int i = 0;
	while (xpcu_speeds[i][1]) {
		if (speed==xpcu_speeds[i][1]) {
			*khz = xpcu_speeds[i][0];
			return ERROR_OK;
		}
		i++;
	}
	LOG_INFO("speed idx %d not supported", speed);
	return ERROR_FAIL;
}

static int xpcu_speed(int speed)
{
	int rc;
	LOG_INFO("set speed %d", speed);
	if (!xpcu2_data || !xpcu2_data->udev) return ERROR_FAIL;
	rc = xpcu2_usb_set_speed(xpcu2_data->udev, speed);
	if (rc==ERROR_OK)
		xpcu2_data->cur_speed_idx = speed;
	return rc;
}

static int xpcu_init(void)
{
	xpcu2_data = malloc(sizeof(xpcu2_private_t));
	do {
		if (xpcu2_data==NULL)
			break;
		memset(xpcu2_data, 0, sizeof(xpcu2_private_t));
		if (xpcu2_usb_open(xpcu2_data)!=ERROR_OK)
			break;

		xpcu2_data->cur_speed_idx = xpcu_speeds[0][1];

		xpcu_tap_init(xpcu2_data);

		return ERROR_OK;
	} while(0);
	if (xpcu2_data) free(xpcu2_data);
	xpcu2_data = NULL;
	return ERROR_FAIL;
}

static int xpcu_quit(void)
{
	int rc = ERROR_OK;
	if (xpcu2_data) {
		rc = xpcu2_usb_close(xpcu2_data);
		free(xpcu2_data);
		xpcu2_data = NULL;
	}
	return rc;
}

/* -------------------------------------------------------------------------- */
#define BYTES_PER_LINE  16
void xpcu_debug_buffer(uint8_t *pb, int sz)
{
	char line[81];
	char s[4];
	int i;
	int j;

	for (i = 0; i < sz; i += BYTES_PER_LINE) {
		snprintf(line, 5, "%04x", i);
		for (j = i; j < i + BYTES_PER_LINE && j < sz; j++) {
			snprintf(s, 4, " %02x", pb[j]);
			strcat(line, s);
		}
		LOG_INFO("%s", line);
	}
}

/* -------------------------------------------------------------------------- */
COMMAND_HANDLER(xpcu_cmd_rd_u16)
{
	int rc;
	unsigned long idx;
	uint16_t val; 
	// arg1 = wIndex
	if (!xpcu2_data || !xpcu2_data->udev) return ERROR_FAIL;
	if (CMD_ARGC < 1) return ERROR_COMMAND_SYNTAX_ERROR;
	idx = strtoul(CMD_ARGV[0], NULL, 0) & 0xffff;

	rc = xpcu2_usb_get_u16reg(xpcu2_data->udev, idx, &val);
	if (rc!=ERROR_OK) {
		LOG_ERROR("read U16 at x%04lX (%lu) failed (%d).", idx, idx, rc);
	} else {
		LOG_INFO("read U16 at x%04lX (%lu) = (%d) %u (0x%04X)", idx, idx, rc, val, val);
	}
	return ERROR_OK;
}

COMMAND_HANDLER(xpcu_cmd_rd_state)
{
	int rc;
	uint8_t val; 
	// arg1 = wIndex
	if (!xpcu2_data || !xpcu2_data->udev) return ERROR_FAIL;

	rc = xpcu2_usb_get_state(xpcu2_data->udev, &val);
	if (rc!=ERROR_OK) {
		LOG_ERROR("read state failed (%d).", rc);
	} else {
		LOG_INFO("read state (%d) %u (0x%02X)", rc, val, val);
	}
	return ERROR_OK;
}

/* -------------------------------------------------------------------------- */
static const struct command_registration xpcu_command_handlers[] = {
	{
		.name = "reg16",
		.mode = COMMAND_ANY,
		.help = "read U16 from XPCU. arg is address (wIndex)",
		.handler = xpcu_cmd_rd_u16,
	},
	{
		.name = "state",
		.mode = COMMAND_ANY,
		.help = "read XPCU2 state (b0: TDO)",
		.handler = xpcu_cmd_rd_state,
	},
	COMMAND_REGISTRATION_DONE,
};

static const struct command_registration xpcu_group[] = {
	{
		.name = "xpcu",
		.mode = COMMAND_ANY,
		.help = "XPCU driver command group",
		.chain = xpcu_command_handlers,
		.usage = ""
	},
	COMMAND_REGISTRATION_DONE
};

/* -------------------------------------------------------------------------- */
struct jtag_interface xpcu_interface = {
		.name = "xpcu2",

		.supported = DEBUG_CAP_TMS_SEQ,
		.commands = xpcu_group,
		.transports = jtag_only,

		.execute_queue = &xpcu_execute_queue,

		.speed = &xpcu_speed,
		.khz = &xpcu_khz,
		.speed_div = &xpcu_speed_div,

		.init = &xpcu_init,
		.quit = &xpcu_quit,
	};
