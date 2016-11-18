/**
 * OOCD target for Leon (Sparc v8) CPUs
 * (C)2016 Roman Bartosinski, daiteq,s.r.o.
 *
 */

#ifndef LEON_OOCD_TARGET_HW_DESCRIPTION_HEADER_FILE
#define LEON_OOCD_TARGET_HW_DESCRIPTION_HEADER_FILE

/* ========================================================================== */
/* LEON, DSU registers */
/* --- LEON2 Memory Map ----------------------------------------------------- */
#define LEON_REGS_BASE_ADDRESS      (0x80000000)
#define LEON_REGS_MEM_CFG_1_REG         (LEON_REGS_BASE_ADDRESS+0x00)
#define LEON_REGS_MEM_CFG_2_REG         (LEON_REGS_BASE_ADDRESS+0x04)
#define LEON_REGS_MEM_CFG_3_REG         (LEON_REGS_BASE_ADDRESS+0x08)
#define LEON_REGS_MEM_CFG_4_REG         (LEON_REGS_BASE_ADDRESS+0x0C)
#define LEON_REGS_AHB_FALL_ADDR_REG     (LEON_REGS_BASE_ADDRESS+0x10)

#define LEON_REGS_CACHE_CTRL_REG        (LEON_REGS_BASE_ADDRESS+0x14)
#define LEON_REGS_PWR_DOWN_REG          (LEON_REGS_BASE_ADDRESS+0x18)
#define LEON_REGS_WRPR_1_REG            (LEON_REGS_BASE_ADDRESS+0x1C)
#define LEON_REGS_WRPR_2_REG            (LEON_REGS_BASE_ADDRESS+0x20)

#define LEON_REGS_LEON_CFG_REG          (LEON_REGS_BASE_ADDRESS+0x24)
// ... TODO

/* --- Leon Configuration Register ------------------------------------------ */
#define LEON_CFG_REG_IS_MMU          (1L<<31)
#define LEON_CFG_REG_IS_DSU          (1L<<30)
#define LEON_CFG_REG_IS_SDRAM        (1L<<29)
#define LEON_CFG_REG_NUM_WPOIS_SHIFT (26)       /* Number of implemented watchpopints */
#define LEON_CFG_REG_NUM_WPOIS_MASK  (7L<<LEON_CFG_REG_NUM_WPOIS_SHIFT)
#define LEON_CFG_REG_INST_MAC        (1L<<25)
#define LEON_CFG_REG_NWINDOWS_SHIFT  (20)       /* Number of SPARC register windows - 1 */
#define LEON_CFG_REG_NWINDOWS_MASK   (31L<<LEON_CFG_REG_NWINDOWS_SHIFT)
#define LEON_CFG_REG_ICSET_SZ_SHIFT  (17)       /* Instruction cache set size (in kB) = 2^ICSZ */
#define LEON_CFG_REG_ICSET_SZ_MASK   (7L<<LEON_CFG_REG_ICSET_SZ_SHIFT)
#define LEON_CFG_REG_ICLINE_SZ_SHIFT (15)       /* Instruction cache line size (in 32b words) = 2^ILSZ */
#define LEON_CFG_REG_ICLINE_SZ_MASK  (3L<<LEON_CFG_REG_ICLINE_SZ_SHIFT)
#define LEON_CFG_REG_DCSET_SZ_SHIFT  (12)       /* Data cache set size (in kB) = 2^DCSZ */
#define LEON_CFG_REG_DCSET_SZ_MASK   (7L<<LEON_CFG_REG_DCSET_SZ_SHIFT)
#define LEON_CFG_REG_DCLINE_SZ_SHIFT (10)   /* Data cache line size (in 32b words) = 2^DLSZ */
#define LEON_CFG_REG_DCLINE_SZ_MASK  (3L<<LEON_CFG_REG_DCLINE_SZ_SHIFT)
#define LEON_CFG_REG_INST_DIV        (1L<<9)
#define LEON_CFG_REG_INST_MUL        (1L<<8)
#define LEON_CFG_REG_IS_WDOG         (1L<<7)
#define LEON_CFG_REG_IS_MEMSTAT      (1L<<6)
#define LEON_CFG_REG_FPU_TYPE_SHIFT  (4)        /* FPU type */
#define LEON_CFG_REG_FPU_TYPE_MASK   (3L<<LEON_CFG_REG_FPU_TYPE_SHIFT)
#define LEON_CFG_REG_PCI_TYPE_SHIFT  (2)        /* PCI core */
#define LEON_CFG_REG_PCI_TYPE_MASK   (3L<<LEON_CFG_REG_PCI_TYPE_SHIFT)
#define LEON_CFG_REG_WRPR_TYPE_SHIFT (0)        /* Write protection */
#define LEON_CFG_REG_WRPR_TYPE_MASK  (3L<<LEON_CFG_REG_WRPR_TYPE_SHIFT)

#define LEON_CFG_WRPR_TYPE_NONE     (0)        /* Write Protection Type = NONE */
#define LEON_CFG_WRPR_TYPE_STD      (1)        /* Write Protection Type = Standard */

