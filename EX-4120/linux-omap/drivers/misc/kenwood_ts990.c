/*************************************************************************/
/* Linux Device Driver framework                                         */
/*-----------------------------------------------------------------------*/
/* Copyright (C) 2011-2013 JVC KENWOOD CORPORATION. All Rights Reserved. */
/*-----------------------------------------------------------------------*/
/* Description:	EX-4120 general purpose driver program                   */
/* Date:	2011.09.26                                               */
/* Author:	K.Matsuhashi                                             */
/*************************************************************************/
/* Change History:								 */
/* DATE		NAME		NOTE					 */
/* 2013.02.20	Matsuhashi	USB Audio problem(IOCOO Y52-84-46)	 */
/*************************************************************************/

#include <linux/module.h>
#include <linux/device.h>

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/interrupt.h>

#include <linux/poll.h>
#include <linux/gpio.h>
#include <linux/errno.h>

#include "kenwood_common.h"
#include "kenwood_ts990.h"


static	void __iomem	*gpio1_base;
static	void __iomem	*gpio2_base;
static	void __iomem	*gpio3_base;
static	void __iomem	*gpio4_base;
static	void __iomem	*gpio5_base;
static	void __iomem	*gpio6_base;
static	void __iomem	*cmdss_base;
static	void __iomem	*dispc_base;
static	void __iomem	*emacss_base;
static	void __iomem	*emac_base;
static	void __iomem	*mdio_base;
static	void __iomem	*padconf_base;

struct kenwood_intctl_str	lcdreq_ctl;
struct kenwood_intctl_str	usb1oc_ctl;
struct kenwood_intctl_str	usb2oc_ctl;
struct kenwood_intctl_str	apshlt_ctl;

static irqreturn_t lcdreq_gpio_irq_handler(int irq, void *param)
{
	int		index;
	unsigned int	gpio_bit;

	printk( KERN_INFO "KENWOOD: LCDREQ IRQ handler called\n" );

        index = GPIO_LCDREQ_INT_NUMBER & 0x1f;
	gpio_bit = 1 << index;

        /* レベル割り込みのときは検出設定を毎回変える */
        if(gpio_bit & __raw_readl(gpio1_base + AM3517_GPIO_DATAIN))
        {
		printk( KERN_INFO "KENWOOD: LCDREQ Interrupt occurred 'H'\n" );
		/* 割り込み要因をクリア */
		GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_IRQSTATUS1, 1);

                if(1 == lcdreq_ctl.level)
		{
			printk( KERN_INFO "KENWOOD: But, ignored\n" );
			return IRQ_NONE;
		}
                lcdreq_ctl.level = 1;

		GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_RISINGDETECT, 0);
		GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_FALLINGDETECT, 1);
	}
        else
        {
		printk( KERN_INFO "KENWOOD: LCDREQ Interrupt occurred 'L'\n" );
		/* 割り込み要因をクリア */
		GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_IRQSTATUS1, 1);

                if(0 == lcdreq_ctl.level)
		{
			printk( KERN_INFO "KENWOOD: But, ignored\n" );
			return IRQ_NONE;
		}
                lcdreq_ctl.level = 0;

		GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_RISINGDETECT, 1);
		GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_FALLINGDETECT, 0);
	}

	/* 条件待ちスリープから起床 */
	lcdreq_ctl.condition_flag = 1;
	wake_up_interruptible(&lcdreq_ctl.queue);
//	printk( KERN_INFO "KENWOOD: LCDREQ wake up from sleep!\n" );

	return IRQ_HANDLED;
}



static irqreturn_t usb1oc_gpio_irq_handler(int irq, void *param)
{
	int		index;
	unsigned int	gpio_bit;

	printk( KERN_INFO "KENWOOD: USB1OC IRQ handler called\n" );

        index = GPIO_USB1OC_INT_NUMBER & 0x1f;
	gpio_bit = 1 << index;

        /* レベル割り込みのときは検出設定を毎回変える */
        if(gpio_bit & __raw_readl(gpio1_base + AM3517_GPIO_DATAIN))
        {
		printk( KERN_INFO "KENWOOD: USB1OC Interrupt occurred 'H'\n" );
		/* 割り込み要因をクリア */
		GPIO_REG_CTL(GPIO_USB1OC_INT_NUMBER, AM3517_GPIO_IRQSTATUS1, 1);

                if(1 == usb1oc_ctl.level)
		{
			printk( KERN_INFO "KENWOOD: But, ignored\n" );
			return IRQ_NONE;
		}
                usb1oc_ctl.level = 1;

//		GPIO_REG_CTL(GPIO_USB1OC_INT_NUMBER, AM3517_GPIO_RISINGDETECT, 0);
//		GPIO_REG_CTL(GPIO_USB1OC_INT_NUMBER, AM3517_GPIO_FALLINGDETECT, 1);
        }
        else
        {
		printk( KERN_INFO "KENWOOD: USB1OC Interrupt occurred 'L'\n" );
		/* 割り込み要因をクリア */
		GPIO_REG_CTL(GPIO_USB1OC_INT_NUMBER, AM3517_GPIO_IRQSTATUS1, 1);

                if(0 == usb1oc_ctl.level)
		{
			printk( KERN_INFO "KENWOOD: But, ignored\n" );
			return IRQ_NONE;
		}
                usb1oc_ctl.level = 0;

//		GPIO_REG_CTL(GPIO_USB1OC_INT_NUMBER, AM3517_GPIO_RISINGDETECT, 1);
//		GPIO_REG_CTL(GPIO_USB1OC_INT_NUMBER, AM3517_GPIO_FALLINGDETECT, 0);
        }

	/* 条件待ちスリープから起床 */
	usb1oc_ctl.condition_flag = 1;
	wake_up_interruptible(&usb1oc_ctl.queue);
//	printk( KERN_INFO "KENWOOD: USB1OC wake up from sleep!\n" );

	return IRQ_HANDLED;
}



static irqreturn_t usb2oc_gpio_irq_handler(int irq, void *param)
{
	int		index;
	unsigned int	gpio_bit;

	printk( KERN_INFO "KENWOOD: USB2OC IRQ handler called\n" );

        index = GPIO_USB2OC_INT_NUMBER & 0x1f;
	gpio_bit = 1 << index;

        /* レベル割り込みのときは検出設定を毎回変える */
        if(gpio_bit & __raw_readl(gpio1_base + AM3517_GPIO_DATAIN))
        {
		printk( KERN_INFO "KENWOOD: USB2OC Interrupt occurred 'H'\n" );
		/* 割り込み要因をクリア */
		GPIO_REG_CTL(GPIO_USB2OC_INT_NUMBER, AM3517_GPIO_IRQSTATUS1, 1);

                if(1 == usb2oc_ctl.level)
		{
			printk( KERN_INFO "KENWOOD: But, ignored\n" );
			return IRQ_NONE;
		}
                usb2oc_ctl.level = 1;

//		GPIO_REG_CTL(GPIO_USB2OC_INT_NUMBER, AM3517_GPIO_RISINGDETECT, 0);
//		GPIO_REG_CTL(GPIO_USB2OC_INT_NUMBER, AM3517_GPIO_FALLINGDETECT, 1);
        }
        else
        {
		printk( KERN_INFO "KENWOOD: USB2OC Interrupt occurred 'L'\n" );
		/* 割り込み要因をクリア */
		GPIO_REG_CTL(GPIO_USB2OC_INT_NUMBER, AM3517_GPIO_IRQSTATUS1, 1);

                if(0 == usb2oc_ctl.level)
		{
			printk( KERN_INFO "KENWOOD: But, ignored\n" );
			return IRQ_NONE;
		}
                usb2oc_ctl.level = 0;

//		GPIO_REG_CTL(GPIO_USB2OC_INT_NUMBER, AM3517_GPIO_RISINGDETECT, 1);
//		GPIO_REG_CTL(GPIO_USB2OC_INT_NUMBER, AM3517_GPIO_FALLINGDETECT, 0);
        }

	/* 条件待ちスリープから起床 */
	usb2oc_ctl.condition_flag = 1;
	wake_up_interruptible(&usb2oc_ctl.queue);
//	printk( KERN_INFO "KENWOOD: USB2OC wake up from sleep!\n" );

	return IRQ_HANDLED;
}



