#pragma once
/* Samsung Galaxy S10+ (beyond2q, SM-G9750, SM8150 v2.1). Values from the
 * stock boot.img base DTB (SM8150 v2) plus dtbo entry 1 ("BEYOND2Q PROJECT
 * Rev16", board-id 0x10, the one ABL picks: androidboot.dtbo_idx=1). The SoC
 * blocks match guacamole (tools/fdt-audit.py: GIC, UFS, SPMI, USB, PHYs, CPU
 * MPIDRs all "ok"). Confirm the DDR banks against the live FDT. */

#define BOARD_NAME "beyond2q"

/* reserved-memory cont_splash_region = 0x9C400000 + 0x02000000 (32 MiB),
 * aliased by disp_rdump_region. Panel: Samsung S6E3HA9 (AMB644WF01), WQHD
 * 1440x3040, DSI cmd mode + DSC, dual DSI. Unlike guacamole's ABL, Samsung's
 * sboot leaves the command-mode datapath idle after the splash frame, so
 * writes to this buffer do not reach the panel until scanout is restarted
 * (see BEYOND2Q.md). */
#define FB_BASE		0x9c400000UL
#define FB_W		1440
#define FB_H		3040
#define FB_STRIDE	(FB_W * 4)

#define GICD_BASE	0x17a00000UL
#define GICR_BASE	0x17a60000UL
#define GICR_SIZE	0x100000UL
#define GICR_PE_SIZE	0x20000UL
#define VTIMER_INTID	19
#define PTIMER_INTID	18

/* restart_reason 0x77665500 sends this Samsung to DOWNLOAD mode (not ABL). */
#define IMEM_RESTART_REASON	0x146bf65cUL
#define PSHOLD			0x0c264000UL

#define UFS_BASE		0x01d84000UL
/* Same SoC as guacamole, where reading the gated UFS host stalls the bus
 * and the watchdog resets the phone. Keep the probe off. */
#define PROBE_UFS		0

#define BOARD_TLMM_PA		0x03000000UL
/* gpio104 is a reset/wake line on this board (live FDT), not a spare pin:
 * skip the probe's TLMM write test. */
#define BOARD_NO_TLMM_WRITE	1
#define BOARD_GPUCC_PA		0x02c90000UL

/* Reset-surviving scratch word: 0x18000000 into the kernel bank, as on
 * guacamole. Live FDT (2026-10-28): DDR 0x80000000+0x3cc00000,
 * 0xc0000000+0x80000000, 0x140000000+0xc0000000; only the high bank is free
 * of Samsung carve-outs, and the kernel goes there. */
#define MARK_PA			0x158000000UL
