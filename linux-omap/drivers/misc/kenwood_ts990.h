/*************************************************************************/
/* Linux Device Driver framework                                         */
/*-----------------------------------------------------------------------*/
/* Copyright (C) 2011-2012 JVC KENWOOD CORPORATION. All Rights Reserved. */
/*-----------------------------------------------------------------------*/
/* Description: EX-4120 general purpose driver program                   */
/* Date:        2011.09.26                                               */
/* Author:      K.Matsuhashi                                             */
/*************************************************************************/

/********************************/
/*     レジスタアドレス定義     */
/********************************/
/*
 * AM3517 specific DSS registers
 */
#define AM3517_DSS_BASE                 0x48050000
#define AM3517_DISPC_BASE               0x48050400
#define AM3517_CMDSS_BASE               0x48004E00
#define	AM3517_PADCONF_BASE		0x48002030

#define AM3517_DISPC_CONTROL            0x0040
#define AM3517_DISPC_TIMING_H           0x0064
#define AM3517_DISPC_TIMING_V           0x0068
#define AM3517_DISPC_POL_FREQ           0x006C
#define AM3517_DISPC_SIZE_LCD           0x007C
#define AM3517_DISPC_GFX_SIZE           0x008C
#define AM3517_CMDSS_CLKSEL             0x0040


/*
 * AM3517 specific GPIO registers
 */
#define AM3517_GPIO1_BASE               0x48310000
#define AM3517_GPIO2_BASE               0x49050000
#define AM3517_GPIO3_BASE               0x49052000
#define AM3517_GPIO4_BASE               0x49054000
#define AM3517_GPIO5_BASE               0x49056000
#define AM3517_GPIO6_BASE               0x49058000

#define AM3517_GPIO_IRQSTATUS1          0x0018
#define AM3517_GPIO_IRQENABLE1          0x001c
#define AM3517_GPIO_OE                  0x0034
#define AM3517_GPIO_DATAIN              0x0038
#define AM3517_GPIO_DATAOUT             0x003c
#define AM3517_GPIO_LEVELDETECT0        0x0040
#define AM3517_GPIO_LEVELDETECT1        0x0044
#define AM3517_GPIO_RISINGDETECT        0x0048
#define AM3517_GPIO_DEBOUNCEEN          0x0050
#define AM3517_GPIO_DEBOUNCEVAL         0x0054
#define AM3517_GPIO_FALLINGDETECT       0x004c
#define AM3517_GPIO_CLEARIRQENABLE1     0x0060
#define AM3517_GPIO_SETIRQENABLE1       0x0064

/*
 * AM3517 specific EMAC subsystem registers
 */
#define AM3517_EMACSS_BASE              0x5c000000

#define AM3517_EMACSS_C0RXEN            0x0014
#define AM3517_EMACSS_C0TXEN	        0x0018

/*
 * AM3517 specific EMAC module registers
 */
#define AM3517_EMAC_BASE                0x5c010000

#define AM3517_EMAC_TXCONTROL           0x0004
#define AM3517_EMAC_RXCONTROL           0x0014
#define AM3517_EMAC_MACCONTROL          0x0160

/*
 * AM3517 specific MDIO registers
 */
#define AM3517_MDIO_BASE                0x5c030000

#define AM3517_MDIO_USERACCESS0         0x0080
#define AM3517_MDIO_USERACCESS1         0x0088


/*
 * AM3517 PADCONF registers(0x48002030 + 0x****)
 */
#define	AM3517_PADCONF_DSS_PCLK		0x00a4
#define	AM3517_PADCONF_DSS_VSYNC	0x00a8
#define	AM3517_PADCONF_DSS_ACBIAS	0x00a8
#define	AM3517_PADCONF_CCDC_FIELD	0x01b4
#define	AM3517_PADCONF_UART2_CTS	0x0144
#define	AM3517_PADCONF_MCSPI1_CLK	0x0198
#define	AM3517_PADCONF_MCSPI1_SOMI	0x019c
#define	AM3517_PADCONF_MCSPI1_CS1	0x01a0
#define	AM3517_PADCONF_MCBSP2_FSX	0x010c
#define	AM3517_PADCONF_MCBSP2_DR	0x0110
#define	AM3517_PADCONF_MCBSP_CLKS	0x0164