static irqreturn_t apshlt_gpio_irq_handler(int irq, void *param)
{
	int		index;
	unsigned int	gpio_bit;

	printk( KERN_INFO "KENWOOD: APSHLT IRQ handler called\n" );

        index = GPIO_APSHLT_INT_NUMBER & 0x1f;
	gpio_bit = 1 << index;

        if(gpio_bit & __raw_readl(gpio5_base + AM3517_GPIO_DATAIN))
        {
		printk( KERN_INFO "KENWOOD: APSHLT Interrupt occurred 'H'\n" );
		/* 割り込み要因をクリア */
		GPIO_REG_CTL(GPIO_APSHLT_INT_NUMBER, AM3517_GPIO_IRQSTATUS1, 1);

                if(1 == apshlt_ctl.level)
		{
			printk( KERN_INFO "KENWOOD: But, ignored\n" );
			return IRQ_NONE;
		}
                apshlt_ctl.level = 1;

//		GPIO_REG_CTL(GPIO_APSHLT_INT_NUMBER, AM3517_GPIO_RISINGDETECT, 0);
//		GPIO_REG_CTL(GPIO_APSHLT_INT_NUMBER, AM3517_GPIO_FALLINGDETECT, 1);
        }
        else
        {
		printk( KERN_INFO "KENWOOD: APSHLT Interrupt occurred 'L'\n" );
		/* 割り込み要因をクリア */
		GPIO_REG_CTL(GPIO_APSHLT_INT_NUMBER, AM3517_GPIO_IRQSTATUS1, 1);

                if(0 == apshlt_ctl.level)
		{
			printk( KERN_INFO "KENWOOD: But, ignored\n" );
			return IRQ_NONE;
		}
                apshlt_ctl.level = 0;

//		GPIO_REG_CTL(GPIO_APSHLT_INT_NUMBER, AM3517_GPIO_RISINGDETECT, 1);
//		GPIO_REG_CTL(GPIO_APSHLT_INT_NUMBER, AM3517_GPIO_FALLINGDETECT, 0);
        }

	/* 条件待ちスリープから起床 */
	apshlt_ctl.condition_flag = 1;
	wake_up_interruptible(&apshlt_ctl.queue);
//	printk( KERN_INFO "KENWOOD: APSHLT wake up from sleep!\n" );

	return IRQ_HANDLED;
}