#define LEON_CFG_PCI_TYPE_NONE      (0)        /* PCI Core Type = NONE */
#define LEON_CFG_PCI_TYPE_INSIL     (1<<2)     /* PCI Core Type = InSilicon */
#define LEON_CFG_PCI_TYPE_ESA       (2<<2)     /* PCI Core Type = ESA */
#define LEON_CFG_PCI_TYPE_OTHER     (3<<2)     /* PCI Core Type = Other */

#define LEON_CFG_FPU_TYPE_NONE      (0)        /* FPU Type = NONE */
#define LEON_CFG_FPU_TYPE_MEIKO     (1<<4)     /* FPU Type = Meiko */
#define LEON_CFG_FPU_TYPE_GRFPU     (2<<4)     /* FPU Type = GRFPU */

/* --- Debug Support Unit Registers ------------------------------------------ */
#define LEON_DSU_BASE_ADDRESS       (0x90000000)
#define LEON_DSU_CTRL_REG           (LEON_DSU_BASE_ADDRESS+0x00000000)
#define LEON_DSU_TRACE_CTRL_REG     (LEON_DSU_BASE_ADDRESS+0x00000004)
#define LEON_DSU_TIME_TAG_REG       (LEON_DSU_BASE_ADDRESS+0x00000008)
#define LEON_DSU_AHB_BRK_ADDR1      (LEON_DSU_BASE_ADDRESS+0x00000010)
#define LEON_DSU_AHB_BRK_MASK1      (LEON_DSU_BASE_ADDRESS+0x00000014)
#define LEON_DSU_AHB_BRK_ADDR2      (LEON_DSU_BASE_ADDRESS+0x00000018)
#define LEON_DSU_AHB_BRK_MASK2      (LEON_DSU_BASE_ADDRESS+0x0000001C)

#define LEON_DSU_L2MT_CTRL_REG      (LEON_DSU_BASE_ADDRESS+0x00000020)

#define LEON_DSU_TRACE_BUFFER_BASE  (LEON_DSU_BASE_ADDRESS+0x00010000)
#define LEON_DSU_TRACE_BUFFER_SIZE  (0x00010000)
#define LEON_DSU_TRACE_BUFFER_LNSZ  (128) /* in bits */
#define LEON_DSU_IU_FPU_RFILE_BASE  (LEON_DSU_BASE_ADDRESS+0x00020000)
#define LEON_DSU_IU_FPU_RFILE_SIZE  (0x00020000)
#define LEON_DSU_IU_SREGS_BASE      (LEON_DSU_BASE_ADDRESS+0x00080000)
#define LEON_DSU_SREG_Y             (LEON_DSU_IU_SREGS_BASE+0x00)
#define LEON_DSU_SREG_PSR           (LEON_DSU_IU_SREGS_BASE+0x04)
#define LEON_DSU_SREG_WIM           (LEON_DSU_IU_SREGS_BASE+0x08)
#define LEON_DSU_SREG_TBR           (LEON_DSU_IU_SREGS_BASE+0x0C)
#define LEON_DSU_SREG_PC            (LEON_DSU_IU_SREGS_BASE+0x10)
#define LEON_DSU_SREG_NPC           (LEON_DSU_IU_SREGS_BASE+0x14)
#define LEON_DSU_SREG_FSR           (LEON_DSU_IU_SREGS_BASE+0x18)
//#define LEON_DSU_SREG_CSR
#define LEON_DSU_SREG_DSU_TRAP      (LEON_DSU_IU_SREGS_BASE+0x1C)
#define LEON_DSU_SREG_ASR_BASE      (LEON_DSU_IU_SREGS_BASE+0x40)
#define LEON_DSU_SREG_ASR_FIRST     (16)
#define LEON_DSU_SREG_ASR_NUM       (16)
#define LEON_DSU_SREG_ASR(x)        (LEON_DSU_SREG_ASR_BASE+(x-LEON_DSU_SREG_ASR_FIRST)*4)
#define LEON_DSU_IC_TAGS_BASE       (LEON_DSU_BASE_ADDRESS+0x00100000)
#define LEON_DSU_IC_TAGS_SIZE       (0x00040000)
#define LEON_DSU_IC_DATA_BASE       (LEON_DSU_BASE_ADDRESS+0x00140000)
#define LEON_DSU_IC_DATA_SIZE       (0x00040000)
#define LEON_DSU_DC_TAGS_BASE       (LEON_DSU_BASE_ADDRESS+0x00180000)
#define LEON_DSU_DC_TAGS_SIZE       (0x00040000)
#define LEON_DSU_DC_DATA_BASE       (LEON_DSU_BASE_ADDRESS+0x001C0000)
#define LEON_DSU_DC_DATA_SIZE       (0x00040000)
#define LEON_DSU_IC_MMU_CONTEXT_BASE     (LEON_DSU_BASE_ADDRESS+0x00300000)
#define LEON_DSU_DC_MMU_CONTEXT_BASE     (LEON_DSU_BASE_ADDRESS+0x00380000)

