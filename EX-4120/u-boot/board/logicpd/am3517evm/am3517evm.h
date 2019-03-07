/*
 * am3517evm.h - Header file for the AM3517 EVM.
 *
 * Author: Vaibhav Hiremath <hvaibhav@ti.com>
 *
 * Based on ti/evm/evm.h
 *
 * Copyright (C) 2010
 * Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _AM3517EVM_H_
#define _AM3517EVM_H_

const omap3_sysinfo sysinfo = {
	DDR_DISCRETE,
	"AM3517EVM Board",
	"NAND",
};

/*
 * IEN  - Input Enable
 * IDIS - Input Disable
 * PTD  - Pull type Down
 * PTU  - Pull type Up
 * DIS  - Pull type selection is inactive
 * EN   - Pull type selection is active
 * M0   - Mode 0
 * The commented string gives the final mux configuration for that pin
 */


#ifdef CONFIG_APP_MAIN	/* AppCPU main */
#define MUX_AM3517EVM() \
	/* SDRC */\
	MUX_VAL(CP(SDRC_D0),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D1),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D2),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D3),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D4),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D5),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D6),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D7),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D8),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D9),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D10),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D11),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D12),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D13),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D14),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D15),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D16),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D17),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D18),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D19),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D20),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D21),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D22),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D23),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D24),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D25),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D26),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D27),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D28),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D29),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D30),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D31),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_CLK),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_DQS0),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_CKE0),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(SDRC_DQS1),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_DQS2),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_DQS3),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_DQS0N),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(SDRC_DQS1N),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(SDRC_DQS2N),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(SDRC_DQS3N),		(IEN  | PTD | EN  | M0)) \
	/*sdrc_strben_dly0*/\
	MUX_VAL(CP(STRBEN_DLY0),	(IEN  | PTD | EN  | M0)) \
	 /*sdrc_strben_dly1*/\
	MUX_VAL(CP(STRBEN_DLY1),	(IEN  | PTD | EN  | M0)) \
	/* GPMC */\
	MUX_VAL(CP(GPMC_A1),		(IDIS | PTD | EN  | M7)) /*GPIO_34*/\
	MUX_VAL(CP(GPMC_A2),		(IDIS | PTD | EN  | M7)) /*GPIO_35*/\
	MUX_VAL(CP(GPMC_A3),		(IDIS | PTD | EN  | M7)) /*GPIO_36*/\
	MUX_VAL(CP(GPMC_A4),		(IDIS | PTD | EN  | M7)) /*GPIO_37*/\
	MUX_VAL(CP(GPMC_A5),		(IDIS | PTD | EN  | M7)) /*GPIO_38*/\
	MUX_VAL(CP(GPMC_A6),		(IDIS | PTD | EN  | M7)) /*GPIO_39*/\
	MUX_VAL(CP(GPMC_A7),		(IDIS | PTD | EN  | M7)) /*GPIO_40*/\
	MUX_VAL(CP(GPMC_A8),		(IDIS | PTD | EN  | M7)) /*GPIO_41*/\
	MUX_VAL(CP(GPMC_A9),		(IDIS | PTD | EN  | M7)) /*GPIO_42*/\
	MUX_VAL(CP(GPMC_A10),		(IDIS | PTD | EN  | M7)) /*GPIO_43*/\
	MUX_VAL(CP(GPMC_D0),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D1),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D2),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D3),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D4),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D5),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D6),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D7),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D8),		(IDIS | PTD | EN  | M7)) /*GPIO_44*/\
	MUX_VAL(CP(GPMC_D9),		(IDIS | PTD | EN  | M7)) /*GPIO_45*/\
	MUX_VAL(CP(GPMC_D10),		(IDIS | PTD | EN  | M7)) /*GPIO_46*/\
	MUX_VAL(CP(GPMC_D11),		(IDIS | PTD | EN  | M7)) /*GPIO_47*/\
	MUX_VAL(CP(GPMC_D12),		(IDIS | PTD | EN  | M7)) /*GPIO_48*/\
	MUX_VAL(CP(GPMC_D13),		(IDIS | PTD | EN  | M7)) /*GPIO_49*/\
	MUX_VAL(CP(GPMC_D14),		(IDIS | PTD | EN  | M7)) /*GPIO_50*/\
	MUX_VAL(CP(GPMC_D15),		(IDIS | PTD | EN  | M7)) /*GPIO_51*/\
	MUX_VAL(CP(GPMC_NCS0),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NCS1),		(IDIS | PTD | EN  | M7)) /*GPIO_52*/\
	MUX_VAL(CP(GPMC_NCS2),		(IDIS | PTD | EN  | M7)) /*GPIO_53*/\
	MUX_VAL(CP(GPMC_NCS3),		(IDIS | PTD | EN  | M7)) /*GPIO_54*/\
	MUX_VAL(CP(GPMC_NCS4),		(IDIS | PTD | EN  | M7)) /*GPIO_55*/\
	MUX_VAL(CP(GPMC_NCS5),		(IDIS | PTD | EN  | M7)) /*GPIO_56*/\
	MUX_VAL(CP(GPMC_NCS6),		(IDIS | PTD | DIS | M7)) /*GPIO_57*/\
	MUX_VAL(CP(GPMC_NCS7),		(IDIS | PTD | EN  | M7)) /*GPIO_58*/\
	MUX_VAL(CP(GPMC_CLK),		(IDIS | PTD | EN  | M7)) /*GPIO_59*/\
	MUX_VAL(CP(GPMC_NADV_ALE),	(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(GPMC_NOE),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(GPMC_NWE),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(GPMC_NBE0_CLE),	(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NBE1),		(IDIS | PTD | EN  | M7)) /*GPIO_61*/\
	MUX_VAL(CP(GPMC_NWP),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(GPMC_WAIT0),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(GPMC_WAIT1),		(IDIS | PTD | EN  | M7)) /*GPIO_63*/\
	MUX_VAL(CP(GPMC_WAIT2),		(IDIS | PTD | EN  | M7)) /*GPIO_64*/\
	MUX_VAL(CP(GPMC_WAIT3),		(IDIS | PTD | EN  | M7)) /*GPIO_65*/\
	/* DSS */\
	MUX_VAL(CP(DSS_PCLK),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_HSYNC),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_VSYNC),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_ACBIAS),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA0),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA1),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA2),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA3),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA4),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA5),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA6),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA7),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA8),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA9),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA10),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA11),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA12),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA13),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA14),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA15),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA16),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA17),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA18),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA19),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA20),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA21),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA22),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA23),		(IDIS | PTD | DIS | M0)) \
	/* CCDC */\
	MUX_VAL(CP(CCDC_PCLK),		(IDIS | PTD | EN  | M7)) /*GPIO_94*/\
	MUX_VAL(CP(CCDC_FIELD),		(IDIS | PTD | DIS | M4)) /*GPIO_95*/\
	MUX_VAL(CP(CCDC_HD),		(IDIS | PTD | EN  | M7)) /*GPIO_96*/\
	MUX_VAL(CP(CCDC_VD),		(IDIS | PTD | EN  | M7)) /*GPIO_97*/\
	MUX_VAL(CP(CCDC_WEN),		(IEN  | PTD | EN  | M2)) /*UART4_RX*/\
	MUX_VAL(CP(CCDC_DATA0),		(IEN  | PTU | DIS | M4)) /*GPIO_99 : MSSELM*/\
	MUX_VAL(CP(CCDC_DATA1),		(IEN  | PTD | EN  | M7)) /*GPIO_100*/\
	MUX_VAL(CP(CCDC_DATA2),		(IDIS | PTD | DIS | M4)) /*GPIO_101 : ABLENM*/\
	MUX_VAL(CP(CCDC_DATA3),		(IDIS | PTD | DIS | M4)) /*GPIO_102 : ABLENS*/\
	MUX_VAL(CP(CCDC_DATA4),		(IDIS | PTD | DIS | M4)) /*GPIO_103 : ETnRST*/\
	MUX_VAL(CP(CCDC_DATA5),		(IDIS | PTD | EN  | M7)) /*GPIO_104*/\
	MUX_VAL(CP(CCDC_DATA6),		(IDIS | PTD | DIS | M4)) /*GPIO_105 : DVIPD*/\
	MUX_VAL(CP(CCDC_DATA7),		(IDIS | PTD | DIS | M4)) /*GPIO_106 : VDACPS*/\
	/* RMII */\
	MUX_VAL(CP(RMII_MDIO_DATA),	(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(RMII_MDIO_CLK),	(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(RMII_RXD0)	,	(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(RMII_RXD1),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(RMII_CRS_DV),	(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(RMII_RXER),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(RMII_TXD0),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(RMII_TXD1),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(RMII_TXEN),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(RMII_50MHZ_CLK),	(IEN  | PTD | EN  | M0)) \
	/* McBSP2 */\
	MUX_VAL(CP(MCBSP2_FSX),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(MCBSP2_CLKX),	(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(MCBSP2_DR),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(MCBSP2_DX),		(IDIS | PTD | EN  | M0)) \
	/* MMC */\
	MUX_VAL(CP(MMC1_CLK),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MMC1_CMD),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MMC1_DAT0),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MMC1_DAT1),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MMC1_DAT2),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MMC1_DAT3),		(IEN  | PTU | DIS | M0)) \
	/* WriteProtect */\
	MUX_VAL(CP(MMC1_DAT4),		(IEN  | PTU | DIS | M4)) /*GPIO_126 : SDWPM*/\
	MUX_VAL(CP(MMC1_DAT5),		(IEN  | PTU | DIS | M4)) /*GPIO_127 : SDDETM*/\
	MUX_VAL(CP(MMC1_DAT6),		(IDIS | PTD | EN  | M7)) /*GPIO_128*/\
	MUX_VAL(CP(MMC1_DAT7),		(IDIS | PTD | EN  | M7)) /*GPIO_129*/\
	MUX_VAL(CP(MMC2_CLK),		(IDIS | PTD | EN  | M7)) /*GPIO_130*/\
	MUX_VAL(CP(MMC2_CMD),		(IDIS | PTD | EN  | M7)) /*GPIO_131*/\
	MUX_VAL(CP(MMC2_DAT0),		(IDIS | PTD | EN  | M7)) /*GPIO_132*/\
	MUX_VAL(CP(MMC2_DAT1),		(IDIS | PTD | EN  | M7)) /*GPIO_133*/\
	MUX_VAL(CP(MMC2_DAT2),		(IDIS | PTD | EN  | M7)) /*GPIO_134*/\
	MUX_VAL(CP(MMC2_DAT3),		(IDIS | PTD | EN  | M7)) /*GPIO_135*/\
	/* WriteProtect */\
	MUX_VAL(CP(MMC2_DAT4),		(IDIS | PTD | EN  | M7)) /*GPIO_136*/\
	MUX_VAL(CP(MMC2_DAT5),		(IDIS | PTD | EN  | M7)) /*GPIO_137*/\
	MUX_VAL(CP(MMC2_DAT6),		(IDIS | PTD | EN  | M7)) /*GPIO_138*/\
	MUX_VAL(CP(MMC2_DAT7),		(IDIS | PTD | EN  | M7)) /*GPIO_139*/\
	/* McBSP3 */\
	MUX_VAL(CP(MCBSP3_DX),		(IDIS | PTD | EN  | M7)) /*GPIO_140*/\
	MUX_VAL(CP(MCBSP3_DR),		(IDIS | PTD | EN  | M7)) /*GPIO_141*/\
	MUX_VAL(CP(MCBSP3_CLKX),	(IDIS | PTD | EN  | M7)) /*GPIO_142*/\
	MUX_VAL(CP(MCBSP3_FSX),		(IDIS | PTD | EN  | M7)) /*GPIO_143*/\
	/* UART2 */\
	MUX_VAL(CP(UART2_CTS),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(UART2_RTS),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(UART2_TX),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(UART2_RX),		(IEN  | PTD | DIS | M0)) \
	/* UART1 */\
	MUX_VAL(CP(UART1_TX),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(UART1_RTS),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(UART1_CTS),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(UART1_RX),		(IEN  | PTD | DIS | M0)) \
	/* McBSP4 */\
	MUX_VAL(CP(MCBSP4_CLKX),	(IDIS | PTD | EN  | M7)) /*GPIO_152*/\
	MUX_VAL(CP(MCBSP4_DR),		(IEN  | PTD | EN  | M7)) /*GPIO_153*/\
	MUX_VAL(CP(MCBSP4_DX),		(IDIS | PTD | DIS | M4)) /*GPIO_154 : PDONEM*/\
	MUX_VAL(CP(MCBSP4_FSX),		(IDIS | PTD | EN  | M4)) /*GPIO_155 : DACnPD*/\
	/* McBSP */\
	MUX_VAL(CP(MCBSP1_CLKR),	(IEN  | PTU | EN  | M1)) /*GPIO_156 : MSTMP1*/\
	MUX_VAL(CP(MCBSP1_FSR),		(IEN  | PTD | DIS | M4)) /*GPIO_157 : BKC*/\
	MUX_VAL(CP(MCBSP1_DX),		(IEN  | PTU | EN  | M1)) /*GPIO_158 : WKUP1*/\
	MUX_VAL(CP(MCBSP1_DR),		(IEN  | PTD | DIS | M4)) /*GPIO_159 : APSHLT*/\
	MUX_VAL(CP(MCBSP_CLKS),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(MCBSP1_FSX),		(IEN  | PTU | EN  | M1)) /*GPIO_161 : WKUP0*/\
	MUX_VAL(CP(MCBSP1_CLKX),	(IDIS | PTD | DIS | M4)) /*GPIO_162 : AHLTM*/\
	/* UART3 */\
