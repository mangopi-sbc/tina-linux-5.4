/* sound\soc\sunxi\snd_sunxi_spdif.h
 * (C) Copyright 2021-2025
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Dby <dby@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef __SND_SUNXI_SPDIF_H
#define __SND_SUNXI_SPDIF_H

#include "snd_sunxi_spdif_rx61937.h"

#define	SUNXI_SPDIF_CTL		0x00
#define	SUNXI_SPDIF_TXCFG	0x04
#define	SUNXI_SPDIF_RXCFG	0x08
#define SUNXI_SPDIF_INT_STA	0x0C
#define	SUNXI_SPDIF_RXFIFO	0x10
#define	SUNXI_SPDIF_FIFO_CTL	0x14
#define	SUNXI_SPDIF_FIFO_STA	0x18
#define	SUNXI_SPDIF_INT		0x1C
#define SUNXI_SPDIF_TXFIFO	0x20
#define	SUNXI_SPDIF_TXCNT	0x24
#define	SUNXI_SPDIF_RXCNT	0x28
#define	SUNXI_SPDIF_TXCH_STA0	0x2C
#define	SUNXI_SPDIF_TXCH_STA1	0x30
#define	SUNXI_SPDIF_RXCH_STA0	0x34
#define	SUNXI_SPDIF_RXCH_STA1	0x38

#if IS_ENABLED(CONFIG_SND_SOC_SUNXI_SPDIF_RXIEC61937)
#define	SUNXI_SPDIF_EXP_VER	0x58
#else
#define	SUNXI_SPDIF_EXP_VER	0x40
#endif

#define SUNXI_SPDIF_REG_MAX	SUNXI_SPDIF_EXP_VER

/* SUNXI_SPDIF_CTL register */
#define	CTL_RESET		0
#define	CTL_GEN_EN		1
#define	CTL_LOOP_EN		2
#define	CTL_RESET_RX		0

/* SUNXI_SPDIF_TXCFG register */
#define	TXCFG_TXEN		0
/* Chan status generated form TX_CHSTA */
#define	TXCFG_CHAN_STA_EN	1
#define	TXCFG_SAMPLE_BIT	2
#define	TXCFG_CLK_DIV_RATIO	4
#define	TXCFG_DATA_TYPE		16
/* Only valid in PCM mode */
#define	TXCFG_ASS		17
#define	TXCFG_SINGLE_MOD	31

/* SUNXI_SPDIF_RXCFG register */
#define	RXCFG_RXEN		0
#define	RXCFG_CHSR_CP		1
#define	RXCFG_CHST_SRC		3
#define	RXCFG_LOCK_FLAG		4

/* SUNXI_SPDIF_FIFO_CTL register */
#define	FIFO_CTL_RXOM		0
#define	FIFO_CTL_TXIM		2
#define	FIFO_CTL_RXTL		4
#define	FIFO_CTL_TXTL		12
#define	SPDIF_RX_SYNC_EN	20
#define	SPDIF_RX_SYNC_EN_START	21
#define	FIFO_CTL_FRX		29
#define	FIFO_CTL_FTX		30
#define	FIFO_CTL_HUBEN		31
#define	CTL_TXTL_MASK		0xFF
#define	CTL_TXTL_DEFAULT	0x40
#define	CTL_RXTL_MASK		0x7F
#define	CTL_RXTL_DEFAULT	0x20

/* SUNXI_SPDIF_FIFO_STA register */
#define	FIFO_STA_RXA_CNT	0
#define	FIFO_STA_RXA		15
#define	FIFO_STA_TXA_CNT	16
#define	FIFO_STA_TXE		31

/* SUNXI_SPDIF_INT register */
#define	INT_RXAIEN		0
#define	INT_RXOIEN		1
#define	INT_RXDRQEN		2
#define	INT_TXEIEN		4
#define	INT_TXOIEN		5
#define	INT_TXUIEN		6
#define	INT_TXDRQEN		7
#define	INT_RXPAREN		16
#define	INT_RXUNLOCKEN		17
#define	INT_RXLOCKEN		18