#define LEON_DSU_MMU_REGS_BASE      (LEON_DSU_BASE_ADDRESS+0x001E0000)
#define LEON_DSU_MMU_CTRL_REG       (LEON_DSU_MMU_REGS_BASE+0x00000000)
#define LEON_DSU_MMU_CTX_REG        (LEON_DSU_MMU_REGS_BASE+0x00000004)
#define LEON_DSU_MMU_CTX_TABPTR_REG (LEON_DSU_MMU_REGS_BASE+0x00000008)
#define LEON_DSU_MMU_FAULT_STAT_REG (LEON_DSU_MMU_REGS_BASE+0x0000000C)
#define LEON_DSU_MMU_FAULT_ADDR_REG (LEON_DSU_MMU_REGS_BASE+0x00000010)

/* --- DSU Control Register ------------------------------------------------- */
#define LEON_DSU_CTRL_TRACE_EN_SHIFT      (0)     /* Trace enable (TE). Enables the trace buffer. Value 0 after reset. */
#define LEON_DSU_CTRL_TRACE_EN            (1<<LEON_DSU_CTRL_TRACE_EN_SHIFT)
#define LEON_DSU_CTRL_DLY_CNT_SHIFT       (1)     /* Delay counter mode (DM). In mixed tracing mode, setting this bit will cause the delay counter to decrement on AHB traces. If reset, the delay counter will decrement on instruction traces. */
#define LEON_DSU_CTRL_DLY_CNT             (1<<LEON_DSU_CTRL_DLY_CNT_SHIFT)
#define LEON_DSU_CTRL_BRK_TRC_SHIFT       (2)     /* Break on trace (BT) - if set, will generate a DSU break condition on trace freeze. Value 0 after reset. */
#define LEON_DSU_CTRL_BRK_TRC             (1<<LEON_DSU_CTRL_BRK_TRC_SHIFT)
#define LEON_DSU_CTRL_FREEZE_TMR_SHIFT    (3)     /* Freeze timers (FT) - if set, the scaler in the LEON timer unit will be stopped during debug mode to preserve the time for the software application. Value 0 after reset. */
#define LEON_DSU_CTRL_FREEZE_TMR          (1<<LEON_DSU_CTRL_FREEZE_TMR_SHIFT)
#define LEON_DSU_CTRL_BRK_ERR_SHIFT       (4)     /* Break on error (BE) - if set, will force the processor to debug mode when the processor would have entered error condition (trap in trap). */
#define LEON_DSU_CTRL_BRK_ERR             (1<<LEON_DSU_CTRL_BRK_ERR_SHIFT)
#define LEON_DSU_CTRL_BRK_IU_WPT_SHIFT    (5)     /* Break on IU watchpoint - if set, debug mode will be forced on a IU watchpoint (trap 0xb). */
#define LEON_DSU_CTRL_BRK_IU_WPT          (1<<LEON_DSU_CTRL_BRK_IU_WPT_SHIFT)
#define LEON_DSU_CTRL_BRK_SW_BRK_SHIFT    (6)     /* Break on S/W breakpoint (BS) - if set, debug mode will be forced when an breakpoint instruction (ta 1) is executed. Value 0 after reset. */
#define LEON_DSU_CTRL_BRK_SW_BRK          (1<<LEON_DSU_CTRL_BRK_SW_BRK_SHIFT)
#define LEON_DSU_CTRL_BRK_NOW_SHIFT       (7)     /* Break now (BN) - Force processor into debug mode. If cleared, the processor will resume execution. */
#define LEON_DSU_CTRL_BRK_NOW             (1<<LEON_DSU_CTRL_BRK_NOW_SHIFT)
#define LEON_DSU_CTRL_BRK_DSU_BRK_SHIFT   (8)     /* Break on DSU breakpoint (BD) - if set, will force the processor to debug mode when an DSU breakpoint is hit. Value 0 after reset. */
#define LEON_DSU_CTRL_BRK_DSU_BRK         (1<<LEON_DSU_CTRL_BRK_DSU_BRK_SHIFT)
#define LEON_DSU_CTRL_BRK_TRAP_SHIFT      (9)     /* Break on trap (BX) - if set, will force the processor into debug mode when any trap occurs. */
#define LEON_DSU_CTRL_BRK_TRAP            (1<<LEON_DSU_CTRL_BRK_TRAP_SHIFT)
#define LEON_DSU_CTRL_BRK_ETRAP_SHIFT     (10)    /* Break on error traps (BZ) - if set, will force the processor into debug mode on all except the following traps: priviledged_instruction, fpu_disabled, window_overflow, window_underflow, asynchronous_interrupt, ticc_trap. */
#define LEON_DSU_CTRL_BRK_ETRAP           (1<<LEON_DSU_CTRL_BRK_ETRAP_SHIFT)
#define LEON_DSU_CTRL_DLY_CNT_EN_SHIFT    (11)    /* Delay counter enable (DE) - if set, the trace buffer delay counter will decrement for each stored trace. This bit is set automatically when an DSU breakpoint is hit and the delay counter is not equal to zero. Value 0 after reset. */
#define LEON_DSU_CTRL_DLY_CNT_EN          (1<<LEON_DSU_CTRL_DLY_CNT_EN_SHIFT)
#define LEON_DSU_CTRL_DEBUG_MODE_SHIFT    (12)    /* Debug mode (DM). Indicates when the processor has entered debug mode (read-only). */
#define LEON_DSU_CTRL_DEBUG_MODE          (1<<LEON_DSU_CTRL_DEBUG_MODE_SHIFT)
#define LEON_DSU_CTRL_DSUBRE_SHIFT        (13)    /* EB - value of the external DSUBRE signal (read-only) */
#define LEON_DSU_CTRL_DSUBRE              (1<<LEON_DSU_CTRL_DSUBRE_SHIFT)
#define LEON_DSU_CTRL_DSUEN_SHIFT         (14)    /* EE - value of the external DSUEN signal (read-only) */
#define LEON_DSU_CTRL_DSUEN               (1<<LEON_DSU_CTRL_DSUEN_SHIFT)
#define LEON_DSU_CTRL_CPU_ERR_SHIFT       (15)    /* Processor error mode (PE) - returns ‘1’ on read when processor is in error mode, else ‘0’. Read-only bit. */
#define LEON_DSU_CTRL_CPU_ERR             (1<<LEON_DSU_CTRL_CPU_ERR_SHIFT)
#define LEON_DSU_CTRL_SINGLE_STEP_SHIFT   (16)    /* Single step (SS) - if set, the processor will execute one instruction and the return to debug mode. Value 0 after reset. */
#define LEON_DSU_CTRL_SINGLE_STEP         (1<<LEON_DSU_CTRL_SINGLE_STEP_SHIFT)
#define LEON_DSU_CTRL_LINK_RESP_SHIFT     (17)    /* Link response (LR) - is set, the DSU communication link will send a response word after AHB transfer. Value 0 after reset. */
#define LEON_DSU_CTRL_LINK_RESP           (1<<LEON_DSU_CTRL_LINK_RESP_SHIFT)
#define LEON_DSU_CTRL_DEBUG_RESP_SHIFT    (18)    /* Debug mode response (DR) - if set, the DSU communication link will send a response word when the processor enters debug mode. Write only bit, it always reads 0. */
#define LEON_DSU_CTRL_DEBUG_RESP          (1<<LEON_DSU_CTRL_DEBUG_RESP_SHIFT)
#define LEON_DSU_CTRL_RESET_CPU_ERR_SHIFT (19)    /* Reset error mode (RE) - if set, will clear the error mode in the processor. */
#define LEON_DSU_CTRL_RESET_CPU_ERR       (1<<LEON_DSU_CTRL_RESET_CPU_ERR_SHIFT)
#define LEON_DSU_CTRL_TRCDLYCNT_SHIFT     (20)    /* Trace buffer delay counter (DCNT). Note that the number of bits actually implemented depends on the size of the trace buffer. */
#define LEON_DSU_CTRL_TRCDLYCNT_MASK      (0xfff<<LEON_DSU_CTRL_TRCDLYCNT_SHIFT)

