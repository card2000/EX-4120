/*
 * am3517evm.c - board file for TI's AM3517 family of devices.
 *
 * Author: Vaibhav Hiremath <hvaibhav@ti.com>
 *
 * Based on ti/evm/evm.c
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

#include <common.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/mux.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/emac_defs.h>
#include <asm/mach-types.h>
#include <i2c.h>
#include <netdev.h>
#include "am3517evm.h"


#ifdef CONFIG_APP_MAIN	/* AppCPU main */
#include "kenwood_genuine800x480.h"
#else	/* AppCPU sub */
#include "kenwood_genuine320x240.h"
#endif


#define AM3517_IP_SW_RESET     0x48002598
#define CPGMACSS_SW_RST                (1 << 1)


int lcdreq;


/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	int	counter;

	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP3517EVM;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

        /* 2013.04.24 Added by Matsuhashi */
        /* シリアルコンソールの有効／無効を判断する */
	omap_set_gpio_direction(125,1);
	if(omap_get_gpio_datain(125))
	{
		/* シリアルコンソールを無効にする */
		MUX_VAL(CP(UART3_TX_IRTX),  (IDIS | PTD | DIS | M7));   /*UART3_TX*/
		MUX_VAL(CP(UART3_RTS_SD),   (IDIS | PTD | DIS | M7));   /*UART3_RTS*/
		MUX_VAL(CP(UART3_CTS_RCTX), (IDIS | PTD | DIS | M7));   /*UART3_CTS*/
		MUX_VAL(CP(UART3_RX_IRRX),  (IDIS | PTD | DIS | M7));   /*UART3_RX*/
	}
	else
	{
		/* シリアルコンソールを有効にする */
		MUX_VAL(CP(UART3_TX_IRTX),  (IDIS | PTD | DIS | M0));   /*UART3_TX*/
		MUX_VAL(CP(UART3_RTS_SD),   (IDIS | PTD | DIS | M0));   /*UART3_RTS*/
		MUX_VAL(CP(UART3_CTS_RCTX), (IEN  | PTU | DIS | M0));   /*UART3_CTS*/
		MUX_VAL(CP(UART3_RX_IRRX),  (IEN  | PTD | DIS | M0));   /*UART3_RX*/
	}

	/* ポート設定 */
#ifdef CONFIG_APP_MAIN	/* AppCPU main */
	omap_set_gpio_direction(10,1);	/* GPIO_10 : LCDREQ */
	omap_set_gpio_direction(12,0);	/* GPIO_12 : HS1EN*/
	omap_set_gpio_dataout(12,1);
	omap_set_gpio_direction(17,0);	/* GPIO_17 : US1SPD*/
	omap_set_gpio_dataout(17,1);
	omap_set_gpio_direction(18,1);	/* GPIO_18 : POLSEL*/
	omap_set_gpio_direction(19,1);	/* GPIO_19 : HS1INT*/
	omap_set_gpio_direction(20,1);	/* GPIO_20 : HS2INT*/
	omap_set_gpio_direction(26,0);	/* GPIO_26 : US2SPD */
	omap_set_gpio_dataout(26,1);
	omap_set_gpio_direction(95,0);	/* GPIO_95 */
	omap_set_gpio_dataout(95,0);
	omap_set_gpio_direction(99,1);	/* GPIO_99 : MSSELM */
	omap_set_gpio_direction(101,0);	/* GPIO_101 : ABLENM */
	omap_set_gpio_dataout(101,0);
	omap_set_gpio_direction(102,0);	/* GPIO_102 : ABLENS */
	omap_set_gpio_dataout(102,0);
	omap_set_gpio_direction(103,0);	/* GPIO_103 : ETnRST */
	omap_set_gpio_dataout(103,0);
	omap_set_gpio_direction(105,0);	/* GPIO_105 : DVIPD */
	omap_set_gpio_dataout(105,0);
	omap_set_gpio_direction(106,0);	/* GPIO_106 : VDACPS */
	omap_set_gpio_dataout(106,0);
	omap_set_gpio_direction(126,1);	/* GPIO_126 : SDWPM */
	omap_set_gpio_direction(127,1);	/* GPIO_127 : SDDETM */
	omap_set_gpio_direction(154,0);	/* GPIO_154 : PDONEM */
	omap_set_gpio_dataout(154,0);
	omap_set_gpio_direction(155,0);	/* GPIO_155 : DACnPD */
	omap_set_gpio_dataout(155,0);
	omap_set_gpio_direction(157,1);	/* GPIO_157 : BKC */
	omap_set_gpio_direction(159,1);	/* GPIO_159 : APSHLT */
	omap_set_gpio_direction(162,0);	/* GPIO_162 : AHLTM */
	omap_set_gpio_dataout(162,0);
	omap_set_gpio_direction(175,1);	/* GPIO_175 : SAREQ */
	omap_set_gpio_direction(176,0);	/* GPIO_176 : HS2EN*/
	omap_set_gpio_dataout(176,1);
	omap_set_gpio_direction(186,0);	/* GPIO_186 */
	omap_set_gpio_dataout(186,0);