/******************************/
/*       GPIOポート定義       */
/******************************/
#define GPIO_MLCD_BKL_NUMBER            101                     /* main LCD backlight */
#define GPIO_SLCD_BKL_NUMBER            102                     /* sub LCD backlight */
#define GPIO_DVI_NUMBER                 105                     /* DVI transmitter */
#define GPIO_VDAC_NUMBER                106                     /* VDAC */
#define GPIO_SLCD_RST_NUMBER            29                      /* SUB LCD reset */
#define GPIO_HEALTH_NUMBER              162                     /* Health check */
#define GPIO_VIDEO_POL_NUMBER           18                      /* Video Output Polarity */
#define GPIO_USB1_BUSPWR_NUMBER		12			/* USB bus power 1 */
#define GPIO_USB2_BUSPWR_NUMBER		176			/* USB bus power 2 */
#define GPIO_USB1_TRANS_NUMBER		22			/* USB transceiver 1 */
#define GPIO_USB2_TRANS_NUMBER		24			/* USB transceiver 2 */
#define	GPIO_AUDIO_RST_NUMBER		155			/* Audio DAC */
#define	GPIO_MS_SELECT_NUMBER		99			/* whether CPU is main or sub? */
#define	GPIO_PDONE_NUMBER		154			/* update done indication */
#define	GPIO_APSHLT_NUMBER		159			/* App SUB CPU health check */

#define GPIO_LCDREQ_INT_NUMBER		10			/* LCDREQ Interrupt */
#define LCDREQ_VIRQ_NUMBER		IH_GPIO_BASE + GPIO_LCDREQ_INT_NUMBER	/* virtual IRQ */

#define	GPIO_USB1OC_INT_NUMBER		19			/* USB over-current Interrupt */
#define	USB1OC_VIRQ_NUMBER		IH_GPIO_BASE + GPIO_USB1OC_INT_NUMBER	/* virtual IRQ */

#define	GPIO_USB2OC_INT_NUMBER		20			/* USB over-current Interrupt */
#define	USB2OC_VIRQ_NUMBER		IH_GPIO_BASE + GPIO_USB2OC_INT_NUMBER	/* virtual IRQ */

#define	GPIO_APSHLT_INT_NUMBER		GPIO_APSHLT_NUMBER	/* App SUB CPU health check */
#define	APSHLT_VIRQ_NUMBER		IH_GPIO_BASE + GPIO_APSHLT_INT_NUMBER	/* virtual IRQ */



/******************************/
/*     割り込み制御構造体     */
/******************************/
struct kenwood_intctl_str
{
	wait_queue_head_t	queue;
	int			condition_flag;
	char			irq_flag;
	char			level;
	char			suspend_flag;
};



/*********************/
/* LCD出力制御マクロ */
/*********************/
#define DISPC_LCD_CTL(set) \
        do {    \
                int l = __raw_readl(dispc_base + AM3517_DISPC_CONTROL); \
                if (set) l |= 0x00000001; \
                else l &= ~0x00000001; \
                __raw_writel(l, dispc_base + AM3517_DISPC_CONTROL); \
        } while(0)



/***********************/
/* HSYNC極性制御マクロ */
/***********************/
#define VIDEO_HSYNC_CTL(set) \
        do {    \
                int l = __raw_readl(dispc_base + AM3517_DISPC_POL_FREQ); \
                if (set) l |= 0x00002000; \
                else l &= ~0x00002000; \
                __raw_writel(l, dispc_base + AM3517_DISPC_POL_FREQ); \
        } while(0)



/*****************************/
/* LCDレジスタ更新制御マクロ */
/*****************************/
#define DISPC_GOLCD() \
        do {    \
                int l = __raw_readl(dispc_base + AM3517_DISPC_CONTROL); \
                l |= 0x00000020; \
                __raw_writel(l, dispc_base + AM3517_DISPC_CONTROL); \
        } while(0)