#define LEON_DSU_CTRL_RW_MASK             (LEON_DSU_CTRL_TRACE_EN |            \
                                           LEON_DSU_CTRL_DLY_CNT |             \
                                           LEON_DSU_CTRL_BRK_TRC |             \
                                           LEON_DSU_CTRL_FREEZE_TMR |          \
                                           LEON_DSU_CTRL_BRK_ERR |             \
                                           LEON_DSU_CTRL_BRK_IU_WPT |          \
                                           LEON_DSU_CTRL_BRK_SW_BRK |          \
                                           LEON_DSU_CTRL_BRK_NOW |             \
                                           LEON_DSU_CTRL_BRK_DSU_BRK |         \
                                           LEON_DSU_CTRL_BRK_TRAP |            \
                                           LEON_DSU_CTRL_BRK_ETRAP |           \
                                           LEON_DSU_CTRL_DLY_CNT_EN)  /* DSU control register - read-write bits */

/* --- DSU Trace Buffer Control Register ------------------------------------ */
#define LEON_DSU_TRCTRL_INST_IDX_SHIFT    (0)     /* Instruction trace index counter. */
#define LEON_DSU_TRCTRL_INST_IDX_MASK     (0xFFF<<LEON_DSU_TRCTRL_INST_IDX_SHIFT)
#define LEON_DSU_TRCTRL_AHB_IDX_SHIFT     (12)     /* AHB trace index counter. */
#define LEON_DSU_TRCTRL_AHB_IDX_MASK      (0xFFF<<LEON_DSU_TRCTRL_AHB_IDX_SHIFT)
#define LEON_DSU_TRCTRL_TI_SHIFT          (24)     /* Trace instruction enable. */
#define LEON_DSU_TRCTRL_TI                (1<<LEON_DSU_TRCTRL_TI_SHIFT)
#define LEON_DSU_TRCTRL_TA_SHIFT          (25)     /* Trace AHB enable. */
#define LEON_DSU_TRCTRL_TA                (1<<LEON_DSU_TRCTRL_TA_SHIFT)
#define LEON_DSU_TRCTRL_AF_SHIFT          (26)     /* AHB trace buffer freeze. If set, the AHB trace buffer will be frozen when the processor enters debug mode. */
#define LEON_DSU_TRCTRL_AF                (1<<LEON_DSU_TRCTRL_AF_SHIFT)
#define LEON_DSU_TRCTRL_SFILT_SHIFT       (27)     /* Slave filtering (SFILT) - Trace only accesses to addresses with a certain prefix (bits 31:28). 0 = trace all accesses, 1 = trace only accesses with prefix 0x8, 2 = trace only addresses with prefix 0xA, 3 = trace only addresses with prefix 0xB. (See documentation in next section. */
#define LEON_DSU_TRCTRL_SFILT_MASK        (0x3<<LEON_DSU_TRCTRL_SFILT_SHIFT)
#define LEON_DSU_TRCTRL_MFILT_SHIFT       (29)     /* Master filtering (MFILT) - Trace only accesses from AHB masters with a particular master index. 0 = trace accesses from all masters, X = 1..6 trace only accesses from master X, 7 = trace only accesses from master 0. (See documentation in next section. */
#define LEON_DSU_TRCTRL_MFILT_MASK        (0x7<<LEON_DSU_TRCTRL_MFILT_SHIFT)