/*	MUX_VAL(CP(UART3_CTS_RCTX),	(IEN  | PTD | EN  | M7)) \
	MUX_VAL(CP(UART3_RTS_SD),	(IDIS | PTD | DIS | M7)) \
	MUX_VAL(CP(UART3_RX_IRRX),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(UART3_TX_IRTX),	(IDIS | PTD | DIS | M0)) */\
	MUX_VAL(CP(USB0_DRVVBUS),	(IEN  | PTU | EN  | M4)) /*GPIO_125*/\
	/* HECC */\
	MUX_VAL(CP(HECC1_TXD),		(IDIS | PTD | EN  | M7)) /*GPIO_130*/\
	MUX_VAL(CP(HECC1_RXD),		(IDIS | PTD | EN  | M7)) /*GPIO_131*/\
	/* I2C */\
	MUX_VAL(CP(I2C1_SCL),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(I2C1_SDA),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(I2C2_SCL),		(IDIS | PTU | EN  | M7)) /*GPIO_168*/\
	MUX_VAL(CP(I2C2_SDA),		(IDIS | PTU | EN  | M7)) /*GPIO_183*/\
	MUX_VAL(CP(I2C3_SCL),		(IDIS | PTU | EN  | M7)) /*GPIO_184*/\
	MUX_VAL(CP(I2C3_SDA),		(IDIS | PTU | EN  | M7)) /*GPIO_185*/\
	/* HDQ */\
	MUX_VAL(CP(HDQ_SIO),		(IDIS | PTD | EN  | M7)) /*GPIO_170*/\
	/* McSPI */\
	MUX_VAL(CP(MCSPI1_CLK),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MCSPI1_SIMO),	(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MCSPI1_SOMI),	(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MCSPI1_CS0),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MCSPI1_CS1),		(IEN  | PTU | DIS | M4)) /*GPIO_175 : SAREQ*/\
	MUX_VAL(CP(MCSPI1_CS2),		(IDIS | PTD | DIS | M4)) /*GPIO_176 : HS2EN*/\
	MUX_VAL(CP(MCSPI1_CS3),		(IDIS | PTD | EN  | M5)) /*mm_fsusb2_txdat*/\
	MUX_VAL(CP(MCSPI2_CLK),		(IDIS | PTD | EN  | M7)) /*GPIO_178*/\
	MUX_VAL(CP(MCSPI2_SIMO),	(IDIS | PTD | EN  | M7)) /*GPIO_179*/\
	MUX_VAL(CP(MCSPI2_SOMI),	(IDIS | PTD | EN  | M7)) /*GPIO_180*/\
	MUX_VAL(CP(MCSPI2_CS0),		(IDIS | PTD | EN  | M7)) /*GPIO_181*/\
	MUX_VAL(CP(MCSPI2_CS1),		(IDIS | PTD | DIS | M5)) /*mm_fsusb2_txen_n*/\
	/* Control and debug */\
	MUX_VAL(CP(SYS_NIRQ),		(IDIS | PTD | EN  | M7)) /*GPIO_0*/\
	MUX_VAL(CP(SYS_CLKOUT2),	(IDIS | PTD | EN  | M4)) /*GPIO_186*/\
	/* ETK (ES2 onwards) */\
	MUX_VAL(CP(ETK_CLK_ES2),	(IDIS | PTD | DIS | M4)) /*GPIO_12 : HS1EN*/\
	MUX_VAL(CP(ETK_CTL_ES2),	(IEN  | PTD | DIS | M5)) /*mm_fsusb1_rxdp*/\
	MUX_VAL(CP(ETK_D0_ES2),		(IEN  | PTD | DIS | M5)) /*mm_fsusb1_rxrcv*/\
	MUX_VAL(CP(ETK_D1_ES2),		(IEN  | PTD | DIS | M5)) /*mm_fsusb1_txse0*/\
	MUX_VAL(CP(ETK_D2_ES2),		(IEN  | PTD | DIS | M5)) /*mm_fsusb1_txdat*/\
	MUX_VAL(CP(ETK_D3_ES2),		(IDIS | PTD | DIS | M4)) /*GPIO_17 : US1SPD*/\
	MUX_VAL(CP(ETK_D4_ES2),		(IEN  | PTD | DIS | M4)) /*GPIO_18 : POLSEL*/\
	MUX_VAL(CP(ETK_D5_ES2),		(IEN  | PTD | DIS | M4)) /*GPIO_19 : HS1INT*/\
	MUX_VAL(CP(ETK_D6_ES2),		(IEN  | PTD | DIS | M4)) /*GPIO_20 : HS2INT*/\
	MUX_VAL(CP(ETK_D7_ES2),		(IEN  | PTD | DIS | M5)) /*mm_fsusb1_txen_n*/\
	MUX_VAL(CP(ETK_D8_ES2),		(IDIS | PTD | DIS | M4)) /*GPIO_22 : US1SUS*/\
	MUX_VAL(CP(ETK_D9_ES2),		(IEN  | PTD | DIS | M5)) /*mm_fsusb1_rxdm*/\
	MUX_VAL(CP(ETK_D10_ES2),	(IDIS | PTD | DIS | M4)) /*GPIO_24 : US2SUS*/\
	MUX_VAL(CP(ETK_D11_ES2),	(IEN  | PTD | DIS | M5)) /*mm_fsusb2_rxdp*/\
	MUX_VAL(CP(ETK_D12_ES2),	(IDIS | PTD | DIS | M4)) /*GPIO_26 : US2SPD*/\
	MUX_VAL(CP(ETK_D13_ES2),	(IEN  | PTD | DIS | M5)) /*mm_fsusb2_rxdm*/\
	MUX_VAL(CP(ETK_D14_ES2),	(IEN  | PTD | DIS | M5)) /*mm_fsusb2_rxrcv*/\
	MUX_VAL(CP(ETK_D15_ES2),	(IEN  | PTD | DIS | M5)) /*mm_fsusb2_txse0*/\
	MUX_VAL(CP(SYS_32K),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_CLKREQ),		(IDIS | PTD | EN  | M7)) /*GPIO_1*/\
	/*SYS_nRESWARM */\
	MUX_VAL(CP(SYS_NRESWARM),     	(IDIS | PTU | DIS | M7)) /*GPIO_30*/\
	MUX_VAL(CP(SYS_BOOT0),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_BOOT1),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_BOOT2),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_BOOT3),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_BOOT4),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_BOOT5),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_BOOT6),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_BOOT7),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_BOOT8),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_CLKOUT1),	(IEN  | PTD | EN  | M4)) /*GPIO_10 : LCDREQ*/\
	/* JTAG */\
	MUX_VAL(CP(JTAG_nTRST),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_TCK),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_RTCK),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_TMS),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_TDI),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_TDO),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_EMU0),		(IDIS | PTD | EN  | M7)) /*GPIO_11*/\
	MUX_VAL(CP(JTAG_EMU1),		(IDIS | PTD | EN  | M7)) /*GPIO_31*/\