#else	/* AppCPU sub */
	omap_set_gpio_direction(10,1);	/* GPIO_10 : LCDREQ */
	for(counter=12;counter<29;counter++)
	{
		omap_set_gpio_direction(counter,0);	/* GPIO_12 - GPIO_28 */
		omap_set_gpio_dataout(counter,0);
	}
	omap_set_gpio_direction(29,0);	/* GPIO_29 : SUBRES */
	omap_set_gpio_dataout(29,1);
	omap_set_gpio_direction(99,1);	/* GPIO_99 : MSSELS */
	omap_set_gpio_direction(126,1);	/* GPIO_126 : SDWPS */
	omap_set_gpio_direction(127,1);	/* GPIO_127 : SDDETS */
	omap_set_gpio_direction(154,0);	/* GPIO_154 : PDONES */
	omap_set_gpio_dataout(154,0);
	omap_set_gpio_direction(159,0);	/* GPIO_159 : APSHLT */
	omap_set_gpio_dataout(159,0);
	omap_set_gpio_direction(162,0);	/* GPIO_162 : AHLTS */
	omap_set_gpio_dataout(162,0);
	omap_set_gpio_direction(186,0);	/* GPIO_186 */
	omap_set_gpio_dataout(186,0);
#endif

	return 0;
}


void dss_init(void)
{
	unsigned int i=0;
	unsigned int j=0;
	unsigned int *vram;
	int offset = 0;

	lcdreq = omap_get_gpio_datain(10);

	/* Reset Display Subsystem */
/*	*((uint *) 0x48050010) = 0x00000003;*/    /* DSS_SYSCONFIG */

	/* Add the image data */
	if(1 == lcdreq) {
		vram = (unsigned int *)(0x8FC00000 + offset);
		for (i = 0; i < (width*height);i++) {
			*(vram+i) = pixel_data[i];
		}
#ifdef CONFIG_APP_MAIN	/* AppCPU main */
		for (j = 0; j < (800*120); j++) {
			*(vram+i+j) = 0x00FFFFFF;
		}
#endif
	} else {
		printf("Disable LCD\n");
		vram = (unsigned int *)(0x8FC00000 + offset);
		for (i = 0; i < (width*height);i++) {
			*(vram+i) = 0;
		}
	}

/* Display Controliler */
/*	while(1 != *((uint *)0x48050014)) {
		udelay(1000);
	}
*/
	*((uint *) 0x48050410) = 0x00002015;    /* DISPC_SYSCONFIG */
	*((uint *) 0x48050444) = 0x00000204;    /* DISPC_CONFIG */

#ifdef CONFIG_APP_MAIN	/* AppCPU main */
	*((uint *) 0x48004E40) = 0x0000100B;    /* CM_CLKSEL_DSS */
	*((uint *) 0x48050464) = 0x0570277F;    /* DISPC_TIMING_H */
	*((uint *) 0x48050468) = 0x01700103;    /* DISPC_TIMING_V */
	*((uint *) 0x4805046c) = 0x00004000;    /* DISPC_POL_FREQ */
	*((uint *) 0x48050470) = 0x00010002;    /* DISPC_DIVISOR */
	*((uint *) 0x4805047c) = 0x0257031F;    /* DISPC_SIZE_LCD */
	*((uint *) 0x4805048c) = 0x0257031F;    /* DISPC_GFX_SIZE */
#else	/* AppCPU sub */
	*((uint *) 0x48004E40) = 0x00001005;    /* CM_CLKSEL_DSS */
	*((uint *) 0x48050464) = 0x01D01D00;    /* DISPC_TIMING_H */
	*((uint *) 0x48050468) = 0x01100400;    /* DISPC_TIMING_V */
	*((uint *) 0x4805046c) = 0x0000F000;    /* DISPC_POL_FREQ */
	*((uint *) 0x48050470) = 0x00010017;    /* DISPC_DIVISOR */
	*((uint *) 0x4805047c) = 0x00EF013F;    /* DISPC_SIZE_LCD */
	*((uint *) 0x4805048c) = 0x00EF013F;    /* DISPC_GFX_SIZE */
#endif
	*((uint *) 0x48050480) = 0x8FC00000;    /* DISPC_GFX_BA0 */
	*((uint *) 0x48050484) = 0x8FC00000;    /* DISPC_GFX_BA1 */
	*((uint *) 0x480504a0) = 0x00000099;    /* DISPC_GFX_ATTRIBUTES */

	udelay(1000);
	*((uint *) 0x48050440) = 0x00018329;	/* DISPC_CONTROL */
}