/* --- DSU Breakpoint Address ----------------------------------------------- */
#define LEON_DSU_BRKADDR_EX_SHIFT         (0)     /* Break on instruction */
#define LEON_DSU_BRKADDR_EX               (1<<LEON_DSU_BRKADDR_EX_SHIFT)
#define LEON_DSU_BRKADDR_BADDR_SHIFT      (2)     /* Breakpoint Address (bits 31:2) */
#define LEON_DSU_BRKADDR_BADDR_MASK       (0x3fffffff<<LEON_DSU_BRKADDR_BADDR_SHIFT)

/* --- DSU Breakpoint Mask -------------------------------------------------- */
#define LEON_DSU_BRKMASK_ST_SHIFT         (0)     /* Break on data store address */
#define LEON_DSU_BRKMASK_ST               (1<<LEON_DSU_BRKMASK_ST_SHIFT)
#define LEON_DSU_BRKMASK_LD_SHIFT         (1)     /* Break on data load address */
#define LEON_DSU_BRKMASK_LD               (1<<LEON_DSU_BRKMASK_LD_SHIFT)
#define LEON_DSU_BRKMASK_BMASK_SHIFT      (2)     /* Breakpoint Mask (bits 31:2) */
#define LEON_DSU_BRKMASK_BMASK_MASK       (0x3fffffff<<LEON_DSU_BRKMASK_BMASK_SHIFT)

/* --- DSU Trace 96-127bit -------------------------------------------------- */
#define LEON_DSU_TRCD_AHB_BRKHIT_SHIFT        (31)     /* Set to ‘1’ if a DSU AHB breakpoint hit occurred */
#define LEON_DSU_TRCD_AHB_BRKHIT              (1<<LEON_DSU_TRCD_AHB_BRKHIT_SHIFT)
#define LEON_DSU_TRCD_AHB_TIMETAG_SHIFT       (0)      /* The value of the time tag counter */
#define LEON_DSU_TRCD_AHB_TIMETAG_MASK        (0x3fffffff<<LEON_DSU_TRCD_AHB_TIMETAG_SHIFT)

#define LEON_DSU_TRCD_INST_BRKHIT_SHIFT       (31)     /* Set to ‘1’ if a DSU instruction breakpoint hit occurred */
#define LEON_DSU_TRCD_INST_BRKHIT             (1<<LEON_DSU_TRCD_INST_BRKHIT_SHIFT)
#define LEON_DSU_TRCD_INST_MULTICYCLE_SHIFT   (30)     /* Set to ‘1’ if a DSU instruction breakpoint hit occurred */
#define LEON_DSU_TRCD_INST_MULTICYCLE         (1<<LEON_DSU_TRCD_INST_MULTICYCLE_SHIFT)
#define LEON_DSU_TRCD_INST_TIMETAG_SHIFT      (0)      /* The value of the time tag counter */
#define LEON_DSU_TRCD_INST_TIMETAG_MASK       (0x3fffffff<<LEON_DSU_TRCD_INST_TIMETAG_SHIFT)
/* --- DSU Trace 64-95bit --------------------------------------------------- */
#define LEON_DSU_TRCC_AHB_IRL_SHIFT           (28)     /* Processor interrupt request input */
#define LEON_DSU_TRCC_AHB_IRL_MASK            (0xf<<LEON_DSU_TRCC_AHB_IRL_SHIFT)
#define LEON_DSU_TRCC_AHB_PIL_SHIFT           (24)     /* Processor interrupt level (psr.pil) */
#define LEON_DSU_TRCC_AHB_PIL_MASK            (0xf<<LEON_DSU_TRCC_AHB_PIL_SHIFT)
#define LEON_DSU_TRCC_AHB_TT_SHIFT            (16)     /* Trap type Processor trap type (psr.tt) */
#define LEON_DSU_TRCC_AHB_TT_MASK             (0xff<<LEON_DSU_TRCC_AHB_TT_SHIFT)
#define LEON_DSU_TRCC_AHB_HWRITE_SHIFT        (15)     /* AHB HWRITE */
#define LEON_DSU_TRCC_AHB_HWRITE              (1<<LEON_DSU_TRCC_AHB_HWRITE_SHIFT)
#define LEON_DSU_TRCC_AHB_HTRANS_SHIFT        (13)     /* AHB HTRANS */
#define LEON_DSU_TRCC_AHB_HTRANS_MASK         (0x3<<LEON_DSU_TRCC_AHB_HTRANS_SHIFT)
#define LEON_DSU_TRCC_AHB_HSIZE_SHIFT         (10)     /* AHB HSIZE */
#define LEON_DSU_TRCC_AHB_HSIZE_MASK          (0x7<<LEON_DSU_TRCC_AHB_HSIZE_SHIFT)
#define LEON_DSU_TRCC_AHB_HBURST_SHIFT        (7)      /* AHB HBURST */
#define LEON_DSU_TRCC_AHB_HBURST_MASK         (0x7<<LEON_DSU_TRCC_AHB_HBURST_SHIFT)
#define LEON_DSU_TRCC_AHB_HMASTER_SHIFT       (3)      /* AHB HMASTER */
#define LEON_DSU_TRCC_AHB_HMASTER_MASK        (0xf<<LEON_DSU_TRCC_AHB_HMASTER_SHIFT)
#define LEON_DSU_TRCC_AHB_HMASTLOCK_SHIFT     (2)      /* AHB HMASTLOCK */
#define LEON_DSU_TRCC_AHB_HMASTLOCK           (1<<LEON_DSU_TRCC_AHB_HMASTLOCK_SHIFT)
#define LEON_DSU_TRCC_AHB_HRESP_SHIFT         (0)      /* AHB HRESP */
#define LEON_DSU_TRCC_AHB_HRESP_MASK          (0x3<<LEON_DSU_TRCC_AHB_HRESP_SHIFT)

