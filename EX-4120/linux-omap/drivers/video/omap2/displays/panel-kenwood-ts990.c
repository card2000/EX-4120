/*
 * LCD panel driver for Kenwood TS-990
 *  linux/arch/arm/mach-omap2/board-ts990main.c
 *
 * Copyright (C) 2012 JVC KENWOOD Corporation
 * Author: Jun Ishibashi <ishibashi.jun@jvckenwood.com>
 *
 * Based on drivers/video/omap2/displays/panel-sharp-lq043t1dg01.c
 *
 * Copyright (C) 2009 Texas Instruments Inc
 * Author: Vaibhav Hiremath <hvaibhav@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>

#include <plat/display.h>

static struct omap_video_timings ts990_timings = {
#ifdef CONFIG_PANEL_KENWOOD_TS990MAIN

	.x_res		= 800,
	.y_res		= 600,

	.pixel_clock	= 39272,

	.hsw		= 128,
	.hbp		= 88,
	.hfp		= 40,

	.vsw		= 4,
	.vbp		= 23,
	.vfp		= 1,

#else	/* CONFIG_PANEL_KENWOOD_TS990SUB */
	.x_res		= 320,
	.y_res		= 240,

	.pixel_clock	= 7800,

	.hsw		= 1,
	.hbp		= 30,
	.hfp		= 30,

	.vsw		= 1,
	.vbp		= 17,
	.vfp		= 4,
#endif
};

static int ts990_panel_power_on(struct omap_dss_device *dssdev)
{
	int r;

	if (dssdev->state == OMAP_DSS_DISPLAY_ACTIVE)
		return 0;

	r = omapdss_dpi_display_enable(dssdev);
	if (r)
		goto err0;

	if (dssdev->platform_enable) {
		r = dssdev->platform_enable(dssdev);
		if (r)
			goto err1;
	}

	return 0;
err1:
	omapdss_dpi_display_disable(dssdev);
err0:
	return r;
}

static void ts990_panel_power_off(struct omap_dss_device *dssdev)
{
	if (dssdev->state != OMAP_DSS_DISPLAY_ACTIVE)
		return;

	if (dssdev->platform_disable)
		dssdev->platform_disable(dssdev);

	omapdss_dpi_display_disable(dssdev);
}

static int ts990_panel_probe(struct omap_dss_device *dssdev)
{
#ifdef CONFIG_PANEL_KENWOOD_TS990MAIN
	dssdev->panel.config = OMAP_DSS_LCD_TFT | OMAP_DSS_LCD_IPC;
#else	/* CONFIG_PANEL_KENWOOD_TS990SUB */
	dssdev->panel.config = OMAP_DSS_LCD_TFT | OMAP_DSS_LCD_IVS |
		OMAP_DSS_LCD_IHS | OMAP_DSS_LCD_IPC | OMAP_DSS_LCD_IEO;
#endif
	dssdev->panel.acb = 0x0;
	dssdev->panel.timings = ts990_timings;

	return 0;
}

static void ts990_panel_remove(struct omap_dss_device *dssdev)
{
}

static int ts990_panel_enable(struct omap_dss_device *dssdev)
{
	int r = 0;

	r = ts990_panel_power_on(dssdev);
	if (r)
		return r;

	dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;

	return 0;
}

static void ts990_panel_disable(struct omap_dss_device *dssdev)
{
	ts990_panel_power_off(dssdev);

	dssdev->state = OMAP_DSS_DISPLAY_DISABLED;
}

static int ts990_panel_suspend(struct omap_dss_device *dssdev)
{
	ts990_panel_power_off(dssdev);
	dssdev->state = OMAP_DSS_DISPLAY_SUSPENDED;
	return 0;
}

static int ts990_panel_resume(struct omap_dss_device *dssdev)
{
	int r = 0;

	r = ts990_panel_power_on(dssdev);
	if (r)
		return r;

	dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;

	return 0;
}

static struct omap_dss_driver ts990_driver = {
	.probe		= ts990_panel_probe,
	.remove		= ts990_panel_remove,

	.enable		= ts990_panel_enable,
	.disable	= ts990_panel_disable,
	.suspend	= ts990_panel_suspend,
	.resume		= ts990_panel_resume,

	.driver         = {
		.name   = "ts990_panel",
		.owner  = THIS_MODULE,
	},
};

static int __init ts990_panel_drv_init(void)
{
	return omap_dss_register_driver(&ts990_driver);
}

static void __exit ts990_panel_drv_exit(void)
{
	omap_dss_unregister_driver(&ts990_driver);
}

module_init(ts990_panel_drv_init);
module_exit(ts990_panel_drv_exit);
MODULE_LICENSE("GPL");