static long kenwood_ioctl(struct file *filep, unsigned int command, unsigned long param)
{
	int		ret;
	int		index;
	unsigned int	gpio_bit;
	unsigned long	*addr = (unsigned long *)param;

	switch(command)
	{
		/* メインCPUのポート変化待ちスリープ */
		case KENWOOD_IOC_MCPU_PWRCTL:
		        index = GPIO_LCDREQ_INT_NUMBER & 0x1f;
		        gpio_bit = 1 << index;

			/* IRQ disable */
			GPIO_IRQ_CTL(GPIO_LCDREQ_INT_NUMBER, 0);

			if(!lcdreq_ctl.suspend_flag)
			{
                        	if(gpio_bit & __raw_readl(gpio1_base + AM3517_GPIO_DATAIN))
				{
					lcdreq_ctl.level = 1; 
					/* Lレベル状態で呼ばれたが、既にHレベル状態のとき */
					if(0 == *addr)
					{
						/* ポートの状態をユーザーランドに渡す */
        					printk( KERN_INFO "KENWOOD: LCDREQ port status[%d]\n", lcdreq_ctl.level );
                 			       *addr = lcdreq_ctl.level;

						/* IRQ disable */
						GPIO_IRQ_CTL(GPIO_LCDREQ_INT_NUMBER, 0);
						break;
					}

 				        /* trigger: 'L' LCD power off */
			        	GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_LEVELDETECT0, 0);
			        	GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_LEVELDETECT1, 0);
                        		GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_RISINGDETECT, 0);
                        		GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_FALLINGDETECT, 1);
					printk( KERN_INFO "KENWOOD: waitting LCDREQ 'L' interrupt\n" );
				}
				else
				{
					lcdreq_ctl.level = 0; 
					/* Hレベル状態で呼ばれたが、既にLレベル状態のとき */
					if(1 == *addr)
					{
						/* ポートの状態をユーザーランドに渡す */
        					printk( KERN_INFO "KENWOOD: LCDREQ port status[%d]\n", lcdreq_ctl.level );
                 			       *addr = lcdreq_ctl.level;

						/* IRQ disable */
						GPIO_IRQ_CTL(GPIO_LCDREQ_INT_NUMBER, 0);
						break;
					}

 			        	/* trigger: 'H' LCD power on */
			        	GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_LEVELDETECT0, 0);
			        	GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_LEVELDETECT1, 0);
                        		GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_RISINGDETECT, 1);
                        		GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_FALLINGDETECT, 0);
					printk( KERN_INFO "KENWOOD: waitting LCDREQ 'H' interrupt\n" );
				}
			}

                        if(!lcdreq_ctl.irq_flag)
                        {
                                /* 割り込み設定 *//* ハンドラ実行中は割り込み禁止 */
                                if(0 > (ret = request_irq(LCDREQ_VIRQ_NUMBER, \
                                                            lcdreq_gpio_irq_handler, \
                                                            IRQF_DISABLED | IRQF_NO_SUSPEND | IRQF_ONESHOT, \
                                                            "MainCPU", \
                                                            NULL)))
                                {
                        	        printk( KERN_INFO "KENWOOD: IRQ request failed![%d]\n", ret );
                                	return -1;
                                }
                                lcdreq_ctl.irq_flag = 1;
                        }

		        /* IRQ enable */
		        GPIO_IRQ_CTL(GPIO_LCDREQ_INT_NUMBER, 1);

                       	if(gpio_bit & __raw_readl(gpio1_base + AM3517_GPIO_DATAIN))
			{
				/* Lレベル状態で呼ばれたが、既にHレベル状態のとき */
				if(0 == *addr)
				{
					lcdreq_ctl.level = 1; 
					/* ポートの状態をユーザーランドに渡す */
        				printk( KERN_INFO "KENWOOD: LCDREQ port status[%d]\n", lcdreq_ctl.level );
                 		       *addr = lcdreq_ctl.level;

					/* IRQ disable */
					GPIO_IRQ_CTL(GPIO_LCDREQ_INT_NUMBER, 0);

					/* 割り込み要因をクリア */
					GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_IRQSTATUS1, 1);
					break;
				}
			}
			else
			{
				/* Hレベル状態で呼ばれたが、既にLレベル状態のとき */
				if(1 == *addr)
				{
					lcdreq_ctl.level = 0; 
					/* ポートの状態をユーザーランドに渡す */
        				printk( KERN_INFO "KENWOOD: LCDREQ port status[%d]\n", lcdreq_ctl.level );
                 		       *addr = lcdreq_ctl.level;

					/* IRQ disable */
					GPIO_IRQ_CTL(GPIO_LCDREQ_INT_NUMBER, 0);

					/* 割り込み要因をクリア */
					GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_IRQSTATUS1, 1);
					break;
				}
			}

        		/* ウェートキューで条件待ちスリープ */
        		if(wait_event_interruptible(lcdreq_ctl.queue, lcdreq_ctl.condition_flag != 0))
			{
			/* OSがプロセスフリーズ時に強制的に待ちが解除される */
        			printk( KERN_INFO "KENWOOD: LCDREQ wait rejected by signal interruption!\n" );
				lcdreq_ctl.level = -1;
                        	*addr = lcdreq_ctl.level;
				lcdreq_ctl.suspend_flag = 1;
				/* IRQ disable */
				GPIO_IRQ_CTL(GPIO_LCDREQ_INT_NUMBER, 0);
				return -EINTR;
			}
        		lcdreq_ctl.condition_flag = 0;
			lcdreq_ctl.suspend_flag = 0;

                        /* ポートの状態をユーザーランドに渡す */
        		printk( KERN_INFO "KENWOOD: LCDREQ port status[%d]\n", lcdreq_ctl.level );
                        *addr = lcdreq_ctl.level;

			/* IRQ disable */
			GPIO_IRQ_CTL(GPIO_LCDREQ_INT_NUMBER, 0);
			break;

		/* エッジトリガでスリープから起こすためのテストプログラム */
		case KENWOOD_IOC_MCPU_WAKEUP:
                        if(!lcdreq_ctl.irq_flag)
                        {
                                /* 割り込み設定 *//* ハンドラ実行中は割り込み禁止 */
                                if( 0 > ( ret = request_irq(    LCDREQ_VIRQ_NUMBER, \
                                                                lcdreq_gpio_irq_handler, \
                                                                IRQF_DISABLED, \
                                                                "MainCPU", \
                                                                NULL    )))
                                {
                                printk( KERN_INFO "KENWOOD: IRQ request failed![%d]\n", ret );
                                return -1;
                                }
                                lcdreq_ctl.irq_flag = 1;
                        }

                        /*----- AM3517 GPIO割り込みのレジスタ設定 -----*/
                        index = GPIO_LCDREQ_INT_NUMBER & 0x1f;
                        gpio_bit = 1 << index;

                        /* trigger: rising edge */
                        GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_LEVELDETECT0, 0);
                        GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_LEVELDETECT1, 0);
                        GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_RISINGDETECT, 1);
                        GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_FALLINGDETECT, 0);
                        printk( KERN_INFO "KENWOOD: waitting LCDREQ edge interrupt\n" );

			/* 立ち上がりエッジで割り込み、ハンドラ内で'H'が見えたときだけ有効とする */
			lcdreq_ctl.level = 0;

                        /* IRQ enable */
                        GPIO_REG_CTL(GPIO_LCDREQ_INT_NUMBER, AM3517_GPIO_SETIRQENABLE1, 1);

                        /* ウェートキューで条件待ちスリープ */
                        wait_event_interruptible(lcdreq_ctl.queue, lcdreq_ctl.condition_flag );
                        lcdreq_ctl.condition_flag = 0;
                        break;

		/* LCD出力オン */
		case KENWOOD_IOC_LCD_ON:
			/* use dss_pclk */
        	        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_DSS_PCLK);
			gpio_bit &= ~0x00000007;
        	        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_DSS_PCLK);
			GPIO_REG_CTL(66, AM3517_GPIO_OE, 1);
			GPIO_REG_CTL(66, AM3517_GPIO_DATAOUT, 0);

			/* use dss_acbias */
        	        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_DSS_ACBIAS);
			gpio_bit &= ~0x00070000;
        	        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_DSS_ACBIAS);
			GPIO_REG_CTL(69, AM3517_GPIO_OE, 1);
			GPIO_REG_CTL(69, AM3517_GPIO_DATAOUT, 0);

			DISPC_LCD_CTL(1);
			break;

		/* LCD出力オフ */
		case KENWOOD_IOC_LCD_OFF:
			DISPC_LCD_CTL(0);

			/* use dss_pclk as GPIO 66 */
        	        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_DSS_PCLK);
        	        gpio_bit &= ~0x00000007;
			gpio_bit |= 0x00000004;
        	        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_DSS_PCLK);
			GPIO_REG_CTL(66, AM3517_GPIO_OE, 0);
			GPIO_REG_CTL(66, AM3517_GPIO_DATAOUT, 0);

			/* use dss_acbias as GPIO 69 */
        	        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_DSS_ACBIAS);
        	        gpio_bit &= ~0x00070000;
			gpio_bit |= 0x00040000;
        	        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_DSS_ACBIAS);
			GPIO_REG_CTL(69, AM3517_GPIO_OE, 0);
			GPIO_REG_CTL(69, AM3517_GPIO_DATAOUT, 0);
			break;

		/* LCDバックライトオン */
		case KENWOOD_IOC_BKL_ALL_ON:
			GPIO_REG_CTL(GPIO_MLCD_BKL_NUMBER, AM3517_GPIO_DATAOUT, 1);
			GPIO_REG_CTL(GPIO_SLCD_BKL_NUMBER, AM3517_GPIO_DATAOUT, 1);
			break;

		/* LCDバックライトオフ */
		case KENWOOD_IOC_BKL_ALL_OFF:
			GPIO_REG_CTL(GPIO_MLCD_BKL_NUMBER, AM3517_GPIO_DATAOUT, 0);
			GPIO_REG_CTL(GPIO_SLCD_BKL_NUMBER, AM3517_GPIO_DATAOUT, 0);
			break;

		/* LCDバックライトオン(メインのみ) */
		case KENWOOD_IOC_BKL_MAIN_ON:
			GPIO_REG_CTL(GPIO_MLCD_BKL_NUMBER, AM3517_GPIO_DATAOUT, 1);
			break;

		/* LCDバックライトオフ(メインのみ) */
		case KENWOOD_IOC_BKL_MAIN_OFF:
			GPIO_REG_CTL(GPIO_MLCD_BKL_NUMBER, AM3517_GPIO_DATAOUT, 0);
			break;

		/* DVI出力オン */
		case KENWOOD_IOC_DVI_ON:
			GPIO_REG_CTL(GPIO_DVI_NUMBER, AM3517_GPIO_DATAOUT, 0);
			break;

		/* DVI出力オフ */
		case KENWOOD_IOC_DVI_OFF:
			GPIO_REG_CTL(GPIO_DVI_NUMBER, AM3517_GPIO_DATAOUT, 1);
			break;

		/* VDAC出力オン */
		case KENWOOD_IOC_VDAC_ON:
			GPIO_REG_CTL(GPIO_VDAC_NUMBER, AM3517_GPIO_DATAOUT, 1);
			break;

                /* VDAC出力オフ */
                case KENWOOD_IOC_VDAC_OFF:
                        GPIO_REG_CTL(GPIO_VDAC_NUMBER, AM3517_GPIO_DATAOUT, 0);
                        break;

		/* サブLCDリセット */
		case KENWOOD_IOC_SLCD_RST:
			GPIO_REG_CTL(GPIO_SLCD_RST_NUMBER, AM3517_GPIO_DATAOUT, 0);
			break;

		/* サブLCDリセット解除 */
		case KENWOOD_IOC_SLCD_GO:
			GPIO_REG_CTL(GPIO_SLCD_RST_NUMBER, AM3517_GPIO_DATAOUT, 0);
			GPIO_REG_CTL(GPIO_SLCD_RST_NUMBER, AM3517_GPIO_DATAOUT, 1);
			break;

                /* ヘルスチェックポートON */
                case KENWOOD_IOC_HEALTH_ON:
                        GPIO_REG_CTL(GPIO_HEALTH_NUMBER, AM3517_GPIO_DATAOUT, 1);
                        break;

                /* ヘルスチェックポートOFF */
                case KENWOOD_IOC_HEALTH_OFF:
                        GPIO_REG_CTL(GPIO_HEALTH_NUMBER, AM3517_GPIO_DATAOUT, 0);
                        break;

                /* MS_SELポート状態読み出し */
                case KENWOOD_IOC_RD_MS_SEL:
                        index = GPIO_MS_SELECT_NUMBER & 0x1f;
                        gpio_bit = 1 << index;
                        *addr = __raw_readl(gpio4_base + AM3517_GPIO_DATAIN);
			*addr &= gpio_bit;
                        break;

                /* LCDREQポート状態読み出し */
                case KENWOOD_IOC_RD_LCDREQ:
                        index = GPIO_LCDREQ_INT_NUMBER & 0x1f;
                        gpio_bit = 1 << index;
                        *addr = __raw_readl(gpio1_base + AM3517_GPIO_DATAIN);
			*addr &= gpio_bit;
			if(*addr)
			{
				*addr = 1;
			}
                        break;

		/* ビデオ出力の極性:POSITIVE */
		case KENWOOD_IOC_VIDEO_HS_POS:
			VIDEO_HSYNC_CTL(0);
			DISPC_GOLCD();
			break;

		/* ビデオ出力の極性:NEGATIVE */
		case KENWOOD_IOC_VIDEO_HS_NEG:
			VIDEO_HSYNC_CTL(1);
			DISPC_GOLCD();
			break;

                /* USB バスパワーON */
                case KENWOOD_IOC_USB_BUSPWR_ON:
                        GPIO_REG_CTL(GPIO_USB1_BUSPWR_NUMBER, AM3517_GPIO_DATAOUT, 1);
                        GPIO_REG_CTL(GPIO_USB2_BUSPWR_NUMBER, AM3517_GPIO_DATAOUT, 1);
                        break;

                /* USB バスパワーOFF */
                case KENWOOD_IOC_USB_BUSPWR_OFF:
                        GPIO_REG_CTL(GPIO_USB1_BUSPWR_NUMBER, AM3517_GPIO_DATAOUT, 0);
                        GPIO_REG_CTL(GPIO_USB2_BUSPWR_NUMBER, AM3517_GPIO_DATAOUT, 0);
                        break;

                /* EMAC 10Mbps mode */
                case KENWOOD_IOC_EMAC_10M:
			EMAC_SPEED_CTL(10);
			MDIO_SPEED_CTL(10);
			break;

                /* EMAC 100Mbps mode */
                case KENWOOD_IOC_EMAC_100M:
			EMAC_SPEED_CTL(100);
			MDIO_SPEED_CTL(100);
			break;

                /* VESA 800x600x60Hz mode */
                case KENWOOD_IOC_VESA_800x600:
			CM_DSS_CTL(0);
			DISPC_LCD_800x600();
			DISPC_GOLCD();
			break;

                /* VESA 848x480x60Hz mode */
                case KENWOOD_IOC_VESA_848x480:
			CM_DSS_CTL(1);
			DISPC_LCD_848x480();
			DISPC_GOLCD();
			break;

                /* Kenwood 800x480x60Hz mode */
                case KENWOOD_IOC_KWD_800x480:
			CM_DSS_CTL(1);
			DISPC_LCD_800x480();
			DISPC_GOLCD();
			break;

		case KENWOOD_IOC_SPI1_ENABLE:
			/* use GPIO 171~175 as SPI#1 */
        	        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_MCSPI1_CLK);
			gpio_bit = 0x01100110;
        	        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_MCSPI1_CLK);

        	        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_MCSPI1_SOMI);
			gpio_bit = 0x01100110;
        	        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_MCSPI1_SOMI);

        	        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_MCSPI1_CS1);
        	        gpio_bit &= ~0x0000FFFF;
			gpio_bit |= 0x00000004;
        	        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_MCSPI1_CS1);

			/* GPIO 171-175 default setting */
			gpio_bit = 0x0000F800;
			GPIO6_REGARRAY_CTL(AM3517_GPIO_OE, gpio_bit, 1);
			GPIO6_REGARRAY_CTL(AM3517_GPIO_DATAOUT, gpio_bit, 0);
			break;

		/* invalidate SPI#1 */
		case KENWOOD_IOC_SPI1_DISABLE:
			/* use SPI#1 as GPIO 171~175 */
        	        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_MCSPI1_CLK);
        	        gpio_bit &= ~0x00070007;
			gpio_bit |= 0x00040004;
        	        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_MCSPI1_CLK);

        	        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_MCSPI1_SOMI);
        	        gpio_bit &= ~0x00070007;
			gpio_bit |= 0x00040004;
        	        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_MCSPI1_SOMI);

        	        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_MCSPI1_CS1);
        	        gpio_bit &= ~0x00000007;
			gpio_bit |= 0x00000004;
        	        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_MCSPI1_CS1);

			/* GPIO 171-175 'L' output */
			gpio_bit = 0x0000F800;
			GPIO6_REGARRAY_CTL(AM3517_GPIO_OE, gpio_bit, 0);
			GPIO6_REGARRAY_CTL(AM3517_GPIO_DATAOUT, gpio_bit, 0);
			break;

		case KENWOOD_IOC_BSP2_MAIN_ENABLE:
                        /* use GPIO 116,117,119,160 as BSP#2 */
                        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_MCBSP2_FSX);
                        gpio_bit = 0x01080108;
                        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_MCBSP2_FSX);
                        
                        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_MCBSP2_DR);
                        gpio_bit &= ~0xFFFF0000;
                        gpio_bit |= 0x00080000;
                        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_MCBSP2_DR);

                        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_MCBSP_CLKS);
                        gpio_bit &= ~0x0000FFFF;
                        gpio_bit |= 0x00000100;
                        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_MCBSP_CLKS);

                        /* GPIO 116,117,119,160 default setting */
                        gpio_bit = 0x00b00000;
                        GPIO4_REGARRAY_CTL(AM3517_GPIO_OE, gpio_bit, 1);
                        GPIO4_REGARRAY_CTL(AM3517_GPIO_DATAOUT, gpio_bit, 0);
 			/* 2013.02.20 comment out by Matsuhashi *//* IOCOO(Y52-84-46) */