#define LEON_DSU_TRCC_INST_LDSTPAR_SHIFT      (0)      /* Load/Store parameters - Instruction result, Store address or Store data */
#define LEON_DSU_TRCC_INST_LDSTPAR_MASK       (0xffffffff<<LEON_DSU_TRCC_INST_LDSTPAR_SHIFT)
/* --- DSU Trace 32-63bit --------------------------------------------------- */
#define LEON_DSU_TRCB_AHB_DATA_SHIFT          (0)      /* AHB HRDATA or HWDATA */
#define LEON_DSU_TRCB_AHB_DATA_MASK           (0xffffffff<<LEON_DSU_TRCB_AHB_DATA_SHIFT)

#define LEON_DSU_TRCB_INST_PC_SHIFT           (0)      /* Program counter (2 lsb bits removed since they are always zero) */
#define LEON_DSU_TRCB_INST_PC_MASK            (0xfffffffc<<LEON_DSU_TRCB_INST_PC_SHIFT)
#define LEON_DSU_TRCB_INST_ITRAP_SHIFT        (1)      /* Set to ‘1’ if traced instruction trapped */
#define LEON_DSU_TRCB_INST_ITRAP              (1<<LEON_DSU_TRCB_INST_ITRAP_SHIFT)
#define LEON_DSU_TRCB_INST_PEMODE_SHIFT       (0)      /* Set to ‘1’ if the traced instruction caused processor error mode */
#define LEON_DSU_TRCB_INST_PEMODE             (1<<LEON_DSU_TRCB_INST_PEMODE_SHIFT)
/* --- DSU Trace 0-31bit --------------------------------------------------- */
#define LEON_DSU_TRCA_AHB_ADDR_SHIFT          (0)      /* AHB HADDR */
#define LEON_DSU_TRCA_AHB_ADDR_MASK           (0xffffffff<<LEON_DSU_TRCA_AHB_ADDR_SHIFT)

#define LEON_DSU_TRCA_INST_OPCODE_SHIFT       (0)      /* Instruction opcode */
#define LEON_DSU_TRCA_INST_OPCODE_MASK        (0xffffffff<<LEON_DSU_TRCA_INST_OPCODE_SHIFT)

/* --- DSU L2MT Reset Register ---------------------------------------------------- */
#define LEON_DSU_MTCTRL_RST_SHIFT    (0)        /* sw reset */
#define LEON_DSU_MTCTRL_RST          (1<<LEON_DSU_MTCTRL_RST_SHIFT)
#define LEON_DSU_MTCTRL_BRK_SHIFT    (1)        /* sw dsu break */
#define LEON_DSU_MTCTRL_BRK          (1<<LEON_DSU_MTCTRL_BRK_SHIFT)

/* --- DSU Trap Register ---------------------------------------------------- */
#define LEON_DSU_TRAP_TT_SHIFT    (4)        /* 8-bit SPARC trap type */
#define LEON_DSU_TRAP_TT_MASK     (255<<LEON_DSU_TRAP_TT_SHIFT)
#define LEON_DSU_TRAP_EM_SHIFT    (12)      /* Set if the trap would have cause the processor to enter error mode. */
#define LEON_DSU_TRAP_EM          (1<<LEON_DSU_TRAP_EM_SHIFT)