/* SUNXI_SPDIF_INT_STA	*/
#define	INT_STA_RXA		0
#define	INT_STA_RXO		1
#define	INT_STA_TXE		4
#define	INT_STA_TXO		5
#define	INT_STA_TXU		6
#define	INT_STA_RXPAR		16
#define	INT_STA_RXUNLOCK	17
#define	INT_STA_RXLOCK		18

/* SUNXI_SPDIF_TXCH_STA0 register */
#define	TXCHSTA0_PRO		0
#define	TXCHSTA0_AUDIO		1
#define	TXCHSTA0_CP		2
#define	TXCHSTA0_EMPHASIS	3
#define	TXCHSTA0_MODE		6
#define	TXCHSTA0_CATACOD	8
#define	TXCHSTA0_SRCNUM		16
#define	TXCHSTA0_CHNUM		20
#define	TXCHSTA0_SAMFREQ	24
#define	TXCHSTA0_CLK		28

/* SUNXI_SPDIF_TXCH_STA1 register */
#define	TXCHSTA1_MAXWORDLEN	0
#define	TXCHSTA1_SAMWORDLEN	1
#define	TXCHSTA1_ORISAMFREQ	4
#define	TXCHSTA1_CGMSA		8

/* SUNXI_SPDIF_RXCH_STA0 register */
#define	RXCHSTA0_PRO		0
#define	RXCHSTA0_AUDIO		1
#define	RXCHSTA0_CP		2
#define	RXCHSTA0_EMPHASIS	3
#define	RXCHSTA0_MODE		6
#define	RXCHSTA0_CATACOD	8
#define	RXCHSTA0_SRCNUM		16
#define	RXCHSTA0_CHNUM		20
#define	RXCHSTA0_SAMFREQ	24
#define	RXCHSTA0_CLK		28

/* SUNXI_SPDIF_RXCH_STA1 register */
#define	RXCHSTA1_MAXWORDLEN	0
#define	RXCHSTA1_SAMWORDLEN	1
#define	RXCHSTA1_ORISAMFREQ	4
#define	RXCHSTA1_CGMSA		8

/* SUNXI_SPDIF_VER register */
#define MOD_VER			0

struct sunxi_spdif_mem_info {
	struct resource res;
	void __iomem *membase;
	struct resource *memregion;
	struct regmap *regmap;
};

struct sunxi_spdif_clk_info {
	struct clk *clk_pll_audio;
	struct clk *clk_pll_audio_4x;
	struct clk *clk_parent;
	struct clk *clk_spdif;	/* tx & rx for 50iw9 */

	unsigned int pll_fs;

	struct clk *clk_bus;
	struct reset_control *clk_rst;
};

struct sunxi_spdif_pinctl_info {
	struct pinctrl *pinctrl;
	struct pinctrl_state *pinstate;
	struct pinctrl_state *pinstate_sleep;

	bool pinctrl_used;
};

struct sunxi_spdif_dts_info {
	/* value must be (2^n)Kbyte */
	size_t playback_cma;
	size_t playback_fifo_size;
	size_t capture_cma;
	size_t capture_fifo_size;

	bool tx_hub_en;		/* tx_hub */

	/* components func -> rx_sync */
	bool rx_sync_en;	/* read from dts */
	bool rx_sync_ctl;
	int rx_sync_id;
	rx_sync_domain_t rx_sync_domain;
};

struct sunxi_spdif_info {
	struct platform_device *pdev;

	struct sunxi_spdif_mem_info mem_info;
	struct sunxi_spdif_clk_info clk_info;
	struct sunxi_spdif_pinctl_info pin_info;
	struct sunxi_spdif_dts_info dts_info;

	struct sunxi_dma_params playback_dma_param;
	struct sunxi_dma_params capture_dma_param;
};

#endif /* __SND_SUNXI_SPDIF_H */
