/*
 *  linux/arch/arm/mach-omap2/board-ts990main.c
 *
 * Copyright (C) 2012 JVC KENWOOD Corporation
 * Author: Jun Ishibashi <ishibashi.jun@jvckenwood.com>
 *
 * Based on mach-omap2/board-am3517evm.c by Ranjith Lohithakshan
 *
 * Copyright (C) 2009 Texas Instruments Incorporated
 * Author: Ranjith Lohithakshan <ranjithl@ti.com>
 *
 * Based on mach-omap2/board-omap3evm.c
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as  published by the
 * Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any kind,
 * whether express or implied; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/davinci_emac.h>
#include <linux/irq.h>
#include <linux/input.h>
#include <linux/mmc/host.h>
#include <linux/mtd/physmap.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand.h>
#include <linux/spi/spi.h>

#include <mach/hardware.h>
#include <mach/am35xx.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <plat/board.h>
#include <plat/common.h>
#include <plat/mcspi.h>
#include <plat/usb.h>
#include <plat/display.h>
#include <plat/gpmc.h>
#include <plat/nand.h>

#include "mux.h"
#include "control.h"
#include "hsmmc.h"
#include "board-flash.h"

#define AM35XX_EVM_MDIO_FREQUENCY	(1000000)

#define NAND_BLOCK_SIZE        SZ_128K

static struct mtd_partition ts990_nand_partitions[] = {
/* All the partition sizes are listed in terms of NAND block size */
	{
		.name           = "xloader-nand",
		.offset         = 0,
		.size           = 4*(SZ_128K)
	},
	{
		.name           = "uboot-nand",
		.offset         = MTDPART_OFS_APPEND,
		.size           = 16*(SZ_128K)
	},
	{
		.name           = "params-nand",
		.offset         = MTDPART_OFS_APPEND,
		.size           = 2*(SZ_128K)
	},
	{
		.name           = "linux-nand",
		.offset         = MTDPART_OFS_APPEND,
		.size           = 48*(SZ_128K)
	},
	{
		.name           = "rootfs-nand",
		.size           = MTDPART_SIZ_FULL,
		.offset         = MTDPART_OFS_APPEND,
	},
};

#ifdef CONFIG_MACH_TS990MAIN		/* for AppCPU Main */
static struct mdio_platform_data ts990_mdio_pdata = {
	.bus_freq	= AM35XX_EVM_MDIO_FREQUENCY,
};

static struct resource ts990_mdio_resources[] = {
	{
		.start  = AM35XX_IPSS_EMAC_BASE + AM35XX_EMAC_MDIO_OFFSET,
		.end    = AM35XX_IPSS_EMAC_BASE + AM35XX_EMAC_MDIO_OFFSET +
			  SZ_4K - 1,
		.flags  = IORESOURCE_MEM,
	},
};

static struct platform_device ts990_mdio_device = {
	.name		= "davinci_mdio",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(ts990_mdio_resources),
	.resource	= ts990_mdio_resources,
	.dev.platform_data = &ts990_mdio_pdata,
};

static struct emac_platform_data ts990_emac_pdata = {
	.rmii_en	= 1,
};