/* --- Sparc v8 IU Registers ------------------------------------------------ */
#define SPARC_V8_REG_G(x)           (0+x)
#define SPARC_V8_REG_O(x)           (8+x)
#define SPARC_V8_REG_L(x)           (16+x)
#define SPARC_V8_REG_I(x)           (24+x)
#define SPARC_V8_REG_SP             (SPARC_V8_REG_O(6))
#define SPARC_V8_REG_FP             (SPARC_V8_REG_I(6))

/* --- Sparc v8 PSR (Processor State Register) ------------------------------ */
#define SPARC_V8_PSR_CWP_SHIFT      (0)       /* Current window pointer */
#define SPARC_V8_PSR_CWP_MASK       (31<<SPARC_V8_PSR_CWP_SHIFT)
#define SPARC_V8_PSR_ET_SHIFT       (5)       /* Traps are enabled */
#define SPARC_V8_PSR_ET             (1<<SPARC_V8_PSR_ET_SHIFT)
#define SPARC_V8_PSR_PS_SHIFT       (6)       /* Previous value of S at time of the most recent trap */
#define SPARC_V8_PSR_PS             (1<<SPARC_V8_PSR_PS_SHIFT)
#define SPARC_V8_PSR_S_SHIFT        (7)       /* Processor is in supervisor mode */
#define SPARC_V8_PSR_S              (1<<SPARC_V8_PSR_S_SHIFT)
#define SPARC_V8_PSR_PIL_SHIFT      (8)       /* Processor interrupt level */
#define SPARC_V8_PSR_PIL_MASK       (15<<SPARC_V8_PSR_PIL_SHIFT)
#define SPARC_V8_PSR_EF_SHIFT       (12)      /* Floating-point is enabled */
#define SPARC_V8_PSR_EF             (1<<SPARC_V8_PSR_EF_SHIFT)
#define SPARC_V8_PSR_EC_SHIFT       (13)      /* Coprocesor is enabled */
#define SPARC_V8_PSR_EC             (1<<SPARC_V8_PSR_EC_SHIFT)
#define SPARC_V8_PSR_ICC_C_SHIFT    (20)   /* ICC bit Carry */
#define SPARC_V8_PSR_ICC_C          (1<<SPARC_V8_PSR_ICC_C_SHIFT)
#define SPARC_V8_PSR_ICC_V_SHIFT    (21)   /* ICC bit Overflow */
#define SPARC_V8_PSR_ICC_V          (1<<SPARC_V8_PSR_ICC_V_SHIFT)
#define SPARC_V8_PSR_ICC_Z_SHIFT    (22)   /* ICC bit Zero */
#define SPARC_V8_PSR_ICC_Z          (1<<SPARC_V8_PSR_ICC_Z_SHIFT)
#define SPARC_V8_PSR_ICC_N_SHIFT    (23)   /* ICC bit Negative */
#define SPARC_V8_PSR_ICC_N          (1<<SPARC_V8_PSR_ICC_N_SHIFT)
#define SPARC_V8_PSR_VER_SHIFT      (24)      /* Implementation version */
#define SPARC_V8_PSR_VER_MASK       (15<<SPARC_V8_PSR_VER_SHIFT)
#define SPARC_V8_PSR_IMPL_SHIFT     (28)      /* Implementation ID */
#define SPARC_V8_PSR_IMPL_MASK      (15<<SPARC_V8_PSR_IMPL_SHIFT)

#ifndef __ASSEMBLY__
enum leon_sparc_implementor_ids {
	SPARC_V8_IMPL_FUJITSU = 0,
	SPARC_V8_IMPL_CYPRESS = 1,
	SPARC_V8_IMPL_BIT     = 2,
	SPARC_V8_IMPL_LSIL    = 3,
	SPARC_V8_IMPL_TI      = 4,
	SPARC_V8_IMPL_MSSC    = 5,
	SPARC_V8_IMPL_PHILIPS = 6,
	SPARC_V8_IMPL_HVDC    = 7,
	SPARC_V8_IMPL_SPEC    = 8,
	SPARC_V8_IMPL_WEITEK  = 9,
};
#endif /* __ASSEMBLY__ */

/* --- Sparc v8 TBR (Trap Base Register) ------------------------------------ */
#define SPARC_V8_TBR_TT_SHIFT       (4)       /* Trap Type - This 8-bit field is written by the hardware when a trap occurs. */
#define SPARC_V8_TBR_TT_MASK        (0xFF<<SPARC_V8_TBR_TT_SHIFT)
#define SPARC_V8_TBR_TBA_SHIFT      (12)      /* Trap base address - It contains the most-significant 20 bits of the trap table address. */
#define SPARC_V8_TBR_TBA_MASK       (0xFFFFF<<SPARC_V8_TBR_TBA_SHIFT)

/* --- Sparc v8 FSR (FPU State Register) ------------------------------------ */