//			gpio_bit = 0x00000001;
//			GPIO6_REGARRAY_CTL(AM3517_GPIO_OE, gpio_bit, 1);
//			GPIO6_REGARRAY_CTL(AM3517_GPIO_DATAOUT, gpio_bit, 0);

                        /* GPIO 155 'H' output */
			gpio_bit = 0x08000000;
			GPIO5_REGARRAY_CTL(AM3517_GPIO_OE, gpio_bit, 0);
			GPIO5_REGARRAY_CTL(AM3517_GPIO_DATAOUT, gpio_bit, 1);
			break;

		case KENWOOD_IOC_BSP2_MAIN_DISABLE:
                        /* GPIO 155 'L' output */
                        gpio_bit = 0x08000000;
                        GPIO5_REGARRAY_CTL(AM3517_GPIO_OE, gpio_bit, 0);
                        GPIO5_REGARRAY_CTL(AM3517_GPIO_DATAOUT, gpio_bit, 0);

                        /* use BSP#2 as GPIO 116,117,119,160 */
                        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_MCBSP2_FSX);
                        gpio_bit &= ~0x00070007;
                        gpio_bit |= 0x00040004;
                        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_MCBSP2_FSX);

                        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_MCBSP2_DR);
                        gpio_bit &= ~0x00070000;
                        gpio_bit |= 0x00040000;
                        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_MCBSP2_DR);

                        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_MCBSP_CLKS);
                        gpio_bit &= ~0x00000007;
 			/* 2013.02.20 change PADCONF setting by Matsuhashi *//* IOCOO(Y52-84-46) */