/*
 * Routine: misc_init_r
 * Description: Init i2c, ethernet, etc... (done here so udelay works)
 */
int misc_init_r(void)
{
       u32 reset;


#ifdef CONFIG_DRIVER_OMAP34XX_I2C
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif

	dieid_num_r();


#if defined(CONFIG_DRIVER_TI_EMAC)

       omap_set_gpio_direction(103,0);
       omap_set_gpio_dataout(103,0);
       udelay(200);
       omap_set_gpio_dataout(103,1);

       /* allow the PHY to stabilize and settle down */
       udelay(1);

       /*ensure that the module is out of reset*/
       reset = readl(AM3517_IP_SW_RESET);
       reset &= (~CPGMACSS_SW_RST);
       writel(reset,AM3517_IP_SW_RESET);

#endif

	dss_init();

#ifdef CONFIG_APP_MAIN	/* AppCPU main */
	omap_set_gpio_direction(22,0);	/* GPIO_22 : US1SUS*/
	omap_set_gpio_dataout(22,0);
	omap_set_gpio_direction(24,0);	/* GPIO_24 : US2SUS*/
	omap_set_gpio_dataout(24,0);

	udelay(200000);
	if(1 == lcdreq) {
		omap_set_gpio_dataout(101,1);	/* Main LCD Backlight Power */
		omap_set_gpio_dataout(102,1);	/* Sub LCD Backlight Power */
	}
#endif

	return 0;
}


/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int cpu_eth_init(bd_t *bis)
{
#if defined(CONFIG_DRIVER_TI_EMAC)
	char mac_buf_msb[9], mac_buf_lsb[9];
	char mac_id[20];
	u_int8_t macaddr[6];
	int      i;
	unsigned long  msb, lsb;

	msb = readl(EMAC_MACADDR_MSB);
	lsb = readl(EMAC_MACADDR_LSB);
	sprintf(mac_buf_msb, "%8.8x", (unsigned int)msb);
	sprintf(mac_buf_lsb, "%8.8x", (unsigned int)lsb);
	memset(mac_id, '\0', sizeof(mac_id));
	sprintf(mac_id, "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c",
		mac_buf_msb[2], mac_buf_msb[3],
		mac_buf_msb[4], mac_buf_msb[5],
		mac_buf_msb[6], mac_buf_msb[7],
		mac_buf_lsb[2], mac_buf_lsb[3],
		mac_buf_lsb[4], mac_buf_lsb[5],
		mac_buf_lsb[6], mac_buf_lsb[7]);

	setenv("ethaddr", mac_id);

	for (i=0; i<3; i++) {
		macaddr[5-i] = lsb & 0xFF;
		lsb >>= 8;
	}
	for (i=0; i<3; i++) {
		macaddr[2-i] = msb & 0xFF;
		msb >>= 8;
	}
	davinci_eth_set_mac_addr((const u_int8_t *)macaddr);

	printf("davinci_emac_initialize\n");
	davinci_emac_initialize();
#endif
	return 0;
 }




/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_AM3517EVM();
}