#else /* AppCPU sub */
#define MUX_AM3517EVM() \
	/* SDRC */\
	MUX_VAL(CP(SDRC_D0),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D1),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D2),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D3),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D4),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D5),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D6),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D7),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D8),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D9),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D10),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D11),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D12),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D13),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D14),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D15),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D16),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D17),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D18),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D19),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D20),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D21),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D22),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D23),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D24),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D25),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D26),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D27),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D28),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D29),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D30),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D31),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_CLK),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_DQS0),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_CKE0),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(SDRC_DQS1),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_DQS2),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_DQS3),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_DQS0N),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(SDRC_DQS1N),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(SDRC_DQS2N),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(SDRC_DQS3N),		(IEN  | PTD | EN  | M0)) \
	/*sdrc_strben_dly0*/\
	MUX_VAL(CP(STRBEN_DLY0),	(IEN  | PTD | EN  | M0)) \
	 /*sdrc_strben_dly1*/\
	MUX_VAL(CP(STRBEN_DLY1),	(IEN  | PTD | EN  | M0)) \
	/* GPMC */\
	MUX_VAL(CP(GPMC_A1),		(IDIS | PTD | EN  | M7)) /*GPIO_34*/\
	MUX_VAL(CP(GPMC_A2),		(IDIS | PTD | EN  | M7)) /*GPIO_35*/\
	MUX_VAL(CP(GPMC_A3),		(IDIS | PTD | EN  | M7)) /*GPIO_36*/\
	MUX_VAL(CP(GPMC_A4),		(IDIS | PTD | EN  | M7)) /*GPIO_37*/\
	MUX_VAL(CP(GPMC_A5),		(IDIS | PTD | EN  | M7)) /*GPIO_38*/\
	MUX_VAL(CP(GPMC_A6),		(IDIS | PTD | EN  | M7)) /*GPIO_39*/\
	MUX_VAL(CP(GPMC_A7),		(IDIS | PTD | EN  | M7)) /*GPIO_40*/\
	MUX_VAL(CP(GPMC_A8),		(IDIS | PTD | EN  | M7)) /*GPIO_41*/\
	MUX_VAL(CP(GPMC_A9),		(IDIS | PTD | EN  | M7)) /*GPIO_42*/\
	MUX_VAL(CP(GPMC_A10),		(IDIS | PTD | EN  | M7)) /*GPIO_43*/\
	MUX_VAL(CP(GPMC_D0),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D1),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D2),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D3),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D4),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D5),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D6),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D7),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D8),		(IDIS | PTD | EN  | M7)) /*GPIO_44*/\
	MUX_VAL(CP(GPMC_D9),		(IDIS | PTD | EN  | M7)) /*GPIO_45*/\
	MUX_VAL(CP(GPMC_D10),		(IDIS | PTD | EN  | M7)) /*GPIO_46*/\
	MUX_VAL(CP(GPMC_D11),		(IDIS | PTD | EN  | M7)) /*GPIO_47*/\
	MUX_VAL(CP(GPMC_D12),		(IDIS | PTD | EN  | M7)) /*GPIO_48*/\
	MUX_VAL(CP(GPMC_D13),		(IDIS | PTD | EN  | M7)) /*GPIO_49*/\
	MUX_VAL(CP(GPMC_D14),		(IDIS | PTD | EN  | M7)) /*GPIO_50*/\
	MUX_VAL(CP(GPMC_D15),		(IDIS | PTD | EN  | M7)) /*GPIO_51*/\
	MUX_VAL(CP(GPMC_NCS0),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NCS1),		(IDIS | PTD | EN  | M7)) /*GPIO_52*/\
	MUX_VAL(CP(GPMC_NCS2),		(IDIS | PTD | EN  | M7)) /*GPIO_53*/\
	MUX_VAL(CP(GPMC_NCS3),		(IDIS | PTD | EN  | M7)) /*GPIO_54*/\
	MUX_VAL(CP(GPMC_NCS4),		(IDIS | PTD | EN  | M7)) /*GPIO_55*/\
	MUX_VAL(CP(GPMC_NCS5),		(IDIS | PTD | EN  | M7)) /*GPIO_56*/\
	MUX_VAL(CP(GPMC_NCS6),		(IDIS | PTD | DIS | M7)) /*GPIO_57*/\
	MUX_VAL(CP(GPMC_NCS7),		(IDIS | PTD | EN  | M7)) /*GPIO_58*/\
	MUX_VAL(CP(GPMC_CLK),		(IDIS | PTD | EN  | M7)) /*GPIO_59*/\
	MUX_VAL(CP(GPMC_NADV_ALE),	(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(GPMC_NOE),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(GPMC_NWE),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(GPMC_NBE0_CLE),	(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NBE1),		(IDIS | PTD | EN  | M7)) /*GPIO_61*/\
	MUX_VAL(CP(GPMC_NWP),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(GPMC_WAIT0),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(GPMC_WAIT1),		(IDIS | PTD | EN  | M7)) /*GPIO_63*/\
	MUX_VAL(CP(GPMC_WAIT2),		(IDIS | PTD | EN  | M7)) /*GPIO_64*/\
	MUX_VAL(CP(GPMC_WAIT3),		(IDIS | PTD | EN  | M7)) /*GPIO_65*/\
	/* DSS */\
	MUX_VAL(CP(DSS_PCLK),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_HSYNC),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_VSYNC),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_ACBIAS),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA0),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA1),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA2),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA3),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA4),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA5),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA6),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA7),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA8),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA9),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA10),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA11),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA12),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA13),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA14),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA15),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA16),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA17),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA18),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA19),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA20),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA21),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA22),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA23),		(IDIS | PTD | DIS | M0)) \
	/* CCDC */\
	MUX_VAL(CP(CCDC_PCLK),		(IDIS | PTD | EN  | M7)) /*GPIO_94*/\
	MUX_VAL(CP(CCDC_FIELD),		(IDIS | PTD | EN  | M7)) /*GPIO_95*/\
	MUX_VAL(CP(CCDC_HD),		(IDIS | PTD | EN  | M7)) /*GPIO_96*/\
	MUX_VAL(CP(CCDC_VD),		(IDIS | PTD | EN  | M7)) /*GPIO_97*/\
	MUX_VAL(CP(CCDC_WEN),		(IDIS | PTD | EN  | M7)) /*GPIO_98*/\
	MUX_VAL(CP(CCDC_DATA0),		(IEN  | PTU | DIS | M4)) /*GPIO_99 : MSSELS*/\
	MUX_VAL(CP(CCDC_DATA1),		(IEN  | PTD | EN  | M7)) /*GPIO_100*/\
	MUX_VAL(CP(CCDC_DATA2),		(IDIS | PTD | EN  | M7)) /*GPIO_101*/\
	MUX_VAL(CP(CCDC_DATA3),		(IDIS | PTD | EN  | M7)) /*GPIO_102*/\
	MUX_VAL(CP(CCDC_DATA4),		(IDIS | PTD | EN  | M7)) /*GPIO_103*/\
	MUX_VAL(CP(CCDC_DATA5),		(IDIS | PTD | EN  | M7)) /*GPIO_104*/\
	MUX_VAL(CP(CCDC_DATA6),		(IDIS | PTD | EN  | M7)) /*GPIO_105*/\
	MUX_VAL(CP(CCDC_DATA7),		(IDIS | PTD | EN  | M7)) /*GPIO_106*/\
	/* RMII */\
	MUX_VAL(CP(RMII_MDIO_DATA),	(IDIS | PTD | EN  | M7)) /*GPIO_107*/\
	MUX_VAL(CP(RMII_MDIO_CLK),	(IDIS | PTD | EN  | M7)) /*GPIO_108*/\
	MUX_VAL(CP(RMII_RXD0)	,	(IDIS | PTD | EN  | M7)) /*GPIO_109*/\
	MUX_VAL(CP(RMII_RXD1),		(IDIS | PTD | EN  | M7)) /*GPIO_110*/\
	MUX_VAL(CP(RMII_CRS_DV),	(IDIS | PTD | EN  | M7)) /*GPIO_111*/\
	MUX_VAL(CP(RMII_RXER),		(IDIS | PTD | EN  | M7)) /*GPIO_167*/\
	MUX_VAL(CP(RMII_TXD0),		(IDIS | PTD | EN  | M7)) /*GPIO_126*/\
	MUX_VAL(CP(RMII_TXD1),		(IDIS | PTD | EN  | M7)) /*GPIO_112*/\
	MUX_VAL(CP(RMII_TXEN),		(IDIS | PTD | EN  | M7)) /*GPIO_113*/\
	MUX_VAL(CP(RMII_50MHZ_CLK),	(IDIS | PTD | EN  | M7)) /*GPIO_114*/\
	/* McBSP2 */\
	MUX_VAL(CP(MCBSP2_FSX),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(MCBSP2_CLKX),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(MCBSP2_DR),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(MCBSP2_DX),		(IDIS | PTD | EN  | M7)) /*GPIO_119*/\
	/* MMC */\
	MUX_VAL(CP(MMC1_CLK),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MMC1_CMD),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MMC1_DAT0),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MMC1_DAT1),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MMC1_DAT2),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MMC1_DAT3),		(IEN  | PTU | DIS | M0)) \
	/* WriteProtect */\
	MUX_VAL(CP(MMC1_DAT4),		(IEN  | PTU | DIS | M4)) /*GPIO_126 : SDWPS*/\
	MUX_VAL(CP(MMC1_DAT5),		(IEN  | PTU | DIS | M4)) /*GPIO_127 : SDDETS*/\
	MUX_VAL(CP(MMC1_DAT6),		(IDIS | PTD | EN  | M7)) /*GPIO_128*/\
	MUX_VAL(CP(MMC1_DAT7),		(IDIS | PTD | EN  | M7)) /*GPIO_129*/\
	MUX_VAL(CP(MMC2_CLK),		(IDIS | PTD | EN  | M7)) /*GPIO_130*/\
	MUX_VAL(CP(MMC2_CMD),		(IDIS | PTD | EN  | M7)) /*GPIO_131*/\
	MUX_VAL(CP(MMC2_DAT0),		(IDIS | PTD | EN  | M7)) /*GPIO_132*/\
	MUX_VAL(CP(MMC2_DAT1),		(IDIS | PTD | EN  | M7)) /*GPIO_133*/\
	MUX_VAL(CP(MMC2_DAT2),		(IDIS | PTD | EN  | M7)) /*GPIO_134*/\
	MUX_VAL(CP(MMC2_DAT3),		(IDIS | PTD | EN  | M7)) /*GPIO_135*/\
	/* WriteProtect */\
	MUX_VAL(CP(MMC2_DAT4),		(IDIS | PTD | EN  | M7)) /*GPIO_136*/\
	MUX_VAL(CP(MMC2_DAT5),		(IDIS | PTD | EN  | M7)) /*GPIO_137*/\
	MUX_VAL(CP(MMC2_DAT6),		(IDIS | PTD | EN  | M7)) /*GPIO_138*/\
	MUX_VAL(CP(MMC2_DAT7),		(IDIS | PTD | EN  | M7)) /*GPIO_139*/\
	/* McBSP3 */\
	MUX_VAL(CP(MCBSP3_DX),		(IDIS | PTD | EN  | M7)) /*GPIO_140*/\
	MUX_VAL(CP(MCBSP3_DR),		(IDIS | PTD | EN  | M7)) /*GPIO_141*/\
	MUX_VAL(CP(MCBSP3_CLKX),	(IDIS | PTD | EN  | M7)) /*GPIO_142*/\
	MUX_VAL(CP(MCBSP3_FSX),		(IDIS | PTD | EN  | M7)) /*GPIO_143*/\
	/* UART2 */\
	MUX_VAL(CP(UART2_CTS),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(UART2_RTS),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(UART2_TX),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(UART2_RX),		(IEN  | PTD | DIS | M0)) \
	/* UART1 */\
	MUX_VAL(CP(UART1_TX),		(IDIS | PTD | EN  | M7)) /*GPIO_148*/\
	MUX_VAL(CP(UART1_RTS),		(IDIS | PTD | EN  | M7)) /*GPIO_149*/\
	MUX_VAL(CP(UART1_CTS),		(IDIS | PTD | EN  | M7)) /*GPIO_150*/\
	MUX_VAL(CP(UART1_RX),		(IDIS | PTD | EN  | M7)) /*GPIO_151*/\
	/* McBSP4 */\
	MUX_VAL(CP(MCBSP4_CLKX),	(IDIS | PTD | EN  | M7)) /*GPIO_152*/\
	MUX_VAL(CP(MCBSP4_DR),		(IEN  | PTD | EN  | M7)) /*GPIO_153*/\
	MUX_VAL(CP(MCBSP4_DX),		(IDIS | PTD | DIS | M4)) /*GPIO_154 : PDONES*/\
	MUX_VAL(CP(MCBSP4_FSX),		(IDIS | PTD | EN  | M7)) /*GPIO_155*/\
	/* McBSP */\
	MUX_VAL(CP(MCBSP1_CLKR),	(IDIS | PTD | DIS | M1)) /*GPIO_156 : MSTMP1*/\
	MUX_VAL(CP(MCBSP1_FSR),		(IEN  | PTU | DIS | M4)) /*GPIO_157 : BKC*/\
	MUX_VAL(CP(MCBSP1_DX),		(IDIS | PTD | DIS | M1)) /*GPIO_158 : WKUP1*/\
	MUX_VAL(CP(MCBSP1_DR),		(IDIS | PTD | DIS | M4)) /*GPIO_159 : APSHLT*/\
	MUX_VAL(CP(MCBSP_CLKS),		(IDIS | PTD | EN  | M7)) /*GPIO_160*/\
	MUX_VAL(CP(MCBSP1_FSX),		(IDIS | PTD | DIS | M1)) /*GPIO_161 : WKUP0*/\
	MUX_VAL(CP(MCBSP1_CLKX),	(IDIS | PTD | DIS | M4)) /*GPIO_162 : AHLTS*/\
	/* UART3 */\