//			gpio_bit |= 0x00000004;		/* GPIO */
                        gpio_bit |= 0x00000007;		/* safe mode */
                        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_MCBSP_CLKS);

                        /* GPIO 116,117,119,160 'L' output */
                        gpio_bit = 0x00b00000;
                        GPIO4_REGARRAY_CTL(AM3517_GPIO_OE, gpio_bit, 0);
                        GPIO4_REGARRAY_CTL(AM3517_GPIO_DATAOUT, gpio_bit, 0);
 			/* 2013.02.20 comment out by Matsuhashi *//* IOCOO(Y52-84-46) */
//			gpio_bit = 0x00000001;
//			GPIO6_REGARRAY_CTL(AM3517_GPIO_OE, gpio_bit, 0);
//			GPIO6_REGARRAY_CTL(AM3517_GPIO_DATAOUT, gpio_bit, 0);
			break;

                case KENWOOD_IOC_BSP2_SUB_ENABLE:
                        /* use GPIO 116,117,119 as BSP#2 */
                        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_MCBSP2_FSX);
                        gpio_bit = 0x01080108;
                        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_MCBSP2_FSX);

                        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_MCBSP2_DR);
                        gpio_bit &= ~0xFFFF0000;
                        gpio_bit |= 0x00080000;
                        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_MCBSP2_DR);

                        /* GPIO 116,117,119 default setting */
                        gpio_bit = 0x00b00000;
                        GPIO4_REGARRAY_CTL(AM3517_GPIO_OE, gpio_bit, 1);
                        GPIO4_REGARRAY_CTL(AM3517_GPIO_DATAOUT, gpio_bit, 0);
                        break;

                case KENWOOD_IOC_BSP2_SUB_DISABLE:
                        /* use BSP#2 as GPIO 116,117,119 */
                        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_MCBSP2_FSX);
                        gpio_bit &= ~0x00070007;
                        gpio_bit |= 0x00040004;
                        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_MCBSP2_FSX);

                        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_MCBSP2_DR);
                        gpio_bit &= ~0x00070000;
                        gpio_bit |= 0x00040000;
                        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_MCBSP2_DR);

                        /* GPIO 116,117,119 'L' output */
                        gpio_bit = 0x00b00000;
                        GPIO4_REGARRAY_CTL(AM3517_GPIO_OE, gpio_bit, 0);
                        GPIO4_REGARRAY_CTL(AM3517_GPIO_DATAOUT, gpio_bit, 0);
                        break;

		case KENWOOD_IOC_USB1_OVRCUR:
		        index = GPIO_USB1OC_INT_NUMBER & 0x1f;
		        gpio_bit = 1 << index;

			/* IRQ disable */
			GPIO_IRQ_CTL(GPIO_USB1OC_INT_NUMBER, 0);

			if(!usb1oc_ctl.suspend_flag)
			{
                        	if(gpio_bit & __raw_readl(gpio1_base + AM3517_GPIO_DATAIN))
				{
					/* 正常なときは異常監視を行う */
 				        /* trigger: 'L' */
			        	GPIO_REG_CTL(GPIO_USB1OC_INT_NUMBER, AM3517_GPIO_LEVELDETECT0, 0);
			        	GPIO_REG_CTL(GPIO_USB1OC_INT_NUMBER, AM3517_GPIO_LEVELDETECT1, 0);
                        		GPIO_REG_CTL(GPIO_USB1OC_INT_NUMBER, AM3517_GPIO_RISINGDETECT, 0);
                        		GPIO_REG_CTL(GPIO_USB1OC_INT_NUMBER, AM3517_GPIO_FALLINGDETECT, 1);
					printk( KERN_INFO "KENWOOD: waitting USB1OC 'L' interrupt\n" );
					usb1oc_ctl.level = 1; 
				}
				else
				{
					/* 既に異常発生中のとき */
					usb1oc_ctl.level = 0; 
        		                *addr = usb1oc_ctl.level;
		                        /* ポートの状態をユーザーランドに渡す */
		        		printk( KERN_INFO "KENWOOD: USB1OC port status[%d]\n", usb1oc_ctl.level );
					/* IRQ disable */
					GPIO_IRQ_CTL(GPIO_USB1OC_INT_NUMBER, 0);
					break;
				}
			}

                        if(!usb1oc_ctl.irq_flag)
                        {
                                /* 割り込み設定 *//* ハンドラ実行中は割り込み禁止 */
                                if( 0 > ( ret = request_irq(    USB1OC_VIRQ_NUMBER, \
                                                                usb1oc_gpio_irq_handler, \
                                                                IRQF_DISABLED | IRQF_NO_SUSPEND | IRQF_ONESHOT, \
                                                                "MainCPU", \
                                                                NULL    )))
                                {
                                	printk( KERN_INFO "KENWOOD: IRQ request failed![%d]\n", ret );
                                	return -1;
                                }
                                usb1oc_ctl.irq_flag = 1;
                        }

		        /* IRQ enable */
		        GPIO_IRQ_CTL(GPIO_USB1OC_INT_NUMBER, 1);

			/* 念のためポートの状態を再チェック */
                       	if(gpio_bit & __raw_readl(gpio1_base + AM3517_GPIO_DATAIN))
			{
				;
			}
			else
			{
				/* 既に異常発生中のとき */
				usb1oc_ctl.level = 0; 
				*addr = usb1oc_ctl.level;
				/* ポートの状態をユーザーランドに渡す */
		        	printk( KERN_INFO "KENWOOD: USB1OC port status[%d]\n", usb1oc_ctl.level );
				/* IRQ disable */
				GPIO_IRQ_CTL(GPIO_USB1OC_INT_NUMBER, 0);

				/* 割り込み要因をクリア */
				GPIO_REG_CTL(GPIO_USB1OC_INT_NUMBER, AM3517_GPIO_IRQSTATUS1, 1);
				break;
			}

        		/* ウェートキューで条件待ちスリープ */
        		if(wait_event_interruptible(usb1oc_ctl.queue, usb1oc_ctl.condition_flag != 0 ))
			{
			/* OSがプロセスフリーズ時に強制的に待ちが解除される */
        			printk( KERN_INFO "KENWOOD: USB1OC wait rejected by signal interruption!\n" );
				usb1oc_ctl.level = -1;
                        	*addr = usb1oc_ctl.level;
				usb1oc_ctl.suspend_flag = 1;
				/* IRQ disable */
				GPIO_IRQ_CTL(GPIO_USB1OC_INT_NUMBER, 0);
				return -EINTR;
			}
        		usb1oc_ctl.condition_flag = 0;
			usb1oc_ctl.suspend_flag = 0;

                        /* ポートの状態をユーザーランドに渡す */
        		printk( KERN_INFO "KENWOOD: USB1OC port status[%d]\n", usb1oc_ctl.level );
                        *addr = usb1oc_ctl.level;

			/* IRQ disable */
			GPIO_IRQ_CTL(GPIO_USB1OC_INT_NUMBER, 0);
			break;

		case KENWOOD_IOC_USB2_OVRCUR:
		        index = GPIO_USB2OC_INT_NUMBER & 0x1f;
		        gpio_bit = 1 << index;

			/* IRQ disable */
			GPIO_IRQ_CTL(GPIO_USB2OC_INT_NUMBER, 0);

			if(!usb2oc_ctl.suspend_flag)
			{
                        	if(gpio_bit & __raw_readl(gpio1_base + AM3517_GPIO_DATAIN))
				{
					/* 正常なときは異常監視を行う */
 				        /* trigger: 'L' */
			        	GPIO_REG_CTL(GPIO_USB2OC_INT_NUMBER, AM3517_GPIO_LEVELDETECT0, 0);
			        	GPIO_REG_CTL(GPIO_USB2OC_INT_NUMBER, AM3517_GPIO_LEVELDETECT1, 0);
                        		GPIO_REG_CTL(GPIO_USB2OC_INT_NUMBER, AM3517_GPIO_RISINGDETECT, 0);
                        		GPIO_REG_CTL(GPIO_USB2OC_INT_NUMBER, AM3517_GPIO_FALLINGDETECT, 1);
					printk( KERN_INFO "KENWOOD: waitting USB2OC 'L' interrupt\n" );
					usb2oc_ctl.level = 1; 
				}
				else
				{
					/* 既に異常発生中のとき */
					usb2oc_ctl.level = 0; 
        		                *addr = usb2oc_ctl.level;
		                        /* ポートの状態をユーザーランドに渡す */
		        		printk( KERN_INFO "KENWOOD: USB2OC port status[%d]\n", usb2oc_ctl.level );
					/* IRQ disable */
					GPIO_IRQ_CTL(GPIO_USB2OC_INT_NUMBER, 0);
					break;
				}
			}

                        if(!usb2oc_ctl.irq_flag)
                        {
                                /* 割り込み設定 *//* ハンドラ実行中は割り込み禁止 */
                                if( 0 > ( ret = request_irq(    USB2OC_VIRQ_NUMBER, \
                                                                usb2oc_gpio_irq_handler, \
                                                                IRQF_DISABLED | IRQF_NO_SUSPEND | IRQF_ONESHOT, \
                                                                "MainCPU", \
                                                                NULL    )))
                                {
                                	printk( KERN_INFO "KENWOOD: IRQ request failed![%d]\n", ret );
                                	return -1;
                                }
                                usb2oc_ctl.irq_flag = 1;
                        }

		        /* IRQ enable */
		        GPIO_IRQ_CTL(GPIO_USB2OC_INT_NUMBER, 1);

			/* 念のためポートの状態を再チェック */
                       	if(gpio_bit & __raw_readl(gpio1_base + AM3517_GPIO_DATAIN))
			{
				;
			}
			else
			{
				/* 既に異常発生中のとき */
				usb2oc_ctl.level = 0; 
				*addr = usb2oc_ctl.level;
				/* ポートの状態をユーザーランドに渡す */
		        	printk( KERN_INFO "KENWOOD: USB2OC port status[%d]\n", usb2oc_ctl.level );
				/* IRQ disable */
				GPIO_IRQ_CTL(GPIO_USB2OC_INT_NUMBER, 0);

				/* 割り込み要因をクリア */
				GPIO_REG_CTL(GPIO_USB2OC_INT_NUMBER, AM3517_GPIO_IRQSTATUS1, 1);
				break;
			}

        		/* ウェートキューで条件待ちスリープ */
        		if(wait_event_interruptible(usb2oc_ctl.queue, usb2oc_ctl.condition_flag != 0 ))
			{
			/* OSがプロセスフリーズ時に強制的に待ちが解除される */
        			printk( KERN_INFO "KENWOOD: USB2OC wait rejected by signal interruption!\n" );
				usb2oc_ctl.level = -1;
                        	*addr = usb2oc_ctl.level;
				usb2oc_ctl.suspend_flag = 1;
				/* IRQ disable */
				GPIO_IRQ_CTL(GPIO_USB2OC_INT_NUMBER, 0);
				return -EINTR;
			}
        		usb2oc_ctl.condition_flag = 0;
			usb2oc_ctl.suspend_flag = 0;

                        /* ポートの状態をユーザーランドに渡す */
        		printk( KERN_INFO "KENWOOD: USB2OC port status[%d]\n", usb2oc_ctl.level );
                        *addr = usb2oc_ctl.level;

			/* IRQ disable */
			GPIO_IRQ_CTL(GPIO_USB2OC_INT_NUMBER, 0);
			break;

                /* USB トランシーバON */
                case KENWOOD_IOC_USB_TRANS_ON:
                        GPIO_REG_CTL(GPIO_USB1_TRANS_NUMBER, AM3517_GPIO_DATAOUT, 0);
                        GPIO_REG_CTL(GPIO_USB2_TRANS_NUMBER, AM3517_GPIO_DATAOUT, 0);
                        break;

                /* USB トランシーバOFF */
                case KENWOOD_IOC_USB_TRANS_OFF:
                        GPIO_REG_CTL(GPIO_USB1_TRANS_NUMBER, AM3517_GPIO_DATAOUT, 1);
                        GPIO_REG_CTL(GPIO_USB2_TRANS_NUMBER, AM3517_GPIO_DATAOUT, 1);
                        break;

		/* オーディオDAC/ADC ON */
		case KENWOOD_IOC_AUDIO_ON:
			GPIO_REG_CTL(GPIO_AUDIO_RST_NUMBER, AM3517_GPIO_DATAOUT, 1);
			GPIO_REG_CTL(GPIO_AUDIO_RST_NUMBER, AM3517_GPIO_DATAOUT, 0);
			GPIO_REG_CTL(GPIO_AUDIO_RST_NUMBER, AM3517_GPIO_DATAOUT, 1);
			break;

		/* オーディオDAC/ADC OFF */
		case KENWOOD_IOC_AUDIO_OFF:
			GPIO_REG_CTL(GPIO_AUDIO_RST_NUMBER, AM3517_GPIO_DATAOUT, 0);
			break;

                /* プログラム書き込み完了ポートON */
                case KENWOOD_IOC_PDONE_ON:
                        GPIO_REG_CTL(GPIO_PDONE_NUMBER, AM3517_GPIO_DATAOUT, 1);
                        break;

                /* プログラム書き込み完了ポートOFF */
                case KENWOOD_IOC_PDONE_OFF:
                        GPIO_REG_CTL(GPIO_PDONE_NUMBER, AM3517_GPIO_DATAOUT, 0);
                        break;

		/* POLSELピンを入力に戻す */
		case KENWOOD_IOC_POLSEL_IN:
                        GPIO_REG_CTL(GPIO_VIDEO_POL_NUMBER, AM3517_GPIO_OE, 1);
                        GPIO_REG_CTL(GPIO_VIDEO_POL_NUMBER, AM3517_GPIO_DATAOUT, 0);
			break;

		/* POLSELピンを出力にする */
		case KENWOOD_IOC_POLSEL_OUT:
                        GPIO_REG_CTL(GPIO_VIDEO_POL_NUMBER, AM3517_GPIO_OE, 0);
                        GPIO_REG_CTL(GPIO_VIDEO_POL_NUMBER, AM3517_GPIO_DATAOUT, 0);
			break;

                /* アプリサブヘルスチェックポートON */
                case KENWOOD_IOC_APSHLT_ON:
                        GPIO_REG_CTL(GPIO_APSHLT_NUMBER, AM3517_GPIO_DATAOUT, 1);
                        break;

		/* LCD hsync/vsync activate */
		case KENWOOD_IOC_HVSYNC_ON:
			/* use dss_hsync */
        	        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_DSS_PCLK);
			gpio_bit &= ~0x00070000;
        	        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_DSS_PCLK);
			GPIO_REG_CTL(67, AM3517_GPIO_OE, 1);
			GPIO_REG_CTL(67, AM3517_GPIO_DATAOUT, 0);

			/* use dss_vsync */
        	        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_DSS_VSYNC);
			gpio_bit &= ~0x00000007;
        	        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_DSS_VSYNC);
			GPIO_REG_CTL(68, AM3517_GPIO_OE, 1);
			GPIO_REG_CTL(68, AM3517_GPIO_DATAOUT, 0);
			break;

		/* LCD hsync/vsync de-activate */
		case KENWOOD_IOC_HVSYNC_OFF:
			/* use dss_hsync as GPIO 67 */
        	        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_DSS_PCLK);
        	        gpio_bit &= ~0x00070000;
			gpio_bit |= 0x00040000;
        	        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_DSS_PCLK);
			GPIO_REG_CTL(67, AM3517_GPIO_OE, 0);
			GPIO_REG_CTL(67, AM3517_GPIO_DATAOUT, 0);

			/* use dss_vsync as GPIO 68 */
        	        gpio_bit = __raw_readl(padconf_base + AM3517_PADCONF_DSS_VSYNC);
        	        gpio_bit &= ~0x00000007;
			gpio_bit |= 0x00000004;
        	        __raw_writel(gpio_bit, padconf_base + AM3517_PADCONF_DSS_VSYNC);
			GPIO_REG_CTL(68, AM3517_GPIO_OE, 0);
			GPIO_REG_CTL(68, AM3517_GPIO_DATAOUT, 0);
			break;

		/* APSHLTポート変化待ちスリープ  */
		case KENWOOD_IOC_APSHLT_CTL:
		        index = GPIO_APSHLT_INT_NUMBER & 0x1f;
		        gpio_bit = 1 << index;

			/* IRQ disable */
			GPIO_IRQ_CTL(GPIO_APSHLT_INT_NUMBER, 0);

			if(!apshlt_ctl.suspend_flag)
			{
                        	if(gpio_bit & __raw_readl(gpio5_base + AM3517_GPIO_DATAIN))
				{
 				        /* trigger: 'L' */
			        	GPIO_REG_CTL(GPIO_APSHLT_INT_NUMBER, AM3517_GPIO_LEVELDETECT0, 0);
			        	GPIO_REG_CTL(GPIO_APSHLT_INT_NUMBER, AM3517_GPIO_LEVELDETECT1, 0);
                        		GPIO_REG_CTL(GPIO_APSHLT_INT_NUMBER, AM3517_GPIO_RISINGDETECT, 0);
                        		GPIO_REG_CTL(GPIO_APSHLT_INT_NUMBER, AM3517_GPIO_FALLINGDETECT, 1);
					printk( KERN_INFO "KENWOOD: waitting APSHLT 'L' interrupt\n" );
					apshlt_ctl.level = 1; 
				}
				else
				{
					if(1 == *addr)
					{
						/* 既にLowのとき */
						apshlt_ctl.level = 0; 
        		                	*addr = apshlt_ctl.level;
		                        	/* ポートの状態をユーザーランドに渡す */
		        			printk( KERN_INFO "KENWOOD: APSHLT port status[%d]\n", apshlt_ctl.level );
						/* IRQ disable */
						GPIO_IRQ_CTL(GPIO_APSHLT_INT_NUMBER, 0);
						break;
					}
					else if(0 == *addr)
					{
 						/* trigger: 'L' */
			        		GPIO_REG_CTL(GPIO_APSHLT_INT_NUMBER, AM3517_GPIO_LEVELDETECT0, 0);
			        		GPIO_REG_CTL(GPIO_APSHLT_INT_NUMBER, AM3517_GPIO_LEVELDETECT1, 0);
                        			GPIO_REG_CTL(GPIO_APSHLT_INT_NUMBER, AM3517_GPIO_RISINGDETECT, 0);
                        			GPIO_REG_CTL(GPIO_APSHLT_INT_NUMBER, AM3517_GPIO_FALLINGDETECT, 1);
						printk( KERN_INFO "KENWOOD: waitting APSHLT 'L' interrupt\n" );
						apshlt_ctl.level = 1; 
					}
				}
			}

                        if(!apshlt_ctl.irq_flag)
                        {
                                /* 割り込み設定 *//* ハンドラ実行中は割り込み禁止 */
                                if( 0 > ( ret = request_irq(    APSHLT_VIRQ_NUMBER, \
                                                                apshlt_gpio_irq_handler, \
                                                                IRQF_DISABLED | IRQF_NO_SUSPEND | IRQF_ONESHOT, \
                                                                "AppCPU Sub", \
                                                                NULL    )))
                                {
                                	printk( KERN_INFO "KENWOOD: IRQ request failed![%d]\n", ret );
                                	return -1;
                                }
                                apshlt_ctl.irq_flag = 1;
                        }

		        /* IRQ enable */
		        GPIO_IRQ_CTL(GPIO_APSHLT_INT_NUMBER, 1);

			/* 念のためポートの状態を再チェック */
			if(gpio_bit & __raw_readl(gpio5_base + AM3517_GPIO_DATAIN))
			{
				;
			}
			else
			{
				if(1 == *addr)
				{
					/* 既にLowのとき */
					apshlt_ctl.level = 0; 
					*addr = apshlt_ctl.level;
                        		/* ポートの状態をユーザーランドに渡す */
        				printk( KERN_INFO "KENWOOD: APSHLT port status[%d]\n", apshlt_ctl.level );
					/* IRQ disable */
					GPIO_IRQ_CTL(GPIO_APSHLT_INT_NUMBER, 0);

					/* 割り込み要因をクリア */
					GPIO_REG_CTL(GPIO_APSHLT_INT_NUMBER, AM3517_GPIO_IRQSTATUS1, 1);
					break;
				}
			}

        		/* ウェートキューで条件待ちスリープ */
        		if(wait_event_interruptible(apshlt_ctl.queue, apshlt_ctl.condition_flag != 0 ))
			{
			/* OSがプロセスフリーズ時に強制的に待ちが解除される */
        			printk( KERN_INFO "KENWOOD: APSHLT wait rejected by signal interruption!\n" );
				apshlt_ctl.level = -1;
                        	*addr = apshlt_ctl.level;
				apshlt_ctl.suspend_flag = 1;
				/* IRQ disable */
				GPIO_IRQ_CTL(GPIO_APSHLT_INT_NUMBER, 0);
				return -EINTR;
			}
        		apshlt_ctl.condition_flag = 0;
			apshlt_ctl.suspend_flag = 0;

                        /* ポートの状態をユーザーランドに渡す */
        		printk( KERN_INFO "KENWOOD: APSHLT port status[%d]\n", apshlt_ctl.level );
                        *addr = apshlt_ctl.level;

			/* IRQ disable */
			GPIO_IRQ_CTL(GPIO_APSHLT_INT_NUMBER, 0);
			break;

		default:
			printk( KERN_INFO "KENWOOD: undefined ioctl command![0x%x]\n", command );
			return -EINVAL;
	}

	return 0;
}