/********************************/
/* LCD解像度制御マクロ(800x600) */
/********************************/
#define DISPC_LCD_800x600() \
        do {    \
                __raw_writel(0x0570277F, dispc_base + AM3517_DISPC_TIMING_H); \
                __raw_writel(0x01700103, dispc_base + AM3517_DISPC_TIMING_V); \
                __raw_writel(0x0257031F, dispc_base + AM3517_DISPC_SIZE_LCD); \
                __raw_writel(0x0257031F, dispc_base + AM3517_DISPC_GFX_SIZE); \
        } while(0)



/********************************/
/* LCD解像度制御マクロ(848x480) */
/********************************/
#define DISPC_LCD_848x480() \
        do {    \
                __raw_writel(0x06F00F6F, dispc_base + AM3517_DISPC_TIMING_H); \
                __raw_writel(0x01700607, dispc_base + AM3517_DISPC_TIMING_V); \
                __raw_writel(0x01DF034F, dispc_base + AM3517_DISPC_SIZE_LCD); \
                __raw_writel(0x01DF034F, dispc_base + AM3517_DISPC_GFX_SIZE); \
        } while(0)


/********************************/
/* LCD解像度制御マクロ(800x480) */
/********************************/
#define DISPC_LCD_800x480() \
        do {    \
                __raw_writel(0x06F00F3F, dispc_base + AM3517_DISPC_TIMING_H); \
                __raw_writel(0x01700607, dispc_base + AM3517_DISPC_TIMING_V); \
                __raw_writel(0x01DF031F, dispc_base + AM3517_DISPC_SIZE_LCD); \
                __raw_writel(0x01DF031F, dispc_base + AM3517_DISPC_GFX_SIZE); \
        } while(0)



/*************************/
/* DSSクロック制御マクロ */
/*************************/
#define CM_DSS_CTL(set) \
        do {    \
                int l; \
                if (set) l = 0x0000100D; \
                else l = 0x0000000B; \
                __raw_writel(l, cmdss_base + AM3517_CMDSS_CLKSEL); \
        } while(0)



/********************************/
/* GPIOレジスタビット設定マクロ */
/********************************/
#define GPIO1_REG_CTL(num, reg, ctl) \
        do {    \
		int shift = num & 0x1f; \
                int bit_mask = 1 << shift; \
                int l = __raw_readl(gpio1_base + reg); \
                if (ctl) l |= bit_mask; \
                else l &= ~bit_mask; \
                __raw_writel(l, gpio1_base + reg); \
        } while(0)

#define GPIO2_REG_CTL(num, reg, ctl) \
        do {    \
		int shift = num & 0x1f; \
                int bit_mask = 1 << shift; \
                int l = __raw_readl(gpio2_base + reg); \
                if (ctl) l |= bit_mask; \
                else l &= ~bit_mask; \
                __raw_writel(l, gpio2_base + reg); \
        } while(0)

#define GPIO3_REG_CTL(num, reg, ctl) \
        do {    \
		int shift = num & 0x1f; \
                int bit_mask = 1 << shift; \
                int l = __raw_readl(gpio3_base + reg); \
                if (ctl) l |= bit_mask; \
                else l &= ~bit_mask; \
                __raw_writel(l, gpio3_base + reg); \
        } while(0)

#define GPIO4_REG_CTL(num, reg, ctl) \
        do {    \
		int shift = num & 0x1f; \
                int bit_mask = 1 << shift; \
                int l = __raw_readl(gpio4_base + reg); \
                if (ctl) l |= bit_mask; \
                else l &= ~bit_mask; \
                __raw_writel(l, gpio4_base + reg); \
        } while(0)

#define GPIO5_REG_CTL(num, reg, ctl) \
        do {    \
		int shift = num & 0x1f; \
                int bit_mask = 1 << shift; \
                int l = __raw_readl(gpio5_base + reg); \
                if (ctl) l |= bit_mask; \
                else l &= ~bit_mask; \
                __raw_writel(l, gpio5_base + reg); \
        } while(0)

