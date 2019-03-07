/*
 * sound/soc/omap/ts990.c  -- ALSA SoC support for TS-990
 *
 * Copyright (C) 2012 JVC KENWOOD Corporation
 * Author: Jun Ishibashi <ishibashi.jun@jvckenwood.com>
 *
 * Based on sound/soc/omap/am3517evm.c by Anuj Aggarwal
 *
 * Copyright (C) 2009 Texas Instruments Incorporated
 * Author: Anuj Aggarwal <anuj.aggarwal@ti.com>
 *
 * Based on sound/soc/omap/beagle.c by Steve Sakoman
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any kind,
 * whether express or implied; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 */

#include <linux/clk.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <plat/mcbsp.h>

#include "omap-mcbsp.h"
#include "omap-pcm.h"

#include "../codecs/wm8782.h"

#define MASTER_CLOCK 	12288000
#define BIT_CLOCK 	1024000

static int ts990_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret;
#ifdef CONFIG_SND_OMAP_SOC_TS990MAIN
	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_LEFT_J |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}

	ret = snd_soc_dai_set_sysclk(cpu_dai, OMAP_MCBSP_SYSCLK_CLKS_EXT, MASTER_CLOCK,
				SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set CPU system clock OMAP_MCBSP_SYSCLK_CLKS_EXT\n");
		return ret;
	}

	ret = snd_soc_dai_set_clkdiv(cpu_dai, OMAP_MCBSP_CLKGDV, 12);
	if (ret < 0) {
		printk(KERN_ERR "can't set SRG clock divider\n");
		return ret;
	}
#else
	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_LEFT_J |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}

	ret = snd_soc_dai_set_sysclk(cpu_dai, OMAP_MCBSP_SYSCLK_CLKS_EXT, BIT_CLOCK,
				SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set CPU system clock OMAP_MCBSP_CLKR_SRC_CLKX\n");
		return ret;
	}
#endif
	return 0;
}

static struct snd_soc_ops ts990_ops = {
	.hw_params = ts990_hw_params,
};

static int ts990_wm8782_init(struct snd_soc_pcm_runtime *rtd)
{
#ifdef CONFIG_SND_OMAP_SOC_TS990MAIN
	struct omap_mcbsp_reg_cfg config;

	memset(&config, 0, sizeof(struct omap_mcbsp_reg_cfg));
	config.spcr2 = FREE | FRST | GRST;
	config.srgr2 = FSGM | FPER(63);
	config.srgr1 = CLKGDV(11) | FWID(31);
	config.pcr0 = FSXM | FSRM | CLKXM | CLKRM | CLKXP | CLKRP;

	omap_mcbsp_start_srg(1, &config);

	gpio_direction_output(155, 1);
#endif
	return 0;
}

/* Digital audio interface glue - connects codec <--> CPU */
static struct snd_soc_dai_link ts990_dai = {
	.name = "WM8782",
	.stream_name = "WM8782",
	.cpu_dai_name ="omap-mcbsp-dai.1",
	.codec_dai_name = "wm8782-hifi",
	.platform_name = "omap-pcm-audio",
	.codec_name = "wm8782-codec",
	.init = ts990_wm8782_init,
	.ops = &ts990_ops,
};

/* Audio machine driver */
static struct snd_soc_card snd_soc_ts990 = {
	.name = "TS-990",
	.dai_link = &ts990_dai,
	.num_links = 1,
};

static struct platform_device wm8782_device = {
	.name = "wm8782-codec",
	.id = -1,
};

static struct platform_device *ts990_snd_device;

static int __init ts990_soc_init(void)
{
	int ret;

	pr_info("TS-990 SoC init\n");

	platform_device_register(&wm8782_device);

	ts990_snd_device = platform_device_alloc("soc-audio", -1);
	if (!ts990_snd_device) {
		printk(KERN_ERR "Platform device allocation failed\n");
		return -ENOMEM;
	}

	platform_set_drvdata(ts990_snd_device, &snd_soc_ts990);

	ret = platform_device_add(ts990_snd_device);
	if (ret)
		goto err1;

	return 0;

err1:
	printk(KERN_ERR "Unable to add platform device\n");
	platform_device_put(ts990_snd_device);

	return ret;
}

static void __exit ts990_soc_exit(void)
{
	platform_device_unregister(ts990_snd_device);
}

module_init(ts990_soc_init);
module_exit(ts990_soc_exit);

MODULE_AUTHOR("Jun Ishibashi <ishibashi.jun@jvckenwood.com>");
MODULE_DESCRIPTION("ALSA SoC TS-990");
MODULE_LICENSE("GPL v2");
