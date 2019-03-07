/*
 * wm8782.c  --  Driver for wm8782/ak4387 ADC/DAC chips
 *
 * Copyright (C) 2012 JVC KENWOOD Corporation
 * Author: Jun Ishibashi <ishibashi.jun@jvckenwood.com>
 *
 * Based on sound/soc/codecs/ads117x.c
 * 
 * Copyright 2009 ShotSpotter Inc.
 * Author: Graeme Gregory <gg@slimlogic.co.uk>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/soc.h>

#define AK4387_RATES (SNDRV_PCM_RATE_16000)
#define WM8782_RATES (SNDRV_PCM_RATE_16000)

#define AK4387_FORMATS (SNDRV_PCM_FMTBIT_S32_LE)
#define WM8782_FORMATS (SNDRV_PCM_FMTBIT_S32_LE)

static struct snd_soc_dai_driver wm8782_dai = {
/* ADC DAC */
	.name = "wm8782-hifi",
	.id = 1,
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = AK4387_RATES,
		.formats = AK4387_FORMATS,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = WM8782_RATES,
		.formats = WM8782_FORMATS,},
};

static struct snd_soc_codec_driver soc_codec_dev_wm8782;

static __devinit int wm8782_platform_probe(struct platform_device *pdev)
{
	return snd_soc_register_codec(&pdev->dev,
			&soc_codec_dev_wm8782, &wm8782_dai, 1);
}

static int __devexit wm8782_platform_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

static struct platform_driver wm8782_codec_driver = {
	.driver = {
			.name = "wm8782-codec",
			.owner = THIS_MODULE,
	},

	.probe = wm8782_platform_probe,
	.remove = __devexit_p(wm8782_platform_remove),
};

static int __init wm8782_init(void)
{
	return platform_driver_register(&wm8782_codec_driver);
}
module_init(wm8782_init);

static void __exit wm8782_exit(void)
{
	platform_driver_unregister(&wm8782_codec_driver);
}
module_exit(wm8782_exit);

MODULE_DESCRIPTION("ASoC wm8782 driver");
MODULE_AUTHOR("Jun Ishibashi");
MODULE_LICENSE("GPL");