#define GPIO6_REG_CTL(num, reg, ctl) \
        do {    \
		int shift = num & 0x1f; \
                int bit_mask = 1 << shift; \
                int l = __raw_readl(gpio6_base + reg); \
                if (ctl) l |= bit_mask; \
                else l &= ~bit_mask; \
                __raw_writel(l, gpio6_base + reg); \
        } while(0)


#define GPIO_REG_CTL(number, reg, ctl) \
        do {    \
                if(0==(number/32))      GPIO1_REG_CTL(number, reg, ctl); \
                else if(1==(number/32)) GPIO2_REG_CTL(number, reg, ctl); \
                else if(2==(number/32)) GPIO3_REG_CTL(number, reg, ctl); \
                else if(3==(number/32)) GPIO4_REG_CTL(number, reg, ctl); \
                else if(4==(number/32)) GPIO5_REG_CTL(number, reg, ctl); \
                else if(5==(number/32)) GPIO6_REG_CTL(number, reg, ctl); \
        } while(0)



/**********************************/
/* GPIOレジスタグループ設定マクロ */
/**********************************/
#define GPIO1_REGARRAY_CTL(reg, bit_mask, ctl) \
        do {    \
                int l = __raw_readl(gpio1_base + reg); \
                if (ctl) l |= bit_mask; \
                else l &= ~bit_mask; \
                __raw_writel(l, gpio1_base + reg); \
        } while(0)

#define GPIO2_REGARRAY_CTL(reg, bit_mask, ctl) \
        do {    \
                int l = __raw_readl(gpio2_base + reg); \
                if (ctl) l |= bit_mask; \
                else l &= ~bit_mask; \
                __raw_writel(l, gpio2_base + reg); \
        } while(0)

#define GPIO3_REGARRAY_CTL(reg, bit_mask, ctl) \
        do {    \
                int l = __raw_readl(gpio3_base + reg); \
                if (ctl) l |= bit_mask; \
                else l &= ~bit_mask; \
                __raw_writel(l, gpio3_base + reg); \
        } while(0)

#define GPIO4_REGARRAY_CTL(reg, bit_mask, ctl) \
        do {    \
                int l = __raw_readl(gpio4_base + reg); \
                if (ctl) l |= bit_mask; \
                else l &= ~bit_mask; \
                __raw_writel(l, gpio4_base + reg); \
        } while(0)

#define GPIO5_REGARRAY_CTL(reg, bit_mask, ctl) \
        do {    \
                int l = __raw_readl(gpio5_base + reg); \
                if (ctl) l |= bit_mask; \
                else l &= ~bit_mask; \
                __raw_writel(l, gpio5_base + reg); \
        } while(0)

#define GPIO6_REGARRAY_CTL(reg, bit_mask, ctl) \
        do {    \
                int l = __raw_readl(gpio6_base + reg); \
                if (ctl) l |= bit_mask; \
                else l &= ~bit_mask; \
                __raw_writel(l, gpio6_base + reg); \
        } while(0)




/**********************************/
/* GPIO割り込みレジスタ設定マクロ */
/**********************************/
#define GPIO1_IRQ_CTL(num, ctl) \
        do {    int shift = num & 0x1f; \
		int bit_mask = 1 << shift; \
                if (ctl) __raw_writel(bit_mask, gpio1_base + AM3517_GPIO_SETIRQENABLE1); \
                else __raw_writel(bit_mask, gpio1_base + AM3517_GPIO_CLEARIRQENABLE1); \
        } while(0)

#define GPIO2_IRQ_CTL(num, ctl) \
        do {    int shift = num & 0x1f; \
		int bit_mask = 1 << shift; \
                if (ctl) __raw_writel(bit_mask, gpio2_base + AM3517_GPIO_SETIRQENABLE1); \
                else __raw_writel(bit_mask, gpio2_base + AM3517_GPIO_CLEARIRQENABLE1); \
        } while(0)

