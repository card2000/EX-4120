/*
 * (C) Copyright 2004-2008 Texas Instruments, <www.ti.com>
 * Rohit Choraria <rohitkc@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/mem.h>
#include <asm/arch/omap_gpmc.h>
#include <asm/arch/omap_bch_soft.h>
#include <linux/mtd/nand_ecc.h>
#include <nand.h>

#define PREFETCH_FIFOTHRESHOLD_MAX	0x40
#define PREFETCH_FIFOTHRESHOLD(val)	((val) << 8)
#define CS_NUM_SHIFT		24
#define ENABLE_PREFETCH		(0x1 << 7)

uint8_t cs;
static struct nand_ecclayout hw_nand_oob = GPMC_NAND_HW_ECC_LAYOUT;
#ifdef GPMC_NAND_ECC_LP_x16_LAYOUT
static struct nand_ecclayout hw_nand_oob_kernel = GPMC_NAND_HW_ECC_LAYOUT_KERNEL;
#endif

/*
 * omap_nand_hwcontrol - Set the address pointers corretly for the
 *			following address/data/command operation
 */
static void omap_nand_hwcontrol(struct mtd_info *mtd, int32_t cmd,
				uint32_t ctrl)
{
	register struct nand_chip *this = mtd->priv;

	/*
	 * Point the IO_ADDR to DATA and ADDRESS registers instead
	 * of chip address
	 */
	switch (ctrl) {
	case NAND_CTRL_CHANGE | NAND_CTRL_CLE:
		this->IO_ADDR_W = (void __iomem *)&gpmc_cfg->cs[cs].nand_cmd;
		break;
	case NAND_CTRL_CHANGE | NAND_CTRL_ALE:
		this->IO_ADDR_W = (void __iomem *)&gpmc_cfg->cs[cs].nand_adr;
		break;
	case NAND_CTRL_CHANGE | NAND_NCE:
		this->IO_ADDR_W = (void __iomem *)&gpmc_cfg->cs[cs].nand_dat;
		break;
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

/**
 * omap_read_buf_pref - read data from NAND controller into buffer
 * @mtd: MTD device structure
 * @buf: buffer to store date
 * @len: number of bytes to read
 */
static void omap_read_buf_pref(struct mtd_info *mtd, u_char *buf, int len)
{
	uint32_t r_count = 0;
	int ret = 0;
	u16 *buf16 = (u16 *)buf;
	u32 *p = (u32 *)buf;
	u32 config1;
	int i;
	register struct nand_chip *this = mtd->priv;
	u32 status;

	/* configure and start prefetch transfer */
	config1 = readl(&gpmc_cfg->prefetch_control);
	if (0 == config1) {
		/* Set the amount of bytes to be prefetched */
		writel(len, &gpmc_cfg->prefetch_config2);

		/* Set dma/mpu mode, the prefetch read / post write and
		 * enable the engine. Set which cs is has requested for.
		 */
		writel((cs << CS_NUM_SHIFT) |
			PREFETCH_FIFOTHRESHOLD(PREFETCH_FIFOTHRESHOLD_MAX) |
			ENABLE_PREFETCH, &gpmc_cfg->prefetch_config1);

		/*  Start the prefetch engine */
		writel(0x1, &gpmc_cfg->prefetch_control);

		ret = 0;
	} else {
		ret = 1;
	}

	if (ret) {
		/* PFPW engine is busy, use cpu copy method */
		if (this->options & NAND_BUSWIDTH_16) {
			len >>= 1;
			for (i = 0; i < len; i++)
				*(buf16+i) = readw(this->IO_ADDR_R);
		} else {
			for (i = 0; i < len; i++)
				*(buf+i) = readb(this->IO_ADDR_R);
		}
	} else {
		do {
			status = readl(&gpmc_cfg->prefetch_status);
			r_count = (status >> 24) & 0x7F;
			if(r_count != ((len > PREFETCH_FIFOTHRESHOLD_MAX) ? PREFETCH_FIFOTHRESHOLD_MAX : len)) {
				continue;
			}
			r_count = r_count >> 2;
			for(i=0;i<r_count;i++) {
				*p++ = readl(this->IO_ADDR_R);
			}
			len -= r_count << 2;
		} while (len);
		/* disable and stop the PFPW engine */

		/* Stop the PFPW engine */
		writel(0x0, &gpmc_cfg->prefetch_control);

		/* Reset/disable the PFPW engine */
		writel(0x00004000, &gpmc_cfg->prefetch_config1);
	}
}

/*
 * omap_hwecc_init - Initialize the Hardware ECC for NAND flash in
 *                   GPMC controller
 * @mtd:        MTD device structure
 *
 */
static void omap_hwecc_init(struct nand_chip *chip)
{
	/*
	 * Init ECC Control Register
	 * Clear all ECC | Enable Reg1
	 */
	writel(ECCCLEAR | ECCRESULTREG1, &gpmc_cfg->ecc_control);
	writel(ECCSIZE1 | ECCSIZE0 | ECCSIZE0SEL, &gpmc_cfg->ecc_size_config);
}

/*
 * gen_true_ecc - This function will generate true ECC value, which
 * can be used when correcting data read from NAND flash memory core
 *
 * @ecc_buf:	buffer to store ecc code
 *
 * @return:	re-formatted ECC value
 */
static uint32_t gen_true_ecc(uint8_t *ecc_buf)
{
	return ecc_buf[0] | (ecc_buf[1] << 16) | ((ecc_buf[2] & 0xF0) << 20) |
		((ecc_buf[2] & 0x0F) << 8);
}

/*
 * omap_correct_data - Compares the ecc read from nand spare area with ECC
 * registers values and corrects one bit error if it has occured
 * Further details can be had from OMAP TRM and the following selected links:
 * http://en.wikipedia.org/wiki/Hamming_code
 * http://www.cs.utexas.edu/users/plaxton/c/337/05f/slides/ErrorCorrection-4.pdf
 *
 * @mtd:		 MTD device structure
 * @dat:		 page data
 * @read_ecc:		 ecc read from nand flash
 * @calc_ecc:		 ecc read from ECC registers
 *
 * @return 0 if data is OK or corrected, else returns -1
 */
static int omap_correct_data(struct mtd_info *mtd, uint8_t *dat,
				uint8_t *read_ecc, uint8_t *calc_ecc)
{
	uint32_t orig_ecc, new_ecc, res, hm;
	uint16_t parity_bits, byte;
	uint8_t bit;

	/* Regenerate the orginal ECC */
	orig_ecc = gen_true_ecc(read_ecc);
	new_ecc = gen_true_ecc(calc_ecc);
	/* Get the XOR of real ecc */
	res = orig_ecc ^ new_ecc;
	if (res) {
		/* Get the hamming width */
		hm = hweight32(res);
		/* Single bit errors can be corrected! */
		if (hm == 12) {
			/* Correctable data! */
			parity_bits = res >> 16;
			bit = (parity_bits & 0x7);
			byte = (parity_bits >> 3) & 0x1FF;
			/* Flip the bit to correct */
			dat[byte] ^= (0x1 << bit);
		} else if (hm == 1) {
			printf("Error: Ecc is wrong\n");
			/* ECC itself is corrupted */
			return 2;
		} else {
			/*
			 * hm distance != parity pairs OR one, could mean 2 bit
			 * error OR potentially be on a blank page..
			 * orig_ecc: contains spare area data from nand flash.
			 * new_ecc: generated ecc while reading data area.
			 * Note: if the ecc = 0, all data bits from which it was
			 * generated are 0xFF.
			 * The 3 byte(24 bits) ecc is generated per 512byte
			 * chunk of a page. If orig_ecc(from spare area)
			 * is 0xFF && new_ecc(computed now from data area)=0x0,
			 * this means that data area is 0xFF and spare area is
			 * 0xFF. A sure sign of a erased page!
			 */
			if ((orig_ecc == 0x0FFF0FFF) && (new_ecc == 0x00000000))
				return 0;
			printf("Error: Bad compare! failed\n");
			/* detected 2 bit error */
			return -1;
		}
	}
	return 0;
}

/*
 *  omap_calculate_ecc - Generate non-inverted ECC bytes.
 *
 *  Using noninverted ECC can be considered ugly since writing a blank
 *  page ie. padding will clear the ECC bytes. This is no problem as
 *  long nobody is trying to write data on the seemingly unused page.
 *  Reading an erased page will produce an ECC mismatch between
 *  generated and read ECC bytes that has to be dealt with separately.
 *  E.g. if page is 0xFF (fresh erased), and if HW ECC engine within GPMC
 *  is used, the result of read will be 0x0 while the ECC offsets of the
 *  spare area will be 0xFF which will result in an ECC mismatch.
 *  @mtd:	MTD structure
 *  @dat:	unused
 *  @ecc_code:	ecc_code buffer
 */
static int omap_calculate_ecc(struct mtd_info *mtd, const uint8_t *dat,
				uint8_t *ecc_code)
{
	u_int32_t val;

	/* Start Reading from HW ECC1_Result = 0x200 */
	val = readl(&gpmc_cfg->ecc1_result);

	ecc_code[0] = val & 0xFF;
	ecc_code[1] = (val >> 16) & 0xFF;
	ecc_code[2] = ((val >> 8) & 0x0F) | ((val >> 20) & 0xF0);

	/*
	 * Stop reading anymore ECC vals and clear old results
	 * enable will be called if more reads are required
	 */
	writel(0x000, &gpmc_cfg->ecc_config);

	return 0;
}

/*
 * omap_enable_ecc - This function enables the hardware ecc functionality
 * @mtd:        MTD device structure
 * @mode:       Read/Write mode
 */
static void omap_enable_hwecc(struct mtd_info *mtd, int32_t mode)
{
	struct nand_chip *chip = mtd->priv;
	uint32_t val, dev_width = (chip->options & NAND_BUSWIDTH_16) >> 1;

	switch (mode) {
	case NAND_ECC_READ:
	case NAND_ECC_WRITE:
		/* Clear the ecc result registers, select ecc reg as 1 */
		writel(ECCCLEAR | ECCRESULTREG1, &gpmc_cfg->ecc_control);

		/*
		 * Size 0 = 0xFF, Size1 is 0xFF - both are 512 bytes
		 * tell all regs to generate size0 sized regs
		 * we just have a single ECC engine for all CS
		 */
		writel(ECCSIZE1 | ECCSIZE0 | ECCSIZE0SEL,
			&gpmc_cfg->ecc_size_config);
		val = (dev_width << 7) | (cs << 1) | (0x1);
		writel(val, &gpmc_cfg->ecc_config);
		break;
	default:
		printf("Error: Unrecognized Mode[%d]!\n", mode);
		break;
	}
}

/*
 * omap_nand_switch_ecc - switch the ECC operation b/w h/w ecc and s/w ecc.
 * The default is to come up on s/w ecc
 *
 * @hardware - 1 -switch to 1-bit h/w ecc (kernel/FS layout), 
 *			   2 -switch to 1-bit h/w ecc (xloader/uboot layout)
 *			   (0 -default value)
 */
void omap_nand_switch_ecc(nand_ecc_modes_t mode, int32_t hardware)
{
	struct nand_chip *nand;
	struct mtd_info *mtd;
	uint32_t dev_width;

	if (nand_curr_device < 0 ||
		nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE) {
			printf("Error: Can't switch ecc, no devices available\n");
			return;
	}

	mtd = &nand_info[nand_curr_device];
	nand = mtd->priv;

	nand->options |= NAND_OWN_BUFFERS;

	/* Reset ecc interface */
	nand->ecc.read_page = NULL;
	nand->ecc.write_page = NULL;
	nand->ecc.read_oob = NULL;
	nand->ecc.write_oob = NULL;
	nand->ecc.hwctl = NULL;
	nand->ecc.correct = NULL;
	nand->ecc.calculate = NULL;

	dev_width = (nand->options & NAND_BUSWIDTH_16) >> 1;

	switch(mode)
	  {
	  case NAND_ECC_HW:
		nand->ecc.mode = NAND_ECC_HW;
#ifdef GPMC_NAND_ECC_LP_x16_LAYOUT
		nand->ecc.layout = (hardware == 1) ? &hw_nand_oob_kernel : &hw_nand_oob;
#else
		nand->ecc.layout = &hw_nand_oob;
#endif
		nand->ecc.hwctl = omap_enable_hwecc;
		nand->ecc.size = 512;
		nand->ecc.bytes = 3;
		nand->ecc.correct = omap_correct_data;
		nand->ecc.calculate = omap_calculate_ecc;
		omap_hwecc_init(nand);
#ifdef GPMC_NAND_ECC_LP_x16_LAYOUT
		printf("HW ECC [%s layout] selected\n",(hardware == 1) ? "Kernel/FS" : "X-loader/U-boot");
#else
		printf("HW ECC selected\n");
#endif
		break;
	  case NAND_ECC_SOFT:
		nand->ecc.mode = NAND_ECC_SOFT;
		/* Use mtd default settings */
		nand->ecc.layout = NULL;
		printf("SW ECC selected\n");
		break;
	  case NAND_ECC_4BIT_SOFT:
		nand->ecc.mode = mode;
		nand->ecc.layout = omap_get_ecc_layout_bch(dev_width, 4);
		nand->ecc.hwctl = omap_enable_hwecc_bch4;
		nand->ecc.size = 2048;
		nand->ecc.bytes = 28;
		nand->ecc.calculate = omap_calculate_ecc_bch4;
		nand->ecc.correct = omap_correct_data_bch4;
		omap_hwecc_init_bch(nand);
		printf("4 BIT SW ECC selected\n");
		break;
	  case NAND_ECC_8BIT_SOFT:
		nand->ecc.mode = mode;
		nand->ecc.layout = omap_get_ecc_layout_bch(dev_width, 8);
		nand->ecc.hwctl = omap_enable_hwecc_bch8;
		nand->ecc.size = 2048;
		nand->ecc.bytes = 52;
		nand->ecc.calculate = omap_calculate_ecc_bch8;
		nand->ecc.correct = omap_correct_data_bch8;
		omap_hwecc_init_bch(nand);
		printf("8 BIT SW ECC selected\n");
		break;
	  default:
		break;	
	  }

	/* Update NAND handling after ECC mode switch */
	nand_scan_tail(mtd);

	nand->options &= ~NAND_OWN_BUFFERS;
}

/*
 * Board-specific NAND initialization. The following members of the
 * argument are board-specific:
 * - IO_ADDR_R: address to read the 8 I/O lines of the flash device
 * - IO_ADDR_W: address to write the 8 I/O lines of the flash device
 * - cmd_ctrl: hardwarespecific function for accesing control-lines
 * - waitfunc: hardwarespecific function for accesing device ready/busy line
 * - ecc.hwctl: function to enable (reset) hardware ecc generator
 * - ecc.mode: mode of ecc, see defines
 * - chip_delay: chip dependent delay for transfering data from array to
 *   read regs (tR)
 * - options: various chip options. They can partly be set to inform
 *   nand_scan about special functionality. See the defines for further
 *   explanation
 */
int board_nand_init(struct nand_chip *nand)
{
	u32 base;
	int32_t gpmc_config = 0;
	cs = 0;

	/*
	 * xloader/Uboot's gpmc configuration would have configured GPMC for
	 * nand type of memory. The following logic scans and latches on to the
	 * first CS with NAND type memory.
	 * TBD: need to make this logic generic to handle multiple CS NAND
	 * devices.
	 */
	while (cs < GPMC_MAX_CS) {
	  /* Check if NAND type is set */
	  if ((readl(&gpmc_cfg->cs[cs].config1) & 0xC00) == 0x800) {
	    /* Found it!! */
	    break;
	  }
	  cs++;
	}
	if (cs >= GPMC_MAX_CS) {
	  printf("NAND: Unable to find NAND settings in "
	    "GPMC Configuration - quitting\n");
	  return -ENODEV;
	}

	gpmc_config = readl(&gpmc_cfg->config);
	/* Disable Write protect */
	gpmc_config |= 0x10;
	writel(gpmc_config, &gpmc_cfg->config);

	base = readl(&gpmc_cfg->cs[cs].config7);
	nand->IO_ADDR_R = (void __iomem *)((base & 0x3F) << 24);
	nand->IO_ADDR_W = (void __iomem *)&gpmc_cfg->cs[cs].nand_cmd;

	nand->cmd_ctrl = omap_nand_hwcontrol;
	nand->options = NAND_NO_PADDING | NAND_CACHEPRG | NAND_NO_AUTOINCR;
	/* If we are 16 bit dev, our gpmc config tells us that */
	if ((readl(&gpmc_cfg->cs[cs].config1) & 0x3000) == 0x1000)
	  nand->options |= NAND_BUSWIDTH_16;

	/* fallback ecc info, this will be overridden by
	 * ti81xx_nand_switch_ecc() below to 1-bit h/w ecc
	 */
	nand->chip_delay = 100;

	nand->ecc.mode = NAND_ECC_4BIT_SOFT;

	/* Making 1-bit hw ecc (kernel/FS layout) as default ecc scheme */
	nand_curr_device = 0;
	omap_nand_switch_ecc(NAND_ECC_HW, 1);

	nand->read_buf = omap_read_buf_pref;

	return 0;
}