/*	MUX_VAL(CP(UART3_CTS_RCTX),	(IEN  | PTD | EN  | M7)) \
	MUX_VAL(CP(UART3_RTS_SD),	(IDIS | PTD | DIS | M7)) \
	MUX_VAL(CP(UART3_RX_IRRX),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(UART3_TX_IRTX),	(IDIS | PTD | DIS | M0)) */\
	MUX_VAL(CP(USB0_DRVVBUS),	(IEN  | PTU | EN  | M4)) /*GPIO_125*/\
	/* HECC */\
	MUX_VAL(CP(HECC1_TXD),		(IDIS | PTD | EN  | M7)) /*GPIO_130*/\
	MUX_VAL(CP(HECC1_RXD),		(IDIS | PTD | EN  | M7)) /*GPIO_131*/\
	/* I2C */\
	MUX_VAL(CP(I2C1_SCL),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(I2C1_SDA),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(I2C2_SCL),		(IDIS | PTU | EN  | M7)) /*GPIO_168*/\
	MUX_VAL(CP(I2C2_SDA),		(IDIS | PTU | EN  | M7)) /*GPIO_183*/\
	MUX_VAL(CP(I2C3_SCL),		(IDIS | PTU | EN  | M7)) /*GPIO_184*/\
	MUX_VAL(CP(I2C3_SDA),		(IDIS | PTU | EN  | M7)) /*GPIO_185*/\
	/* HDQ */\
	MUX_VAL(CP(HDQ_SIO),		(IDIS | PTD | EN  | M7)) /*GPIO_170*/\
	/* McSPI */\
	MUX_VAL(CP(MCSPI1_CLK),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MCSPI1_SIMO),	(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MCSPI1_SOMI),	(IDIS | PTD | EN  | M7)) /*GPIO_173*/\
	MUX_VAL(CP(MCSPI1_CS0),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MCSPI1_CS1),		(IDIS | PTD | EN  | M7)) /*GPIO_175*/\
	MUX_VAL(CP(MCSPI1_CS2),		(IDIS | PTD | EN  | M7)) /*GPIO_176*/\
	MUX_VAL(CP(MCSPI1_CS3),		(IDIS | PTD | EN  | M7)) /*GPIO_177*/\
	MUX_VAL(CP(MCSPI2_CLK),		(IDIS | PTD | EN  | M7)) /*GPIO_178*/\
	MUX_VAL(CP(MCSPI2_SIMO),	(IDIS | PTD | EN  | M7)) /*GPIO_179*/\
	MUX_VAL(CP(MCSPI2_SOMI),	(IDIS | PTD | EN  | M7)) /*GPIO_180*/\
	MUX_VAL(CP(MCSPI2_CS0),		(IDIS | PTD | EN  | M7)) /*GPIO_181*/\
	MUX_VAL(CP(MCSPI2_CS1),		(IDIS | PTD | EN  | M7)) /*GPIO_182*/\
	/* Control and debug */\
	MUX_VAL(CP(SYS_NIRQ),		(IDIS | PTD | EN  | M7)) /*GPIO_0*/\
	MUX_VAL(CP(SYS_CLKOUT2),	(IDIS | PTD | EN  | M4)) /*GPIO_186*/\
	/* ETK (ES2 onwards) */\
	MUX_VAL(CP(ETK_CLK_ES2),	(IDIS | PTD | EN  | M4)) /*GPIO_12*/\
	MUX_VAL(CP(ETK_CTL_ES2),	(IDIS | PTD | EN  | M4)) /*GPIO_13*/\
	MUX_VAL(CP(ETK_D0_ES2),		(IDIS | PTD | EN  | M4)) /*GPIO_14*/\
	MUX_VAL(CP(ETK_D1_ES2),		(IDIS | PTD | EN  | M4)) /*GPIO_15*/\
	MUX_VAL(CP(ETK_D2_ES2),		(IDIS | PTD | EN  | M4)) /*GPIO_16*/\
	MUX_VAL(CP(ETK_D3_ES2),		(IDIS | PTD | EN  | M4)) /*GPIO_17*/\
	MUX_VAL(CP(ETK_D4_ES2),		(IDIS | PTD | EN  | M4)) /*GPIO_18*/\
	MUX_VAL(CP(ETK_D5_ES2),		(IDIS | PTD | EN  | M4)) /*GPIO_19*/\
	MUX_VAL(CP(ETK_D6_ES2),		(IDIS | PTD | EN  | M4)) /*GPIO_20*/\
	MUX_VAL(CP(ETK_D7_ES2),		(IDIS | PTD | EN  | M4)) /*GPIO_21*/\
	MUX_VAL(CP(ETK_D8_ES2),		(IDIS | PTD | EN  | M4)) /*GPIO_22*/\
	MUX_VAL(CP(ETK_D9_ES2),		(IDIS | PTD | EN  | M4)) /*GPIO_23*/\
	MUX_VAL(CP(ETK_D10_ES2),	(IDIS | PTD | EN  | M4)) /*GPIO_24*/\
	MUX_VAL(CP(ETK_D11_ES2),	(IDIS | PTD | EN  | M4)) /*GPIO_25*/\
	MUX_VAL(CP(ETK_D12_ES2),	(IDIS | PTD | EN  | M4)) /*GPIO_26*/\
	MUX_VAL(CP(ETK_D13_ES2),	(IDIS | PTD | EN  | M4)) /*GPIO_27*/\
	MUX_VAL(CP(ETK_D14_ES2),	(IDIS | PTD | EN  | M4)) /*GPIO_28*/\
	MUX_VAL(CP(ETK_D15_ES2),	(IDIS | PTD | EN  | M4)) /*GPIO_29 : SUBRES*/\
	MUX_VAL(CP(SYS_32K),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_CLKREQ),		(IDIS | PTD | EN  | M7)) /*GPIO_1*/\
	/*SYS_nRESWARM */\
	MUX_VAL(CP(SYS_NRESWARM),     	(IDIS | PTU | DIS | M7)) /*GPIO_30*/\
	MUX_VAL(CP(SYS_BOOT0),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_BOOT1),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_BOOT2),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_BOOT3),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_BOOT4),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_BOOT5),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_BOOT6),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_BOOT7),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_BOOT8),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_CLKOUT1),	(IEN  | PTD | EN  | M4)) /*GPIO_10 : LCDREQ*/\
	/* JTAG */\
	MUX_VAL(CP(JTAG_nTRST),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_TCK),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_RTCK),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_TMS),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_TDI),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_TDO),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_EMU0),		(IDIS | PTD | EN  | M7)) /*GPIO_11*/\
	MUX_VAL(CP(JTAG_EMU1),		(IDIS | PTD | EN  | M7)) /*GPIO_31*/\

#endif

#endif