#define GPIO3_IRQ_CTL(num, ctl) \
        do {    int shift = num & 0x1f; \
		int bit_mask = 1 << shift; \
                if (ctl) __raw_writel(bit_mask, gpio3_base + AM3517_GPIO_SETIRQENABLE1); \
                else __raw_writel(bit_mask, gpio3_base + AM3517_GPIO_CLEARIRQENABLE1); \
        } while(0)

#define GPIO4_IRQ_CTL(num, ctl) \
        do {    int shift = num & 0x1f; \
		int bit_mask = 1 << shift; \
                if (ctl) __raw_writel(bit_mask, gpio4_base + AM3517_GPIO_SETIRQENABLE1); \
                else __raw_writel(bit_mask, gpio4_base + AM3517_GPIO_CLEARIRQENABLE1); \
        } while(0)

#define GPIO5_IRQ_CTL(num, ctl) \
        do {    int shift = num & 0x1f; \
		int bit_mask = 1 << shift; \
                if (ctl) __raw_writel(bit_mask, gpio5_base + AM3517_GPIO_SETIRQENABLE1); \
                else __raw_writel(bit_mask, gpio5_base + AM3517_GPIO_CLEARIRQENABLE1); \
        } while(0)

#define GPIO6_IRQ_CTL(num, ctl) \
        do {    int shift = num & 0x1f; \
		int bit_mask = 1 << shift; \
                if (ctl) __raw_writel(bit_mask, gpio6_base + AM3517_GPIO_SETIRQENABLE1); \
                else __raw_writel(bit_mask, gpio6_base + AM3517_GPIO_CLEARIRQENABLE1); \
        } while(0)


#define GPIO_IRQ_CTL(number, ctl) \
        do {    \
                if(0==(number/32))      GPIO1_IRQ_CTL(number, ctl); \
                else if(1==(number/32)) GPIO2_IRQ_CTL(number, ctl); \
                else if(2==(number/32)) GPIO3_IRQ_CTL(number, ctl); \
                else if(3==(number/32)) GPIO4_IRQ_CTL(number, ctl); \
                else if(4==(number/32)) GPIO5_IRQ_CTL(number, ctl); \
                else if(5==(number/32)) GPIO6_IRQ_CTL(number, ctl); \
        } while(0)




/**************************/
/* EMACスピード制御マクロ */
/**************************/
#define EMAC_SPEED_CTL(speed) \
        do {    \
                __raw_writel(0x00000000, emacss_base + AM3517_EMACSS_C0RXEN); \
                __raw_writel(0x00000000, emacss_base + AM3517_EMACSS_C0RXEN); \
                __raw_writel(0x00000000, emac_base + AM3517_EMAC_RXCONTROL); \
                __raw_writel(0x00000000, emac_base + AM3517_EMAC_TXCONTROL); \
                int l = __raw_readl(emac_base + AM3517_EMAC_MACCONTROL); \
                if (100==speed){ l |= 0x00008000; \
                l |= 0x00000001; } \
                else if(10==speed){ l &= ~0x00008000; \
                l |= 0x00000001; } \
                __raw_writel(l, emac_base + AM3517_EMAC_MACCONTROL); \
                __raw_writel(0x00000001, emac_base + AM3517_EMAC_TXCONTROL); \
                __raw_writel(0x00000001, emac_base + AM3517_EMAC_RXCONTROL); \
                __raw_writel(0x000000ff, emacss_base + AM3517_EMACSS_C0RXEN); \
                __raw_writel(0x000000ff, emacss_base + AM3517_EMACSS_C0RXEN); \
        } while(0)



/**************************/
/* MDIOスピード制御マクロ */
/**************************/
#define MDIO_SPEED_CTL(speed) \
        do {    \
                int l = __raw_readl(mdio_base + AM3517_MDIO_USERACCESS1); \
                if (100==speed) l =  0x40002100; \
                else if(10==speed) l = 0x40000100; \
                __raw_writel(l, mdio_base + AM3517_MDIO_USERACCESS1); \
                l |= 0x80000100; \
                __raw_writel(l, mdio_base + AM3517_MDIO_USERACCESS1); \
        } while(0)