#define SPARC_V8_FSR_CEXC_MASK      (31<<0)   /* FP current exception - mask */
#define SPARC_V8_FSR_CEXC_SHIFT     (0)       /* FP current exception - shift */
#define SPARC_V8_FSR_CEXC_NXC       (1<<0)    /* FP current exception - INEXACT */
#define SPARC_V8_FSR_CEXC_DZC       (1<<1)    /* FP current exception - DIVISION BY ZERO */
#define SPARC_V8_FSR_CEXC_UFC       (1<<2)    /* FP current exception - UNDERFLOW */
#define SPARC_V8_FSR_CEXC_OFC       (1<<3)    /* FP current exception - OVERFLOW */
#define SPARC_V8_FSR_CEXC_NVC       (1<<4)    /* FP current exception - INVALID */
#define SPARC_V8_FSR_AEXC_MASK      (31<<5)   /* FP accrued exception - mask */
#define SPARC_V8_FSR_AEXC_SHIFT     (5)       /* FP accrued exception - shift */
#define SPARC_V8_FSR_AEXC_NXA       (1<<5)    /* FP accrued exception - INEXACT */
#define SPARC_V8_FSR_AEXC_DZA       (1<<6)    /* FP accrued exception - DIVISION BY ZERO */
#define SPARC_V8_FSR_AEXC_UFA       (1<<7)    /* FP accrued exception - UNDERFLOW */
#define SPARC_V8_FSR_AEXC_OFA       (1<<8)    /* FP accrued exception - OVERFLOW */
#define SPARC_V8_FSR_AEXC_NVA       (1<<9)    /* FP accrued exception - INVALID */
#define SPARC_V8_FSR_FCC_MASK       (3<<10)   /* FP condition codes - mask (updated by FP instr. FCMP,FCMPE) */
#define SPARC_V8_FSR_FCC_SHIFT      (10)      /* FP condition codes - shift */
#define SPARC_V8_FSR_QNE            (1<<13)   /* FP deferred-trap queue is not empty */
#define SPARC_V8_FSR_FTT_MASK       (7<<14)   /* FPU Trap Type - mask */
#define SPARC_V8_FSR_FTT_SHIFT      (14)      /* FPU Trap Type - shift */
#define SPARC_V8_FSR_VER_MASK       (7<<17)   /* FPU version - mask (version=7 means FPU not present) */
#define SPARC_V8_FSR_VER_SHIFT      (17)      /* FPU version - shift */
#define SPARC_V8_FSR_NS             (1<<22)   /* Non standard FP - results may not correspond to ANSI/IEEE Standard 754-1985 */
#define SPARC_V8_FSR_TEM_MASK       (31<<23)  /* Trap enable mask - mask */
#define SPARC_V8_FSR_TEM_SHIFT      (23)      /* Trap enable mask - shift */
#define SPARC_V8_FSR_TEM_NXM        (1<<23)   /* Trap enable mask - INEXACT */
#define SPARC_V8_FSR_TEM_DZM        (1<<24)   /* Trap enable mask - DIVISION BY ZERO */
#define SPARC_V8_FSR_TEM_UFM        (1<<25)   /* Trap enable mask - UNDERFLOW */
#define SPARC_V8_FSR_TEM_OFM        (1<<26)   /* Trap enable mask - OVERFLOW */
#define SPARC_V8_FSR_TEM_NVM        (1<<27)   /* Trap enable mask - INVALID */
#define SPARC_V8_FSR_RD_MASK        (3<<30)   /* Rounding direction - mask */
#define SPACR_V8_FSR_RD_SHIFT       (30)      /* Rounding direction - shift */

#ifndef __ASSEMBLY__
enum leon_fpu_rounding_directions {
	SPARC_V8_FPU_RD_NEAREST  = 0,
	SPARC_V8_FPU_RD_0        = 1,
	SPARC_V8_FPU_RD_POS_INF  = 2,
	SPARC_V8_FPU_RD_NEG_INF  = 3,
};

enum leon_fpu_trap_types {
	SPARC_V8_FPU_TT_NONE      = 0,
	SPARC_V8_FPU_TT_IEEE754   = 1,
	SPARC_V8_FPU_TT_UNFIN_OP  = 2,
	SPARC_V8_FPU_TT_UNIMPL_OP = 3,
	SPARC_V8_FPU_TT_SEQ_ERROR = 4,
	SPARC_V8_FPU_TT_HW_ERROR  = 5,
	SPARC_V8_FPU_TT_INVAL_REG = 6,
	SPARC_V8_FPU_TT_RESERVED  = 7,
};

enum leon_fpu_cond_codes {
	SPARC_V8_FPU_FCC_EQUAL     = 0,
	SPARC_V8_FPU_FCC_R1_LS_R2  = 1,
	SPARC_V8_FPU_FCC_R1_GT_R2  = 2,
	SPARC_V8_FPU_FCC_UNORDERED = 3,
};

#endif /* __ASSEMBLY__ */

/* --- Sparc Trap Types ------------- */
#define SPARC_V8_TT_WATCHPOINT     (0x0B)      /* BN in DSU Control Register causes this trap type */



/* === MACROs =============================================================== */
#define LEON_GET_BIT(ptr, bit)    (*ptr & (bit))
#define LEON_GET_VAL(ptr, base)   ((*ptr & (base##_MASK))>>(base##_SHIFT))

#endif /* LEON_OOCD_TARGET_HW_DESCRIPTION_HEADER_FILE */