static int kenwood_open(struct inode *inode, struct file *filep)
{
	padconf_base = ioremap(AM3517_PADCONF_BASE, SZ_8K);
	cmdss_base = ioremap(AM3517_CMDSS_BASE, SZ_8K);
	dispc_base = ioremap(AM3517_DISPC_BASE, SZ_8K);
	gpio1_base = ioremap(AM3517_GPIO1_BASE, SZ_8K);
	gpio2_base = ioremap(AM3517_GPIO2_BASE, SZ_8K);
	gpio3_base = ioremap(AM3517_GPIO3_BASE, SZ_8K);
	gpio4_base = ioremap(AM3517_GPIO4_BASE, SZ_8K);
	gpio5_base = ioremap(AM3517_GPIO5_BASE, SZ_8K);
	gpio6_base = ioremap(AM3517_GPIO6_BASE, SZ_8K);
	emacss_base = ioremap(AM3517_EMACSS_BASE, SZ_8K);
	emac_base = ioremap(AM3517_EMAC_BASE, SZ_8K);
	mdio_base = ioremap(AM3517_MDIO_BASE, SZ_8K);

	lcdreq_ctl.level = 0;
	lcdreq_ctl.irq_flag = 0;
	lcdreq_ctl.suspend_flag = 0;

	usb1oc_ctl.level = 0;
	usb1oc_ctl.irq_flag = 0;
	usb1oc_ctl.suspend_flag = 0;

	usb2oc_ctl.level = 0;
	usb2oc_ctl.irq_flag = 0;
	usb2oc_ctl.suspend_flag = 0;

	apshlt_ctl.level = 0;
	apshlt_ctl.irq_flag = 0;
	apshlt_ctl.suspend_flag = 0;
	return 0;
}