static struct resource ts990_emac_resources[] = {
	{
		.start  = AM35XX_IPSS_EMAC_BASE,
		.end    = AM35XX_IPSS_EMAC_BASE + 0x2FFFF,
		.flags  = IORESOURCE_MEM,
	},
	{
		.start  = INT_35XX_EMAC_C0_RXTHRESH_IRQ,
		.end    = INT_35XX_EMAC_C0_RXTHRESH_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.start  = INT_35XX_EMAC_C0_RX_PULSE_IRQ,
		.end    = INT_35XX_EMAC_C0_RX_PULSE_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.start  = INT_35XX_EMAC_C0_TX_PULSE_IRQ,
		.end    = INT_35XX_EMAC_C0_TX_PULSE_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.start  = INT_35XX_EMAC_C0_MISC_PULSE_IRQ,
		.end    = INT_35XX_EMAC_C0_MISC_PULSE_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device ts990_emac_device = {
	.name		= "davinci_emac",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(ts990_emac_resources),
	.resource	= ts990_emac_resources,
};

static void ts990_enable_ethernet_int(void)
{
	u32 regval;

	regval = omap_ctrl_readl(AM35XX_CONTROL_LVL_INTR_CLEAR);
	regval = (regval | AM35XX_CPGMAC_C0_RX_PULSE_CLR |
		AM35XX_CPGMAC_C0_TX_PULSE_CLR |
		AM35XX_CPGMAC_C0_MISC_PULSE_CLR |
		AM35XX_CPGMAC_C0_RX_THRESH_CLR);
	omap_ctrl_writel(regval, AM35XX_CONTROL_LVL_INTR_CLEAR);
	regval = omap_ctrl_readl(AM35XX_CONTROL_LVL_INTR_CLEAR);
}

static void ts990_disable_ethernet_int(void)
{
	u32 regval;

	regval = omap_ctrl_readl(AM35XX_CONTROL_LVL_INTR_CLEAR);
	regval = (regval | AM35XX_CPGMAC_C0_RX_PULSE_CLR |
		AM35XX_CPGMAC_C0_TX_PULSE_CLR);
	omap_ctrl_writel(regval, AM35XX_CONTROL_LVL_INTR_CLEAR);
	regval = omap_ctrl_readl(AM35XX_CONTROL_LVL_INTR_CLEAR);
}

static void ts990_ethernet_init(struct emac_platform_data *pdata)
{
	u32 regval, mac_lo, mac_hi;

	mac_lo = omap_ctrl_readl(AM35XX_CONTROL_FUSE_EMAC_LSB);
	mac_hi = omap_ctrl_readl(AM35XX_CONTROL_FUSE_EMAC_MSB);

	pdata->mac_addr[0] = (u_int8_t)((mac_hi & 0xFF0000) >> 16);
	pdata->mac_addr[1] = (u_int8_t)((mac_hi & 0xFF00) >> 8);
	pdata->mac_addr[2] = (u_int8_t)((mac_hi & 0xFF) >> 0);
	pdata->mac_addr[3] = (u_int8_t)((mac_lo & 0xFF0000) >> 16);
	pdata->mac_addr[4] = (u_int8_t)((mac_lo & 0xFF00) >> 8);
	pdata->mac_addr[5] = (u_int8_t)((mac_lo & 0xFF) >> 0);

	pdata->ctrl_reg_offset		= AM35XX_EMAC_CNTRL_OFFSET;
	pdata->ctrl_mod_reg_offset	= AM35XX_EMAC_CNTRL_MOD_OFFSET;
	pdata->ctrl_ram_offset		= AM35XX_EMAC_CNTRL_RAM_OFFSET;
	pdata->ctrl_ram_size		= AM35XX_EMAC_CNTRL_RAM_SIZE;
	pdata->version			= EMAC_VERSION_2;
	pdata->hw_ram_addr		= AM35XX_EMAC_HW_RAM_ADDR;
	pdata->interrupt_enable		= ts990_enable_ethernet_int;
	pdata->interrupt_disable	= ts990_disable_ethernet_int;
	ts990_emac_device.dev.platform_data	= pdata;
	platform_device_register(&ts990_mdio_device);
	platform_device_register(&ts990_emac_device);
	clk_add_alias(NULL, dev_name(&ts990_mdio_device.dev),
		      NULL, &ts990_emac_device.dev);

	regval = omap_ctrl_readl(AM35XX_CONTROL_IP_SW_RESET);
	regval = regval & (~(AM35XX_CPGMACSS_SW_RST));
	omap_ctrl_writel(regval, AM35XX_CONTROL_IP_SW_RESET);
	regval = omap_ctrl_readl(AM35XX_CONTROL_IP_SW_RESET);

	return ;
}

static int __init ts990_i2c_init(void)
{
	omap_register_i2c_bus(1, 100, NULL, 0);

	return 0;
}
#endif

#define MAIN_LCD_PANEL_PWR		101
#define SUB_LCD_PANEL_PWR		102

static int ts990_panel_enable_lcd(struct omap_dss_device *dssdev)
{
	return 0;
}

static void ts990_panel_disable_lcd(struct omap_dss_device *dssdev)
{
	gpio_set_value(MAIN_LCD_PANEL_PWR, 0);
	gpio_set_value(SUB_LCD_PANEL_PWR, 0);
}

static struct omap_dss_device ts990_lcd_device = {
	.type			= OMAP_DISPLAY_TYPE_DPI,
	.name			= "lcd",
	.driver_name		= "ts990_panel",
	.phy.dpi.data_lines 	= 24,
	.platform_enable	= ts990_panel_enable_lcd,
	.platform_disable	= ts990_panel_disable_lcd,
};

static struct omap_dss_device *ts990_dss_devices[] = {
	&ts990_lcd_device,
};

static struct omap_dss_board_info ts990_dss_data = {
	.num_devices	= ARRAY_SIZE(ts990_dss_devices),
	.devices	= ts990_dss_devices,
	.default_device	= &ts990_lcd_device,
};

static struct platform_device ts990_dss_device = {
	.name		= "omapdss",
	.id		= -1,
	.dev		= {
		.platform_data	= &ts990_dss_data,
	},
};

/**************************************************************/
/* SPI Initialization                                         */
/* for Scope DSP Communication and inter AM3517 Communication */
/* Added by Matsuhashi                                        */
/**************************************************************/

/* SPI #1 controller driver */
static struct omap2_mcspi_device_config ts990_mcspi1_config = {
        .turbo_mode     = 0,
        .single_channel = 0,    /* 0: slave, 1: master */
};

struct spi_board_info ts990_spi_board_info[] = {
        [0] = {
                .modalias               = "spidev",
                .bus_num                = 1,
                .chip_select            = 0,
       		.mode                   = SPI_MODE_3,
                .max_speed_hz           = 1500000,
                .controller_data        = &ts990_mcspi1_config,
        },
};

/*
 * Board initialization
 */
static struct omap_board_config_kernel ts990_config[] __initdata = {
};

static struct platform_device *ts990_devices[] __initdata = {
	&ts990_dss_device,
};

static void __init ts990_init_irq(void)
{
	omap_board_config = ts990_config;
	omap_board_config_size = ARRAY_SIZE(ts990_config);
	omap2_init_common_infrastructure();
	omap2_init_common_devices(NULL, NULL);
	omap_init_irq();
	gpmc_init();
}

#ifdef CONFIG_MACH_TS990MAIN		/* for AppCPU Main */
static struct omap_musb_board_data musb_board_data = {
	.interface_type         = MUSB_INTERFACE_ULPI,
	.mode                   = MUSB_OTG,
	.power                  = 500,
};

static __init void ts990_musb_init(void)
{
	u32 devconf2;

	/*
	 * Set up USB clock/mode in the DEVCONF2 register.
	 */
	devconf2 = omap_ctrl_readl(AM35XX_CONTROL_DEVCONF2);

	/* USB2.0 PHY reference clock is 13 MHz */
	devconf2 &= ~(CONF2_REFFREQ | CONF2_OTGMODE | CONF2_PHY_GPIOMODE);
	devconf2 |=  CONF2_REFFREQ_13MHZ | CONF2_SESENDEN | CONF2_VBDTCTEN
			| CONF2_DATPOL;

	omap_ctrl_writel(devconf2, AM35XX_CONTROL_DEVCONF2);

	usb_musb_init(&musb_board_data);
}

static const struct ohci_hcd_omap_platform_data ohci_pdata __initconst = {
	.port_mode[0] = OMAP_OHCI_PORT_MODE_PHY_6PIN_DPDM,
	.port_mode[1] = OMAP_OHCI_PORT_MODE_PHY_6PIN_DPDM,
	.port_mode[2] = OMAP_OHCI_PORT_MODE_UNUSED,

	.es2_compatibility = false
};
#endif

#ifdef CONFIG_OMAP_MUX
static struct omap_board_mux board_mux[] __initdata = {
	{ .reg_offset = OMAP_MUX_TERMINATOR },
};
#endif


static struct omap2_hsmmc_info mmc[] = {
	{
		.mmc		= 1,
		.caps		= MMC_CAP_4_BIT_DATA,
		.gpio_cd	= 127,
		.gpio_wp	= 126,
	},
	{}      /* Terminator */
};


static void __init ts990_init(void)
{
	omap3_mux_init(board_mux, OMAP_PACKAGE_CBB);

	platform_add_devices(ts990_devices,
				ARRAY_SIZE(ts990_devices));

	omap_serial_init();

	/* NAND */
	board_nand_init(ts990_nand_partitions,
					ARRAY_SIZE(ts990_nand_partitions), 0, 0);

	/* MMC init function */
	omap2_hsmmc_init(mmc);

	/* for SPI driver */
	spi_register_board_info(ts990_spi_board_info,
				ARRAY_SIZE(ts990_spi_board_info));

#ifdef CONFIG_MACH_TS990MAIN		/* for AppCPU Main */
	ts990_i2c_init();

	usb_ohci_init(&ohci_pdata);

	/*Ethernet*/
	ts990_ethernet_init(&ts990_emac_pdata);

	/* MUSB */
	ts990_musb_init();

	/* GPIO 155 : DAC RSTN */
	gpio_request(155, "dac_reset");
#endif

	/* PDONE */
	gpio_request(154, "pdone");
	gpio_export(154, 0);
	gpio_direction_output(154, 0);
	gpio_set_value(154, 0);
}
MACHINE_START(OMAP3517EVM, "TS-990")
	.boot_params	= 0x80000100,
	.map_io		= omap3_map_io,
	.reserve	= omap_reserve,
	.init_irq	= ts990_init_irq,
	.init_machine	= ts990_init,
	.timer		= &omap_timer,
MACHINE_END