static int kenwood_release(struct inode *inode, struct file *filep)
{
	if(lcdreq_ctl.irq_flag)
	{
		free_irq(LCDREQ_VIRQ_NUMBER, NULL);
	}

	if(usb1oc_ctl.irq_flag)
	{
		free_irq(USB1OC_VIRQ_NUMBER, NULL);
	}

	if(usb2oc_ctl.irq_flag)
	{
		free_irq(USB2OC_VIRQ_NUMBER, NULL);
	}

	return 0;
}


static struct class *kenwood_class;
static dev_t	dev;
static struct cdev cdev;
static int	devno;

struct file_operations f_ops={
		.owner = 		THIS_MODULE,
		.open = 		kenwood_open,
		.release = 		kenwood_release,
		.unlocked_ioctl = 	kenwood_ioctl,
};

static int am3517_kenwood_init(void)
{
	int	ret;
	int	major;

	/* メジャー番号とマイナー番号を割り当てる */
	alloc_chrdev_region(&dev, 0, 1, "kenwood");
	major = MAJOR(dev);
	devno = MKDEV(major, 0);

	/* キャラクタデバイスの登録 */
	cdev_init(&cdev, &f_ops);
	cdev.owner = THIS_MODULE;
	cdev.ops = &f_ops;
	if( 0 > (ret = cdev_add(&cdev, devno, 1)))
	{
		printk( KERN_INFO "KENWOOD: kenwood original module fail to register\n" );
		return -1;	
	}

	/* デバイスクラスの生成 */
	kenwood_class = class_create(THIS_MODULE, "kenwood");

	/* デバイスファイル（ノード）の生成 */
	device_create( kenwood_class, NULL, devno, NULL, "kenwood" );

        /* ウェートキュー初期化 */
        init_waitqueue_head(&lcdreq_ctl.queue);
        init_waitqueue_head(&usb1oc_ctl.queue);
        init_waitqueue_head(&usb2oc_ctl.queue);
        init_waitqueue_head(&apshlt_ctl.queue);

        return 0;
}

static void am3517_kenwood_exit(void)
{
	device_destroy(kenwood_class, devno);
	class_destroy(kenwood_class);
	cdev_del(&cdev);
	unregister_chrdev_region(dev, 1);

        printk( KERN_INFO "KENWOOD: kenwood original module exit\n" );
}

module_init(am3517_kenwood_init);
module_exit(am3517_kenwood_exit);

MODULE_LICENSE("GPL");

