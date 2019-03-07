/*
 * ispccdc.c
 *
 * TI OMAP3 ISP - CCDC module
 *
 * Copyright (C) 2009-2010 Nokia Corporation
 * Copyright (C) 2009 Texas Instruments, Inc.
 *
 * Contacts: Laurent Pinchart <laurent.pinchart@ideasonboard.com>
 *	     Sakari Ailus <sakari.ailus@maxwell.research.nokia.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-event.h>

#include "isp.h"
#include "ispreg.h"
#include "ispccdc.h"

static struct v4l2_mbus_framefmt *
__ccdc_get_format(struct isp_ccdc_device *ccdc, struct v4l2_subdev_fh *fh,
		  unsigned int pad, enum v4l2_subdev_format_whence which);

static const unsigned int ccdc_fmts[] = {
	V4L2_MBUS_FMT_SGRBG10_1X10,
	V4L2_MBUS_FMT_SRGGB10_1X10,
	V4L2_MBUS_FMT_SBGGR10_1X10,
	V4L2_MBUS_FMT_SGBRG10_1X10,
	V4L2_MBUS_FMT_SGRBG12_1X12,
	V4L2_MBUS_FMT_SRGGB12_1X12,
	V4L2_MBUS_FMT_SBGGR12_1X12,
	V4L2_MBUS_FMT_SGBRG12_1X12,
	V4L2_MBUS_FMT_UYVY8_2X8,
	V4L2_MBUS_FMT_YUYV8_2X8,
};

/*
 * ispccdc_print_status - Print current CCDC Module register values.
 * @ccdc: Pointer to ISP CCDC device.
 *
 * Also prints other debug information stored in the CCDC module.
 */
#define CCDC_PRINT_REGISTER(isp, name)\
	dev_dbg(isp->dev, "###CCDC " #name "=0x%08x\n", \
		isp_reg_readl(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_##name))

static void ispccdc_print_status(struct isp_ccdc_device *ccdc)
{
	struct isp_device *isp = to_isp_device(ccdc);

	dev_dbg(isp->dev, "-------------CCDC Register dump-------------\n");

	CCDC_PRINT_REGISTER(isp, PCR);
	CCDC_PRINT_REGISTER(isp, SYN_MODE);
	CCDC_PRINT_REGISTER(isp, HD_VD_WID);
	CCDC_PRINT_REGISTER(isp, PIX_LINES);
	CCDC_PRINT_REGISTER(isp, HORZ_INFO);
	CCDC_PRINT_REGISTER(isp, VERT_START);
	CCDC_PRINT_REGISTER(isp, VERT_LINES);
	CCDC_PRINT_REGISTER(isp, CULLING);
	CCDC_PRINT_REGISTER(isp, HSIZE_OFF);
	CCDC_PRINT_REGISTER(isp, SDOFST);
	CCDC_PRINT_REGISTER(isp, SDR_ADDR);
	CCDC_PRINT_REGISTER(isp, CLAMP);
	CCDC_PRINT_REGISTER(isp, DCSUB);
	CCDC_PRINT_REGISTER(isp, COLPTN);
	CCDC_PRINT_REGISTER(isp, BLKCMP);
	CCDC_PRINT_REGISTER(isp, FPC);
	CCDC_PRINT_REGISTER(isp, FPC_ADDR);
	CCDC_PRINT_REGISTER(isp, VDINT);
	CCDC_PRINT_REGISTER(isp, ALAW);
	CCDC_PRINT_REGISTER(isp, REC656IF);
	CCDC_PRINT_REGISTER(isp, CFG);
	CCDC_PRINT_REGISTER(isp, FMTCFG);
	CCDC_PRINT_REGISTER(isp, FMT_HORZ);
	CCDC_PRINT_REGISTER(isp, FMT_VERT);
	CCDC_PRINT_REGISTER(isp, PRGEVEN0);
	CCDC_PRINT_REGISTER(isp, PRGEVEN1);
	CCDC_PRINT_REGISTER(isp, PRGODD0);
	CCDC_PRINT_REGISTER(isp, PRGODD1);
	CCDC_PRINT_REGISTER(isp, VP_OUT);
	CCDC_PRINT_REGISTER(isp, LSC_CONFIG);
	CCDC_PRINT_REGISTER(isp, LSC_INITIAL);
	CCDC_PRINT_REGISTER(isp, LSC_TABLE_BASE);
	CCDC_PRINT_REGISTER(isp, LSC_TABLE_OFFSET);

	dev_dbg(isp->dev, "--------------------------------------------\n");
}

/*
 * ispccdc_busy - Get busy state of the CCDC.
 * @ccdc: Pointer to ISP CCDC device.
 */
int ispccdc_busy(struct isp_ccdc_device *ccdc)
{
	struct isp_device *isp = to_isp_device(ccdc);

	return isp_reg_readl(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_PCR) &
		ISPCCDC_PCR_BUSY;
}

/* -----------------------------------------------------------------------------
 * Lens Shading Compensation
 */

/*
 * ispccdc_lsc_validate_config - Check that LSC configuration is valid.
 * @ccdc: Pointer to ISP CCDC device.
 * @lsc_cfg: the LSC configuration to check.
 *
 * Returns 0 if the LSC configuration is valid, or -EINVAL if invalid.
 */
static int ispccdc_lsc_validate_config(struct isp_ccdc_device *ccdc,
				       struct omap3isp_ccdc_lsc_config *lsc_cfg)
{
	struct isp_device *isp = to_isp_device(ccdc);
	struct v4l2_mbus_framefmt *format;
	unsigned int paxel_width, paxel_height;
	unsigned int paxel_shift_x, paxel_shift_y;
	unsigned int min_width, min_height, min_size;
	unsigned int input_width, input_height;

	paxel_shift_x = lsc_cfg->gain_mode_m;
	paxel_shift_y = lsc_cfg->gain_mode_n;

	if ((paxel_shift_x < 2) || (paxel_shift_x > 6) ||
	    (paxel_shift_y < 2) || (paxel_shift_y > 6)) {
		dev_dbg(isp->dev, "CCDC: LSC: Invalid paxel size\n");
		return -EINVAL;
	}

	if (lsc_cfg->offset & 3) {
		dev_dbg(isp->dev, "CCDC: LSC: Offset must be a multiple of "
			"4\n");
		return -EINVAL;
	}

	if ((lsc_cfg->initial_x & 1) || (lsc_cfg->initial_y & 1)) {
		dev_dbg(isp->dev, "CCDC: LSC: initial_x and y must be even\n");
		return -EINVAL;
	}

	format = __ccdc_get_format(ccdc, NULL, CCDC_PAD_SINK,
				   V4L2_SUBDEV_FORMAT_ACTIVE);
	input_width = format->width;
	input_height = format->height;

	/* Calculate minimum bytesize for validation */
	paxel_width = 1 << paxel_shift_x;
	min_width = ((input_width + lsc_cfg->initial_x + paxel_width - 1)
		     >> paxel_shift_x) + 1;

	paxel_height = 1 << paxel_shift_y;
	min_height = ((input_height + lsc_cfg->initial_y + paxel_height - 1)
		     >> paxel_shift_y) + 1;

	min_size = 4 * min_width * min_height;
	if (min_size > lsc_cfg->size) {
		dev_dbg(isp->dev, "CCDC: LSC: too small table\n");
		return -EINVAL;
	}
	if (lsc_cfg->offset < (min_width * 4)) {
		dev_dbg(isp->dev, "CCDC: LSC: Offset is too small\n");
		return -EINVAL;
	}
	if ((lsc_cfg->size / lsc_cfg->offset) < min_height) {
		dev_dbg(isp->dev, "CCDC: LSC: Wrong size/offset combination\n");
		return -EINVAL;
	}
	return 0;
}

/*
 * ispccdc_lsc_program_table - Program Lens Shading Compensation table address.
 * @ccdc: Pointer to ISP CCDC device.
 */
static void ispccdc_lsc_program_table(struct isp_ccdc_device *ccdc, u32 addr)
{
	isp_reg_writel(to_isp_device(ccdc), addr,
		       OMAP3_ISP_IOMEM_CCDC, ISPCCDC_LSC_TABLE_BASE);
}

/*
 * ispccdc_lsc_setup_regs - Configures the lens shading compensation module
 * @ccdc: Pointer to ISP CCDC device.
 */
static void ispccdc_lsc_setup_regs(struct isp_ccdc_device *ccdc,
				   struct omap3isp_ccdc_lsc_config *cfg)
{
	struct isp_device *isp = to_isp_device(ccdc);
	int reg;

	isp_reg_writel(isp, cfg->offset, OMAP3_ISP_IOMEM_CCDC,
		       ISPCCDC_LSC_TABLE_OFFSET);

	reg = 0;
	reg |= cfg->gain_mode_n << ISPCCDC_LSC_GAIN_MODE_N_SHIFT;
	reg |= cfg->gain_mode_m << ISPCCDC_LSC_GAIN_MODE_M_SHIFT;
	reg |= cfg->gain_format << ISPCCDC_LSC_GAIN_FORMAT_SHIFT;
	isp_reg_writel(isp, reg, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_LSC_CONFIG);

	reg = 0;
	reg &= ~ISPCCDC_LSC_INITIAL_X_MASK;
	reg |= cfg->initial_x << ISPCCDC_LSC_INITIAL_X_SHIFT;
	reg &= ~ISPCCDC_LSC_INITIAL_Y_MASK;
	reg |= cfg->initial_y << ISPCCDC_LSC_INITIAL_Y_SHIFT;
	isp_reg_writel(isp, reg, OMAP3_ISP_IOMEM_CCDC,
		       ISPCCDC_LSC_INITIAL);
}

static int ispccdc_lsc_wait_prefetch(struct isp_ccdc_device *ccdc)
{
	struct isp_device *isp = to_isp_device(ccdc);
	unsigned int wait;

	isp_reg_writel(isp, IRQ0STATUS_CCDC_LSC_PREF_COMP_IRQ,
		       OMAP3_ISP_IOMEM_MAIN, ISP_IRQ0STATUS);

	/* timeout 1 ms */
	for (wait = 0; wait < 1000; wait++) {
		if (isp_reg_readl(isp, OMAP3_ISP_IOMEM_MAIN, ISP_IRQ0STATUS) &
				  IRQ0STATUS_CCDC_LSC_PREF_COMP_IRQ) {
			isp_reg_writel(isp, IRQ0STATUS_CCDC_LSC_PREF_COMP_IRQ,
				       OMAP3_ISP_IOMEM_MAIN, ISP_IRQ0STATUS);
			return 0;
		}

		rmb();
		udelay(1);
	}

	return -ETIMEDOUT;
}

/*
 * __ispccdc_lsc_enable - Enables/Disables the Lens Shading Compensation module.
 * @ccdc: Pointer to ISP CCDC device.
 * @enable: 0 Disables LSC, 1 Enables LSC.
 */
static int __ispccdc_lsc_enable(struct isp_ccdc_device *ccdc, int enable)
{
	struct isp_device *isp = to_isp_device(ccdc);
	const struct v4l2_mbus_framefmt *format =
		__ccdc_get_format(ccdc, NULL, CCDC_PAD_SINK,
				  V4L2_SUBDEV_FORMAT_ACTIVE);

	if ((format->code != V4L2_MBUS_FMT_SGRBG10_1X10) &&
	    (format->code != V4L2_MBUS_FMT_SRGGB10_1X10) &&
	    (format->code != V4L2_MBUS_FMT_SBGGR10_1X10) &&
	    (format->code != V4L2_MBUS_FMT_SGBRG10_1X10))
		return -EINVAL;

	if (enable)
		isp_sbl_enable(isp, OMAP3_ISP_SBL_CCDC_LSC_READ);

	isp_reg_clr_set(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_LSC_CONFIG,
			ISPCCDC_LSC_ENABLE, enable ? ISPCCDC_LSC_ENABLE : 0);

	if (enable) {
		if (ispccdc_lsc_wait_prefetch(ccdc) < 0) {
			isp_reg_clr(isp, OMAP3_ISP_IOMEM_CCDC,
				    ISPCCDC_LSC_CONFIG, ISPCCDC_LSC_ENABLE);
			ccdc->lsc.state = LSC_STATE_STOPPED;
			dev_warn(to_device(ccdc), "LSC prefecth timeout\n");
			return -ETIMEDOUT;
		}
		ccdc->lsc.state = LSC_STATE_RUNNING;
	} else {
		ccdc->lsc.state = LSC_STATE_STOPPING;
	}

	return 0;
}

static int ispccdc_lsc_busy(struct isp_ccdc_device *ccdc)
{
	struct isp_device *isp = to_isp_device(ccdc);

	return isp_reg_readl(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_LSC_CONFIG) &
			     ISPCCDC_LSC_BUSY;
}

/* __ispccdc_lsc_configure - Apply a new configuration to the LSC engine
 * @ccdc: Pointer to ISP CCDC device
 * @req: New configuration request
 *
 * context: in_interrupt()
 */
static int __ispccdc_lsc_configure(struct isp_ccdc_device *ccdc,
				   struct ispccdc_lsc_config_req *req)
{
	if (!req->enable)
		return -EINVAL;

	if (ispccdc_lsc_validate_config(ccdc, &req->config) < 0) {
		dev_dbg(to_device(ccdc), "Discard LSC configuration\n");
		return -EINVAL;
	}

	if (ispccdc_lsc_busy(ccdc))
		return -EBUSY;

	ispccdc_lsc_setup_regs(ccdc, &req->config);
	ispccdc_lsc_program_table(ccdc, req->table);
	return 0;
}

/*
 * ispccdc_lsc_error_handler - Handle LSC prefetch error scenario.
 * @ccdc: Pointer to ISP CCDC device.
 *
 * Disables LSC, and defers enablement to shadow registers update time.
 */
static void ispccdc_lsc_error_handler(struct isp_ccdc_device *ccdc)
{
	struct isp_device *isp = to_isp_device(ccdc);
	/*
	 * From OMAP3 TRM: When this event is pending, the module
	 * goes into transparent mode (output =input). Normal
	 * operation can be resumed at the start of the next frame
	 * after:
	 *  1) Clearing this event
	 *  2) Disabling the LSC module
	 *  3) Enabling it
	 */
	isp_reg_clr(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_LSC_CONFIG,
		    ISPCCDC_LSC_ENABLE);
	ccdc->lsc.state = LSC_STATE_STOPPED;
}

static void ispccdc_lsc_free_request(struct isp_ccdc_device *ccdc,
				     struct ispccdc_lsc_config_req *req)
{
	struct isp_device *isp = to_isp_device(ccdc);

	if (req == NULL)
		return;

	if (req->iovm)
		dma_unmap_sg(isp->dev, req->iovm->sgt->sgl,
			     req->iovm->sgt->nents, DMA_TO_DEVICE);
	if (req->table)
		iommu_vfree(isp->iommu, req->table);
	kfree(req);
}

static void ispccdc_lsc_free_queue(struct isp_ccdc_device *ccdc,
				   struct list_head *queue)
{
	struct ispccdc_lsc_config_req *req, *n;
	unsigned long flags;

	spin_lock_irqsave(&ccdc->lsc.req_lock, flags);
	list_for_each_entry_safe(req, n, queue, list) {
		list_del(&req->list);
		spin_unlock_irqrestore(&ccdc->lsc.req_lock, flags);
		ispccdc_lsc_free_request(ccdc, req);
		spin_lock_irqsave(&ccdc->lsc.req_lock, flags);
	}
	spin_unlock_irqrestore(&ccdc->lsc.req_lock, flags);
}

static void ispccdc_lsc_free_table_work(struct work_struct *work)
{
	struct isp_ccdc_device *ccdc;
	struct ispccdc_lsc *lsc;

	lsc = container_of(work, struct ispccdc_lsc, table_work);
	ccdc = container_of(lsc, struct isp_ccdc_device, lsc);

	ispccdc_lsc_free_queue(ccdc, &lsc->free_queue);
}

/*
 * ispccdc_lsc_config - Configure the LSC module from a userspace request
 *
 * Store the request LSC configuration in the LSC engine request pointer. The
 * configuration will be applied to the hardware when the CCDC will be enabled,
 * or at the next LSC interrupt if the CCDC is already running.
 */
static int ispccdc_lsc_config(struct isp_ccdc_device *ccdc,
			      struct omap3isp_ccdc_update_config *config)
{
	struct isp_device *isp = to_isp_device(ccdc);
	struct ispccdc_lsc_config_req *req;
	unsigned long flags;
	void *table;
	u16 update;
	int ret;

	update = config->update &
		 (OMAP3ISP_CCDC_CONFIG_LSC | OMAP3ISP_CCDC_TBL_LSC);
	if (!update)
		return 0;

	if (update != (OMAP3ISP_CCDC_CONFIG_LSC | OMAP3ISP_CCDC_TBL_LSC)) {
		dev_dbg(to_device(ccdc), "%s: Both LSC configuration and table "
			"need to be supplied\n", __func__);
		return -EINVAL;
	}

	req = kzalloc(sizeof(*req), GFP_KERNEL);
	if (req == NULL)
		return -ENOMEM;

	if (config->flag & OMAP3ISP_CCDC_CONFIG_LSC) {
		if (copy_from_user(&req->config, config->lsc_cfg,
				   sizeof(req->config))) {
			ret = -EFAULT;
			goto done;
		}

		req->enable = 1;

		req->table = iommu_vmalloc(isp->iommu, 0, req->config.size,
					   IOMMU_FLAG);
		if (IS_ERR_VALUE(req->table)) {
			req->table = 0;
			ret = -ENOMEM;
			goto done;
		}

		req->iovm = find_iovm_area(isp->iommu, req->table);
		if (req->iovm == NULL) {
			ret = -ENOMEM;
			goto done;
		}

		if (!dma_map_sg(isp->dev, req->iovm->sgt->sgl,
				req->iovm->sgt->nents, DMA_TO_DEVICE)) {
			ret = -ENOMEM;
			req->iovm = NULL;
			goto done;
		}

		dma_sync_sg_for_cpu(isp->dev, req->iovm->sgt->sgl,
				    req->iovm->sgt->nents, DMA_TO_DEVICE);

		table = da_to_va(isp->iommu, req->table);
		if (copy_from_user(table, config->lsc, req->config.size)) {
			ret = -EFAULT;
			goto done;
		}

		dma_sync_sg_for_device(isp->dev, req->iovm->sgt->sgl,
				       req->iovm->sgt->nents, DMA_TO_DEVICE);
	}

	spin_lock_irqsave(&ccdc->lsc.req_lock, flags);
	if (ccdc->lsc.request) {
		list_add_tail(&ccdc->lsc.request->list, &ccdc->lsc.free_queue);
		schedule_work(&ccdc->lsc.table_work);
	}
	ccdc->lsc.request = req;
	spin_unlock_irqrestore(&ccdc->lsc.req_lock, flags);

	ret = 0;

done:
	if (ret < 0)
		ispccdc_lsc_free_request(ccdc, req);

	return ret;
}

static inline int ispccdc_lsc_is_configured(struct isp_ccdc_device *ccdc)
{
	unsigned long flags;

	spin_lock_irqsave(&ccdc->lsc.req_lock, flags);
	if (ccdc->lsc.active) {
		spin_unlock_irqrestore(&ccdc->lsc.req_lock, flags);
		return 1;
	}
	spin_unlock_irqrestore(&ccdc->lsc.req_lock, flags);
	return 0;
}

static int ispccdc_lsc_enable(struct isp_ccdc_device *ccdc)
{
	struct ispccdc_lsc *lsc = &ccdc->lsc;

	if (lsc->state != LSC_STATE_STOPPED)
		return -EINVAL;

	if (lsc->active) {
		list_add_tail(&lsc->active->list, &lsc->free_queue);
		lsc->active = NULL;
	}

	if (__ispccdc_lsc_configure(ccdc, lsc->request) < 0) {
		isp_sbl_disable(to_isp_device(ccdc),
				OMAP3_ISP_SBL_CCDC_LSC_READ);
		list_add_tail(&lsc->request->list, &lsc->free_queue);
		lsc->request = NULL;
		goto done;
	}

	lsc->active = lsc->request;
	lsc->request = NULL;
	__ispccdc_lsc_enable(ccdc, 1);

done:
	if (!list_empty(&lsc->free_queue))
		schedule_work(&lsc->table_work);

	return 0;
}

/* -----------------------------------------------------------------------------
 * Parameters configuration
 */

/*
 * ispccdc_configure_clamp - Configure optical-black or digital clamping
 * @ccdc: Pointer to ISP CCDC device.
 *
 * The CCDC performs either optical-black or digital clamp. Configure and enable
 * the selected clamp method.
 */
static void ispccdc_configure_clamp(struct isp_ccdc_device *ccdc)
{
	struct isp_device *isp = to_isp_device(ccdc);
	u32 clamp;

	if (ccdc->obclamp) {
		clamp  = ccdc->clamp.obgain << ISPCCDC_CLAMP_OBGAIN_SHIFT;
		clamp |= ccdc->clamp.oblen << ISPCCDC_CLAMP_OBSLEN_SHIFT;
		clamp |= ccdc->clamp.oblines << ISPCCDC_CLAMP_OBSLN_SHIFT;
		clamp |= ccdc->clamp.obstpixel << ISPCCDC_CLAMP_OBST_SHIFT;
		isp_reg_writel(isp, clamp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_CLAMP);
	} else {
		isp_reg_writel(isp, ccdc->clamp.dcsubval,
			       OMAP3_ISP_IOMEM_CCDC, ISPCCDC_DCSUB);
	}

	isp_reg_clr_set(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_CLAMP,
			ISPCCDC_CLAMP_CLAMPEN,
			ccdc->obclamp ? ISPCCDC_CLAMP_CLAMPEN : 0);
}

/*
 * ispccdc_configure_fpc - Configure Faulty Pixel Correction
 * @ccdc: Pointer to ISP CCDC device.
 */
static void ispccdc_configure_fpc(struct isp_ccdc_device *ccdc)
{
	struct isp_device *isp = to_isp_device(ccdc);

	isp_reg_clr(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_FPC, ISPCCDC_FPC_FPCEN);

	if (!ccdc->fpc_en)
		return;

	isp_reg_writel(isp, ccdc->fpc.fpcaddr, OMAP3_ISP_IOMEM_CCDC,
		       ISPCCDC_FPC_ADDR);
	/* The FPNUM field must be set before enabling FPC. */
	isp_reg_writel(isp, (ccdc->fpc.fpnum << ISPCCDC_FPC_FPNUM_SHIFT),
		       OMAP3_ISP_IOMEM_CCDC, ISPCCDC_FPC);
	isp_reg_writel(isp, (ccdc->fpc.fpnum << ISPCCDC_FPC_FPNUM_SHIFT) |
		       ISPCCDC_FPC_FPCEN, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_FPC);
}

/*
 * ispccdc_configure_black_comp - Configure Black Level Compensation.
 * @ccdc: Pointer to ISP CCDC device.
 */
static void ispccdc_configure_black_comp(struct isp_ccdc_device *ccdc)
{
	struct isp_device *isp = to_isp_device(ccdc);
	u32 blcomp;

	blcomp  = ccdc->blcomp.b_mg << ISPCCDC_BLKCMP_B_MG_SHIFT;
	blcomp |= ccdc->blcomp.gb_g << ISPCCDC_BLKCMP_GB_G_SHIFT;
	blcomp |= ccdc->blcomp.gr_cy << ISPCCDC_BLKCMP_GR_CY_SHIFT;
	blcomp |= ccdc->blcomp.r_ye << ISPCCDC_BLKCMP_R_YE_SHIFT;

	isp_reg_writel(isp, blcomp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_BLKCMP);
}

/*
 * ispccdc_configure_lpf - Configure Low-Pass Filter (LPF).
 * @ccdc: Pointer to ISP CCDC device.
 */
static void ispccdc_configure_lpf(struct isp_ccdc_device *ccdc)
{
	struct isp_device *isp = to_isp_device(ccdc);

	isp_reg_clr_set(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_SYN_MODE,
			ISPCCDC_SYN_MODE_LPF,
			ccdc->lpf ? ISPCCDC_SYN_MODE_LPF : 0);
}

/*
 * ispccdc_configure_alaw - Configure A-law compression.
 * @ccdc: Pointer to ISP CCDC device.
 */
static void ispccdc_configure_alaw(struct isp_ccdc_device *ccdc)
{
	struct isp_device *isp = to_isp_device(ccdc);
	u32 alaw = 0;

	switch (ccdc->syncif.datsz) {
	case 8:
		return;

	case 10:
		alaw = ISPCCDC_ALAW_GWDI_9_0;
		break;
	case 11:
		alaw = ISPCCDC_ALAW_GWDI_10_1;
		break;
	case 12:
		alaw = ISPCCDC_ALAW_GWDI_11_2;
		break;
	case 13:
		alaw = ISPCCDC_ALAW_GWDI_12_3;
		break;
	}

	if (ccdc->alaw)
		alaw |= ISPCCDC_ALAW_CCDTBL;

	isp_reg_writel(isp, alaw, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_ALAW);
}

/*
 * ispccdc_config_imgattr - Configure sensor image specific attributes.
 * @ccdc: Pointer to ISP CCDC device.
 * @colptn: Color pattern of the sensor.
 */
static void ispccdc_config_imgattr(struct isp_ccdc_device *ccdc, u32 colptn)
{
	struct isp_device *isp = to_isp_device(ccdc);

	isp_reg_writel(isp, colptn, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_COLPTN);
}

/*
 * ispccdc_config - Set CCDC configuration from userspace
 * @ccdc: Pointer to ISP CCDC device.
 * @userspace_add: Structure containing CCDC configuration sent from userspace.
 *
 * Returns 0 if successful, -EINVAL if the pointer to the configuration
 * structure is null, or the copy_from_user function fails to copy user space
 * memory to kernel space memory.
 */
static int ispccdc_config(struct isp_ccdc_device *ccdc,
			  struct omap3isp_ccdc_update_config *ccdc_struct)
{
	struct isp_device *isp = to_isp_device(ccdc);
	unsigned long flags;

	spin_lock_irqsave(&ccdc->lock, flags);
	ccdc->shadow_update = 1;
	spin_unlock_irqrestore(&ccdc->lock, flags);

	if (OMAP3ISP_CCDC_ALAW & ccdc_struct->update) {
		ccdc->alaw = !!(OMAP3ISP_CCDC_ALAW & ccdc_struct->flag);
		ccdc->update |= OMAP3ISP_CCDC_ALAW;
	}

	if (OMAP3ISP_CCDC_LPF & ccdc_struct->update) {
		ccdc->lpf = !!(OMAP3ISP_CCDC_LPF & ccdc_struct->flag);
		ccdc->update |= OMAP3ISP_CCDC_LPF;
	}

	if (OMAP3ISP_CCDC_BLCLAMP & ccdc_struct->update) {
		if (copy_from_user(&ccdc->clamp, ccdc_struct->bclamp,
				   sizeof(ccdc->clamp))) {
			ccdc->shadow_update = 0;
			return -EFAULT;
		}

		ccdc->obclamp = !!(OMAP3ISP_CCDC_BLCLAMP & ccdc_struct->flag);
		ccdc->update |= OMAP3ISP_CCDC_BLCLAMP;
	}

	if (OMAP3ISP_CCDC_BCOMP & ccdc_struct->update) {
		if (copy_from_user(&ccdc->blcomp, ccdc_struct->blcomp,
				   sizeof(ccdc->blcomp))) {
			ccdc->shadow_update = 0;
			return -EFAULT;
		}

		ccdc->update |= OMAP3ISP_CCDC_BCOMP;
	}

	ccdc->shadow_update = 0;

	if (OMAP3ISP_CCDC_FPC & ccdc_struct->update) {
		u32 table_old = 0;
		u32 table_new;
		u32 size;

		if (ccdc->state != ISP_PIPELINE_STREAM_STOPPED)
			return -EBUSY;

		ccdc->fpc_en = !!(OMAP3ISP_CCDC_FPC & ccdc_struct->flag);

		if (ccdc->fpc_en) {
			if (copy_from_user(&ccdc->fpc, ccdc_struct->fpc,
					   sizeof(ccdc->fpc)))
				return -EFAULT;

			/*
			 * table_new must be 64-bytes aligned, but it's
			 * already done by iommu_vmalloc().
			 */
			size = ccdc->fpc.fpnum * 4;
			table_new = iommu_vmalloc(isp->iommu, 0, size,
						  IOMMU_FLAG);
			if (IS_ERR_VALUE(table_new))
				return -ENOMEM;

			if (copy_from_user(da_to_va(isp->iommu, table_new),
					   (__force void __user *)
					   ccdc->fpc.fpcaddr, size)) {
				iommu_vfree(isp->iommu, table_new);
				return -EFAULT;
			}

			table_old = ccdc->fpc.fpcaddr;
			ccdc->fpc.fpcaddr = table_new;
		}

		ispccdc_configure_fpc(ccdc);
		if (table_old != 0)
			iommu_vfree(isp->iommu, table_old);
	}

	return ispccdc_lsc_config(ccdc, ccdc_struct);
}

static void ccdc_apply_controls(struct isp_ccdc_device *ccdc)
{
	if (ccdc->update & OMAP3ISP_CCDC_ALAW) {
		ispccdc_configure_alaw(ccdc);
		ccdc->update &= ~OMAP3ISP_CCDC_ALAW;
	}

	if (ccdc->update & OMAP3ISP_CCDC_LPF) {
		ispccdc_configure_lpf(ccdc);
		ccdc->update &= ~OMAP3ISP_CCDC_LPF;
	}

	if (ccdc->update & OMAP3ISP_CCDC_BLCLAMP) {
		ispccdc_configure_clamp(ccdc);
		ccdc->update &= ~OMAP3ISP_CCDC_BLCLAMP;
	}

	if (ccdc->update & OMAP3ISP_CCDC_BCOMP) {
		ispccdc_configure_black_comp(ccdc);
		ccdc->update &= ~OMAP3ISP_CCDC_BCOMP;
	}
}

/*
 * ispccdc_restore_context - Restore values of the CCDC module registers
 * @dev: Pointer to ISP device
 */
void ispccdc_restore_context(struct isp_device *isp)
{
	struct isp_ccdc_device *ccdc = &isp->isp_ccdc;
	struct v4l2_mbus_framefmt *format;

	isp_reg_set(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_CFG, ISPCCDC_CFG_VDLC);

	/* CCDC_PAD_SINK */
	format = &ccdc->formats[CCDC_PAD_SINK];

	if ((format->code != V4L2_MBUS_FMT_UYVY8_2X8) &&
			(format->code != V4L2_MBUS_FMT_UYVY8_2X8))
		ccdc->update = OMAP3ISP_CCDC_ALAW | OMAP3ISP_CCDC_LPF
			     | OMAP3ISP_CCDC_BLCLAMP | OMAP3ISP_CCDC_BCOMP;

	ccdc_apply_controls(ccdc);
	ispccdc_configure_fpc(ccdc);
}

/* -----------------------------------------------------------------------------
 * Format- and pipeline-related configuration helpers
 */

/*
 * ispccdc_config_vp - Configure the Video Port.
 * @ccdc: Pointer to ISP CCDC device.
 */
static void ispccdc_config_vp(struct isp_ccdc_device *ccdc)
{
	struct isp_pipeline *pipe = to_isp_pipeline(&ccdc->subdev.entity);
	struct isp_device *isp = to_isp_device(ccdc);
	unsigned long l3_ick = pipe->l3_ick;
	unsigned int max_div = isp->revision == ISP_REVISION_15_0 ? 64 : 8;
	unsigned int div = 0;
	u32 fmtcfg_vp;

	fmtcfg_vp = isp_reg_readl(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_FMTCFG)
		  & ~(ISPCCDC_FMTCFG_VPIN_MASK | ISPCCDC_FMTCFG_VPIF_FRQ_MASK);

	switch (ccdc->syncif.datsz) {
	case 8:
	case 10:
		fmtcfg_vp |= ISPCCDC_FMTCFG_VPIN_9_0;
		break;
	case 11:
		fmtcfg_vp |= ISPCCDC_FMTCFG_VPIN_10_1;
		break;
	case 12:
		fmtcfg_vp |= ISPCCDC_FMTCFG_VPIN_11_2;
		break;
	case 13:
		fmtcfg_vp |= ISPCCDC_FMTCFG_VPIN_12_3;
		break;
	};

	if (pipe->input)
		div = DIV_ROUND_UP(l3_ick, pipe->max_rate);
	else if (ccdc->vpcfg.pixelclk)
		div = l3_ick / ccdc->vpcfg.pixelclk;

	div = clamp(div, 2U, max_div);
	fmtcfg_vp |= (div - 2) << ISPCCDC_FMTCFG_VPIF_FRQ_SHIFT;

	isp_reg_writel(isp, fmtcfg_vp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_FMTCFG);
}

/*
 * ispccdc_enable_vp - Enable Video Port.
 * @ccdc: Pointer to ISP CCDC device.
 * @enable: 0 Disables VP, 1 Enables VP
 *
 * This is needed for outputting image to Preview, H3A and HIST ISP submodules.
 */
static void ispccdc_enable_vp(struct isp_ccdc_device *ccdc, u8 enable)
{
	struct isp_device *isp = to_isp_device(ccdc);

	isp_reg_clr_set(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_FMTCFG,
			ISPCCDC_FMTCFG_VPEN, enable ? ISPCCDC_FMTCFG_VPEN : 0);
}

/*
 * ispccdc_config_outlineoffset - Configure memory saving output line offset
 * @ccdc: Pointer to ISP CCDC device.
 * @offset: Address offset to start a new line. Must be twice the
 *          Output width and aligned on 32 byte boundary
 * @oddeven: Specifies the odd/even line pattern to be chosen to store the
 *           output.
 * @numlines: Set the value 0-3 for +1-4lines, 4-7 for -1-4lines.
 *
 * - Configures the output line offset when stored in memory
 * - Sets the odd/even line pattern to store the output
 *    (EVENEVEN (1), ODDEVEN (2), EVENODD (3), ODDODD (4))
 * - Configures the number of even and odd line fields in case of rearranging
 * the lines.
 */
static void ispccdc_config_outlineoffset(struct isp_ccdc_device *ccdc,
					u32 offset, u8 oddeven, u8 numlines)
{
	struct isp_device *isp = to_isp_device(ccdc);

	isp_reg_writel(isp, offset & 0xffff,
		       OMAP3_ISP_IOMEM_CCDC, ISPCCDC_HSIZE_OFF);

	isp_reg_clr(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_SDOFST,
		    ISPCCDC_SDOFST_FINV);

	isp_reg_clr(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_SDOFST,
		    ISPCCDC_SDOFST_FOFST_4L);

	switch (oddeven) {
	case EVENEVEN:
		isp_reg_set(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_SDOFST,
			    (numlines & 0x7) << ISPCCDC_SDOFST_LOFST0_SHIFT);
		break;
	case ODDEVEN:
		isp_reg_set(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_SDOFST,
			    (numlines & 0x7) << ISPCCDC_SDOFST_LOFST1_SHIFT);
		break;
	case EVENODD:
		isp_reg_set(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_SDOFST,
			    (numlines & 0x7) << ISPCCDC_SDOFST_LOFST2_SHIFT);
		break;
	case ODDODD:
		isp_reg_set(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_SDOFST,
			    (numlines & 0x7) << ISPCCDC_SDOFST_LOFST3_SHIFT);
		break;
	default:
		break;
	}
}

/*
 * ispccdc_set_outaddr - Set memory address to save output image
 * @ccdc: Pointer to ISP CCDC device.
 * @addr: ISP MMU Mapped 32-bit memory address aligned on 32 byte boundary.
 *
 * Sets the memory address where the output will be saved.
 */
static void ispccdc_set_outaddr(struct isp_ccdc_device *ccdc, u32 addr)
{
	struct isp_device *isp = to_isp_device(ccdc);

	isp_reg_writel(isp, addr, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_SDR_ADDR);
}

/*
 * ispccdc_max_rate - Calculate maximum input data rate depending on the input
 * @ccdc: Pointer to ISP CCDC device.
 * @max_rate: Maximum calculated data rate.
 *
 * Returns in *max_rate less value between calculated and passed
 */
void ispccdc_max_rate(struct isp_ccdc_device *ccdc, unsigned int *max_rate)
{
	struct isp_pipeline *pipe = to_isp_pipeline(&ccdc->subdev.entity);
	unsigned int rate;

	if (pipe == NULL)
		return;

	/*
	 * TRM says that for parallel sensors the maximum data rate
	 * should be 90% form L3/2 clock, otherwise just L3/2.
	 */
	if (ccdc->input == CCDC_INPUT_PARALLEL)
		rate = pipe->l3_ick / 2 * 9 / 10;
	else
		rate = pipe->l3_ick / 2;

	*max_rate = min(*max_rate, rate);
}

/*
 * ispccdc_config_sync_if - Set CCDC sync interface configuration
 * @ccdc: Pointer to ISP CCDC device.
 * @syncif: Structure containing the sync parameters like field state, CCDC in
 *          master/slave mode, raw/yuv data, polarity of data, field, hs, vs
 *          signals.
 */
static void ispccdc_config_sync_if(struct isp_ccdc_device *ccdc,
				   struct ispccdc_syncif *syncif)
{
	struct isp_device *isp = to_isp_device(ccdc);
	u32 syn_mode = isp_reg_readl(isp, OMAP3_ISP_IOMEM_CCDC,
				     ISPCCDC_SYN_MODE);

	syn_mode &= ~(ISPCCDC_SYN_MODE_VDHDOUT |
			ISPCCDC_SYN_MODE_FLDOUT |
			ISPCCDC_SYN_MODE_VDPOL |
			ISPCCDC_SYN_MODE_HDPOL |
			ISPCCDC_SYN_MODE_FLDPOL |
			ISPCCDC_SYN_MODE_FLDMODE |
			ISPCCDC_SYN_MODE_DATAPOL |
			ISPCCDC_SYN_MODE_DATSIZ_MASK |
			ISPCCDC_SYN_MODE_PACK8 |
			ISPCCDC_SYN_MODE_INPMOD_MASK);

	syn_mode |= ISPCCDC_SYN_MODE_VDHDEN;

	if (syncif->fldstat)
		syn_mode |= ISPCCDC_SYN_MODE_FLDSTAT;

	switch (syncif->datsz) {
	case 8:
		syn_mode |= ISPCCDC_SYN_MODE_DATSIZ_8;
		break;
	case 10:
		syn_mode |= ISPCCDC_SYN_MODE_DATSIZ_10;
		break;
	case 11:
		syn_mode |= ISPCCDC_SYN_MODE_DATSIZ_11;
		break;
	case 12:
		syn_mode |= ISPCCDC_SYN_MODE_DATSIZ_12;
		break;
	};

	if (syncif->fldmode)
		syn_mode |= ISPCCDC_SYN_MODE_FLDMODE;

	if (syncif->datapol)
		syn_mode |= ISPCCDC_SYN_MODE_DATAPOL;

	if (syncif->fldpol)
		syn_mode |= ISPCCDC_SYN_MODE_FLDPOL;

	if (syncif->hdpol)
		syn_mode |= ISPCCDC_SYN_MODE_HDPOL;

	if (syncif->vdpol)
		syn_mode |= ISPCCDC_SYN_MODE_VDPOL;

	if (syncif->bt_r656_en)
		syn_mode |= ISPCCDC_SYN_MODE_PACK8;

	if (syncif->ccdc_mastermode) {
		syn_mode |= ISPCCDC_SYN_MODE_FLDOUT | ISPCCDC_SYN_MODE_VDHDOUT;
		isp_reg_writel(isp,
			       syncif->hs_width << ISPCCDC_HD_VD_WID_HDW_SHIFT
			     | syncif->vs_width << ISPCCDC_HD_VD_WID_VDW_SHIFT,
			       OMAP3_ISP_IOMEM_CCDC,
			       ISPCCDC_HD_VD_WID);

		isp_reg_writel(isp,
			       syncif->ppln << ISPCCDC_PIX_LINES_PPLN_SHIFT
			     | syncif->hlprf << ISPCCDC_PIX_LINES_HLPRF_SHIFT,
			       OMAP3_ISP_IOMEM_CCDC,
			       ISPCCDC_PIX_LINES);
	}

	isp_reg_writel(isp, syn_mode, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_SYN_MODE);

	if (syncif->bt_r656_en)
		isp_reg_set(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_REC656IF,
			    ISPCCDC_REC656IF_R656ON | ISPCCDC_REC656IF_ECCFVH);
	else
		isp_reg_clr(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_REC656IF,
			    ISPCCDC_REC656IF_R656ON | ISPCCDC_REC656IF_ECCFVH);
}

/* CCDC formats descriptions */
static const u32 ccdc_sgrbg_pattern =
	ISPCCDC_COLPTN_Gr_Cy << ISPCCDC_COLPTN_CP0PLC0_SHIFT |
	ISPCCDC_COLPTN_R_Ye  << ISPCCDC_COLPTN_CP0PLC1_SHIFT |
	ISPCCDC_COLPTN_Gr_Cy << ISPCCDC_COLPTN_CP0PLC2_SHIFT |
	ISPCCDC_COLPTN_R_Ye  << ISPCCDC_COLPTN_CP0PLC3_SHIFT |
	ISPCCDC_COLPTN_B_Mg  << ISPCCDC_COLPTN_CP1PLC0_SHIFT |
	ISPCCDC_COLPTN_Gb_G  << ISPCCDC_COLPTN_CP1PLC1_SHIFT |
	ISPCCDC_COLPTN_B_Mg  << ISPCCDC_COLPTN_CP1PLC2_SHIFT |
	ISPCCDC_COLPTN_Gb_G  << ISPCCDC_COLPTN_CP1PLC3_SHIFT |
	ISPCCDC_COLPTN_Gr_Cy << ISPCCDC_COLPTN_CP2PLC0_SHIFT |
	ISPCCDC_COLPTN_R_Ye  << ISPCCDC_COLPTN_CP2PLC1_SHIFT |
	ISPCCDC_COLPTN_Gr_Cy << ISPCCDC_COLPTN_CP2PLC2_SHIFT |
	ISPCCDC_COLPTN_R_Ye  << ISPCCDC_COLPTN_CP2PLC3_SHIFT |
	ISPCCDC_COLPTN_B_Mg  << ISPCCDC_COLPTN_CP3PLC0_SHIFT |
	ISPCCDC_COLPTN_Gb_G  << ISPCCDC_COLPTN_CP3PLC1_SHIFT |
	ISPCCDC_COLPTN_B_Mg  << ISPCCDC_COLPTN_CP3PLC2_SHIFT |
	ISPCCDC_COLPTN_Gb_G  << ISPCCDC_COLPTN_CP3PLC3_SHIFT;

static const u32 ccdc_srggb_pattern =
	ISPCCDC_COLPTN_R_Ye  << ISPCCDC_COLPTN_CP0PLC0_SHIFT |
	ISPCCDC_COLPTN_Gr_Cy << ISPCCDC_COLPTN_CP0PLC1_SHIFT |
	ISPCCDC_COLPTN_R_Ye  << ISPCCDC_COLPTN_CP0PLC2_SHIFT |
	ISPCCDC_COLPTN_Gr_Cy << ISPCCDC_COLPTN_CP0PLC3_SHIFT |
	ISPCCDC_COLPTN_Gb_G  << ISPCCDC_COLPTN_CP1PLC0_SHIFT |
	ISPCCDC_COLPTN_B_Mg  << ISPCCDC_COLPTN_CP1PLC1_SHIFT |
	ISPCCDC_COLPTN_Gb_G  << ISPCCDC_COLPTN_CP1PLC2_SHIFT |
	ISPCCDC_COLPTN_B_Mg  << ISPCCDC_COLPTN_CP1PLC3_SHIFT |
	ISPCCDC_COLPTN_R_Ye  << ISPCCDC_COLPTN_CP2PLC0_SHIFT |
	ISPCCDC_COLPTN_Gr_Cy << ISPCCDC_COLPTN_CP2PLC1_SHIFT |
	ISPCCDC_COLPTN_R_Ye  << ISPCCDC_COLPTN_CP2PLC2_SHIFT |
	ISPCCDC_COLPTN_Gr_Cy << ISPCCDC_COLPTN_CP2PLC3_SHIFT |
	ISPCCDC_COLPTN_Gb_G  << ISPCCDC_COLPTN_CP3PLC0_SHIFT |
	ISPCCDC_COLPTN_B_Mg  << ISPCCDC_COLPTN_CP3PLC1_SHIFT |
	ISPCCDC_COLPTN_Gb_G  << ISPCCDC_COLPTN_CP3PLC2_SHIFT |
	ISPCCDC_COLPTN_B_Mg  << ISPCCDC_COLPTN_CP3PLC3_SHIFT;

static const u32 ccdc_sbggr_pattern =
	ISPCCDC_COLPTN_B_Mg  << ISPCCDC_COLPTN_CP0PLC0_SHIFT |
	ISPCCDC_COLPTN_Gb_G  << ISPCCDC_COLPTN_CP0PLC1_SHIFT |
	ISPCCDC_COLPTN_B_Mg  << ISPCCDC_COLPTN_CP0PLC2_SHIFT |
	ISPCCDC_COLPTN_Gb_G  << ISPCCDC_COLPTN_CP0PLC3_SHIFT |
	ISPCCDC_COLPTN_Gr_Cy << ISPCCDC_COLPTN_CP1PLC0_SHIFT |
	ISPCCDC_COLPTN_R_Ye  << ISPCCDC_COLPTN_CP1PLC1_SHIFT |
	ISPCCDC_COLPTN_Gr_Cy << ISPCCDC_COLPTN_CP1PLC2_SHIFT |
	ISPCCDC_COLPTN_R_Ye  << ISPCCDC_COLPTN_CP1PLC3_SHIFT |
	ISPCCDC_COLPTN_B_Mg  << ISPCCDC_COLPTN_CP2PLC0_SHIFT |
	ISPCCDC_COLPTN_Gb_G  << ISPCCDC_COLPTN_CP2PLC1_SHIFT |
	ISPCCDC_COLPTN_B_Mg  << ISPCCDC_COLPTN_CP2PLC2_SHIFT |
	ISPCCDC_COLPTN_Gb_G  << ISPCCDC_COLPTN_CP2PLC3_SHIFT |
	ISPCCDC_COLPTN_Gr_Cy << ISPCCDC_COLPTN_CP3PLC0_SHIFT |
	ISPCCDC_COLPTN_R_Ye  << ISPCCDC_COLPTN_CP3PLC1_SHIFT |
	ISPCCDC_COLPTN_Gr_Cy << ISPCCDC_COLPTN_CP3PLC2_SHIFT |
	ISPCCDC_COLPTN_R_Ye  << ISPCCDC_COLPTN_CP3PLC3_SHIFT;

static const u32 ccdc_sgbrg_pattern =
	ISPCCDC_COLPTN_Gb_G  << ISPCCDC_COLPTN_CP0PLC0_SHIFT |
	ISPCCDC_COLPTN_B_Mg  << ISPCCDC_COLPTN_CP0PLC1_SHIFT |
	ISPCCDC_COLPTN_Gb_G  << ISPCCDC_COLPTN_CP0PLC2_SHIFT |
	ISPCCDC_COLPTN_B_Mg  << ISPCCDC_COLPTN_CP0PLC3_SHIFT |
	ISPCCDC_COLPTN_R_Ye  << ISPCCDC_COLPTN_CP1PLC0_SHIFT |
	ISPCCDC_COLPTN_Gr_Cy << ISPCCDC_COLPTN_CP1PLC1_SHIFT |
	ISPCCDC_COLPTN_R_Ye  << ISPCCDC_COLPTN_CP1PLC2_SHIFT |
	ISPCCDC_COLPTN_Gr_Cy << ISPCCDC_COLPTN_CP1PLC3_SHIFT |
	ISPCCDC_COLPTN_Gb_G  << ISPCCDC_COLPTN_CP2PLC0_SHIFT |
	ISPCCDC_COLPTN_B_Mg  << ISPCCDC_COLPTN_CP2PLC1_SHIFT |
	ISPCCDC_COLPTN_Gb_G  << ISPCCDC_COLPTN_CP2PLC2_SHIFT |
	ISPCCDC_COLPTN_B_Mg  << ISPCCDC_COLPTN_CP2PLC3_SHIFT |
	ISPCCDC_COLPTN_R_Ye  << ISPCCDC_COLPTN_CP3PLC0_SHIFT |
	ISPCCDC_COLPTN_Gr_Cy << ISPCCDC_COLPTN_CP3PLC1_SHIFT |
	ISPCCDC_COLPTN_R_Ye  << ISPCCDC_COLPTN_CP3PLC2_SHIFT |
	ISPCCDC_COLPTN_Gr_Cy << ISPCCDC_COLPTN_CP3PLC3_SHIFT;

static void ccdc_configure(struct isp_ccdc_device *ccdc)
{
	struct isp_device *isp = to_isp_device(ccdc);
	struct isp_parallel_platform_data *pdata = NULL;
	struct v4l2_subdev *sensor;
	struct v4l2_mbus_framefmt *format;
	struct v4l2_pix_format pix;
	struct media_pad *pad;
	unsigned long flags;
	u32 syn_mode;
	u32 ccdc_pattern;

	if (ccdc->input == CCDC_INPUT_PARALLEL) {
		pad = media_entity_remote_source(&ccdc->pads[CCDC_PAD_SINK]);
		sensor = media_entity_to_v4l2_subdev(pad->entity);
		pdata = &((struct isp_v4l2_subdevs_group *)sensor->host_priv)
			->bus.parallel;
	}

	isp_configure_bridge(isp, ccdc->input, pdata);

	if (pdata) {
		ccdc->syncif.datsz = pdata->width;
		ccdc->syncif.fldmode = pdata->fldmode;
		ccdc->syncif.hdpol = pdata->hdpol;
		ccdc->syncif.vdpol = pdata->vdpol;
		ccdc->syncif.bt_r656_en = pdata->is_bt656;
	} else {
		ccdc->syncif.datsz = 10;
	}

	ispccdc_config_sync_if(ccdc, &ccdc->syncif);

	syn_mode = isp_reg_readl(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_SYN_MODE);

	/* Use the raw, unprocessed data when writing to memory. The H3A and
	 * histogram modules are still fed with lens shading corrected data.
	 */
	syn_mode &= ~ISPCCDC_SYN_MODE_VP2SDR;

	if (ccdc->output & CCDC_OUTPUT_MEMORY)
		syn_mode |= ISPCCDC_SYN_MODE_WEN;
	else
		syn_mode &= ~ISPCCDC_SYN_MODE_WEN;

	if (ccdc->output & CCDC_OUTPUT_RESIZER)
		syn_mode |= ISPCCDC_SYN_MODE_SDR2RSZ;
	else
		syn_mode &= ~ISPCCDC_SYN_MODE_SDR2RSZ;

	/* CCDC_PAD_SINK */
	format = &ccdc->formats[CCDC_PAD_SINK];

	if ((format->code == V4L2_MBUS_FMT_YUYV8_2X8) ||
			(format->code == V4L2_MBUS_FMT_UYVY8_2X8)) {
		if (pdata->is_bt656)
			syn_mode |= ISPCCDC_SYN_MODE_INPMOD_YCBCR8;
		else
			syn_mode |= ISPCCDC_SYN_MODE_INPMOD_YCBCR16;
	}

	isp_reg_writel(isp, syn_mode, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_SYN_MODE);

	if (format->code == V4L2_MBUS_FMT_UYVY8_2X8)
		isp_reg_set(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_CFG,
			    ISPCCDC_CFG_Y8POS);
	/* Mosaic filter */
	switch (format->code) {
	case V4L2_MBUS_FMT_SRGGB10_1X10:
	case V4L2_MBUS_FMT_SRGGB12_1X12:
		ccdc_pattern = ccdc_srggb_pattern;
		break;
	case V4L2_MBUS_FMT_SBGGR10_1X10:
	case V4L2_MBUS_FMT_SBGGR12_1X12:
		ccdc_pattern = ccdc_sbggr_pattern;
		break;
	case V4L2_MBUS_FMT_SGBRG10_1X10:
	case V4L2_MBUS_FMT_SGBRG12_1X12:
		ccdc_pattern = ccdc_sgbrg_pattern;
		break;
	default:
		/* Use GRBG */
		ccdc_pattern = ccdc_sgrbg_pattern;
		break;
	}
	if ((format->code != V4L2_MBUS_FMT_YUYV8_2X8) &&
			(format->code != V4L2_MBUS_FMT_UYVY8_2X8))
	ispccdc_config_imgattr(ccdc, ccdc_pattern);

	/* BT656: Generate VD0 on the last line of each field, and we
	 * don't use VD1.
	 * Non BT656: Generate VD0 on the last line of the image and VD1 on the
	 * 2/3 height line.
	 */
	if (pdata->is_bt656)
		isp_reg_writel(isp,
			(format->height/2 - 2) << ISPCCDC_VDINT_0_SHIFT,
		       OMAP3_ISP_IOMEM_CCDC, ISPCCDC_VDINT);
	else
		isp_reg_writel(isp, ((format->height - 2) << ISPCCDC_VDINT_0_SHIFT) |
		       ((format->height * 2 / 3) << ISPCCDC_VDINT_1_SHIFT),
		       OMAP3_ISP_IOMEM_CCDC, ISPCCDC_VDINT);

	/* CCDC_PAD_SOURCE_OF */
	format = &ccdc->formats[CCDC_PAD_SOURCE_OF];

	isp_video_mbus_to_pix(&ccdc->video_out, format, &pix);
	/* For BT656 the number of bytes would be width*2 */
	if (pdata->is_bt656)
		isp_reg_writel(isp, (0 << ISPCCDC_HORZ_INFO_SPH_SHIFT) |
		       ((pix.bytesperline - 1) << ISPCCDC_HORZ_INFO_NPH_SHIFT),
		       OMAP3_ISP_IOMEM_CCDC, ISPCCDC_HORZ_INFO);
	else
		isp_reg_writel(isp, (0 << ISPCCDC_HORZ_INFO_SPH_SHIFT) |
		       ((format->width - 1) << ISPCCDC_HORZ_INFO_NPH_SHIFT),
		       OMAP3_ISP_IOMEM_CCDC, ISPCCDC_HORZ_INFO);

	isp_reg_writel(isp, 0 << ISPCCDC_VERT_START_SLV0_SHIFT,
		       OMAP3_ISP_IOMEM_CCDC, ISPCCDC_VERT_START);

	if (pdata->is_bt656)
		isp_reg_writel(isp, ((format->height >> 1) - 1)
				<< ISPCCDC_VERT_LINES_NLV_SHIFT,
			       OMAP3_ISP_IOMEM_CCDC, ISPCCDC_VERT_LINES);
	else
		isp_reg_writel(isp, (format->height - 1)
				<< ISPCCDC_VERT_LINES_NLV_SHIFT,
			       OMAP3_ISP_IOMEM_CCDC, ISPCCDC_VERT_LINES);

	/* In case of BT656 each alternate line must be stored into memory */
	isp_reg_clr(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_SDOFST,
		    ISPCCDC_SDOFST_LOFST_MASK << ISPCCDC_SDOFST_LOFST0_SHIFT);
	isp_reg_clr(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_SDOFST,
		    ISPCCDC_SDOFST_LOFST_MASK << ISPCCDC_SDOFST_LOFST1_SHIFT);
	isp_reg_clr(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_SDOFST,
		    ISPCCDC_SDOFST_LOFST_MASK << ISPCCDC_SDOFST_LOFST2_SHIFT);
	isp_reg_clr(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_SDOFST,
		    ISPCCDC_SDOFST_LOFST_MASK << ISPCCDC_SDOFST_LOFST3_SHIFT);
	if (pdata->is_bt656) {
		ispccdc_config_outlineoffset(ccdc, pix.bytesperline, EVENEVEN, 1);
		ispccdc_config_outlineoffset(ccdc, pix.bytesperline, EVENODD, 1);
		ispccdc_config_outlineoffset(ccdc, pix.bytesperline, ODDEVEN, 1);
		ispccdc_config_outlineoffset(ccdc, pix.bytesperline, ODDODD, 1);
	} else {
		ispccdc_config_outlineoffset(ccdc, pix.bytesperline, 0, 0);
	}
	/* CCDC_PAD_SOURCE_VP */
	format = &ccdc->formats[CCDC_PAD_SOURCE_VP];

	isp_reg_writel(isp, (0 << ISPCCDC_FMT_HORZ_FMTSPH_SHIFT) |
		       (format->width << ISPCCDC_FMT_HORZ_FMTLNH_SHIFT),
		       OMAP3_ISP_IOMEM_CCDC, ISPCCDC_FMT_HORZ);
	isp_reg_writel(isp, (0 << ISPCCDC_FMT_VERT_FMTSLV_SHIFT) |
		       ((format->height + 1) << ISPCCDC_FMT_VERT_FMTLNV_SHIFT),
		       OMAP3_ISP_IOMEM_CCDC, ISPCCDC_FMT_VERT);

	isp_reg_writel(isp, (format->width << ISPCCDC_VP_OUT_HORZ_NUM_SHIFT) |
		       (format->height << ISPCCDC_VP_OUT_VERT_NUM_SHIFT),
		       OMAP3_ISP_IOMEM_CCDC, ISPCCDC_VP_OUT);

	spin_lock_irqsave(&ccdc->lsc.req_lock, flags);
	if (ccdc->lsc.request == NULL)
		goto unlock;

	WARN_ON(ccdc->lsc.active);

	/* Get last good LSC configuration. If it is not supported for
	 * the current active resolution discard it.
	 */
	if (ccdc->lsc.active == NULL &&
	    __ispccdc_lsc_configure(ccdc, ccdc->lsc.request) == 0) {
		ccdc->lsc.active = ccdc->lsc.request;
	} else {
		list_add_tail(&ccdc->lsc.request->list, &ccdc->lsc.free_queue);
		schedule_work(&ccdc->lsc.table_work);
	}

	ccdc->lsc.request = NULL;

unlock:
	spin_unlock_irqrestore(&ccdc->lsc.req_lock, flags);

	if (pdata->is_bt656)
		ccdc->update = OMAP3ISP_CCDC_BLCLAMP;
	else
		ccdc->update = 0;

	ccdc_apply_controls(ccdc);
}

static void __ispccdc_enable(struct isp_ccdc_device *ccdc, int enable)
{
	struct isp_device *isp = to_isp_device(ccdc);

	isp_reg_clr_set(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_PCR,
			ISPCCDC_PCR_EN, enable ? ISPCCDC_PCR_EN : 0);
}

static int __ispccdc_handle_stopping(struct isp_ccdc_device *ccdc, u32 event);
static int ispccdc_disable(struct isp_ccdc_device *ccdc)
{
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&ccdc->lock, flags);
	if (ccdc->state == ISP_PIPELINE_STREAM_CONTINUOUS)
		ccdc->stopping = CCDC_STOP_REQUEST;
	spin_unlock_irqrestore(&ccdc->lock, flags);

	__ispccdc_lsc_enable(ccdc, 0);
	__ispccdc_enable(ccdc, 0);
	ccdc->stopping = CCDC_STOP_EXECUTED;
	__ispccdc_handle_stopping(ccdc, CCDC_STOP_FINISHED);

	ret = wait_event_timeout(ccdc->wait,
				 ccdc->stopping == CCDC_STOP_FINISHED,
				 msecs_to_jiffies(2000));
	if (ret == 0) {
		ret = -ETIMEDOUT;
		dev_warn(to_device(ccdc), "CCDC stop timeout!\n");
	}

	isp_sbl_disable(to_isp_device(ccdc), OMAP3_ISP_SBL_CCDC_LSC_READ);

	mutex_lock(&ccdc->ioctl_lock);
	ispccdc_lsc_free_request(ccdc, ccdc->lsc.request);
	ccdc->lsc.request = ccdc->lsc.active;
	ccdc->lsc.active = NULL;
	cancel_work_sync(&ccdc->lsc.table_work);
	ispccdc_lsc_free_queue(ccdc, &ccdc->lsc.free_queue);
	mutex_unlock(&ccdc->ioctl_lock);

	ccdc->stopping = CCDC_STOP_NOT_REQUESTED;

	return ret > 0 ? 0 : ret;
}

static void ispccdc_enable(struct isp_ccdc_device *ccdc)
{
	if (ispccdc_lsc_is_configured(ccdc))
		__ispccdc_lsc_enable(ccdc, 1);
	__ispccdc_enable(ccdc, 1);
}

/* -----------------------------------------------------------------------------
 * Interrupt handling
 */

/*
 * ispccdc_sbl_busy - Poll idle state of CCDC and related SBL memory write bits
 * @ccdc: Pointer to ISP CCDC device.
 *
 * Returns zero if the CCDC is idle and the image has been written to
 * memory, too.
 */
static int ispccdc_sbl_busy(struct isp_ccdc_device *ccdc)
{
	struct isp_device *isp = to_isp_device(ccdc);

	return ispccdc_busy(ccdc)
		| (isp_reg_readl(isp, OMAP3_ISP_IOMEM_SBL, ISPSBL_CCDC_WR_0) &
		   ISPSBL_CCDC_WR_0_DATA_READY)
		| (isp_reg_readl(isp, OMAP3_ISP_IOMEM_SBL, ISPSBL_CCDC_WR_1) &
		   ISPSBL_CCDC_WR_0_DATA_READY)
		| (isp_reg_readl(isp, OMAP3_ISP_IOMEM_SBL, ISPSBL_CCDC_WR_2) &
		   ISPSBL_CCDC_WR_0_DATA_READY)
		| (isp_reg_readl(isp, OMAP3_ISP_IOMEM_SBL, ISPSBL_CCDC_WR_3) &
		   ISPSBL_CCDC_WR_0_DATA_READY);
}

/*
 * ispccdc_sbl_wait_idle - Wait until the CCDC and related SBL are idle
 * @ccdc: Pointer to ISP CCDC device.
 * @max_wait: Max retry count in us for wait for idle/busy transition.
 */
static int ispccdc_sbl_wait_idle(struct isp_ccdc_device *ccdc,
				 unsigned int max_wait)
{
	unsigned int wait = 0;

	if (max_wait == 0)
		max_wait = 10000; /* 10 ms */

	for (wait = 0; wait <= max_wait; wait++) {
		if (!ispccdc_sbl_busy(ccdc))
			return 0;

		rmb();
		udelay(1);
	}

	return -EBUSY;
}

/* __ispccdc_handle_stopping - Handle CCDC and/or LSC stopping sequence
 * @ccdc: Pointer to ISP CCDC device.
 * @event: Pointing which event trigger handler
 *
 * Return 1 when the event and stopping request combination is satisfyied,
 * zero otherwise.
 */
static int __ispccdc_handle_stopping(struct isp_ccdc_device *ccdc, u32 event)
{
	int rval = 0;

	switch ((ccdc->stopping & 3) | event) {
	case CCDC_STOP_REQUEST | CCDC_EVENT_VD1:
		if (ccdc->lsc.state != LSC_STATE_STOPPED)
			__ispccdc_lsc_enable(ccdc, 0);
		__ispccdc_enable(ccdc, 0);
		ccdc->stopping = CCDC_STOP_EXECUTED;
		return 1;

	case CCDC_STOP_EXECUTED | CCDC_EVENT_VD0:
		ccdc->stopping |= CCDC_STOP_CCDC_FINISHED;
		if (ccdc->lsc.state == LSC_STATE_STOPPED)
			ccdc->stopping |= CCDC_STOP_LSC_FINISHED;
		rval = 1;
		break;

	case CCDC_STOP_EXECUTED | CCDC_EVENT_LSC_DONE:
		ccdc->stopping |= CCDC_STOP_LSC_FINISHED;
		rval = 1;
		break;

	case CCDC_STOP_EXECUTED | CCDC_EVENT_VD1:
		return 1;
	}

	if (ccdc->stopping == CCDC_STOP_FINISHED) {
		wake_up(&ccdc->wait);
		rval = 1;
	}

	return rval;
}

static void ispccdc_hs_vs_isr(struct isp_ccdc_device *ccdc)
{
	struct video_device *vdev = &ccdc->subdev.devnode;
	struct v4l2_event event;

	memset(&event, 0, sizeof(event));
	event.type = V4L2_EVENT_OMAP3ISP_HS_VS;

	v4l2_event_queue(vdev, &event);
}

/*
 * ispccdc_lsc_isr - Handle LSC events
 * @ccdc: Pointer to ISP CCDC device.
 * @events: LSC events
 */
static void ispccdc_lsc_isr(struct isp_ccdc_device *ccdc, u32 events)
{
	unsigned long flags;

	if (events & IRQ0STATUS_CCDC_LSC_PREF_ERR_IRQ) {
		ispccdc_lsc_error_handler(ccdc);
		ccdc->error = 1;
		dev_dbg(to_device(ccdc), "lsc prefetch error\n");
	}

	if (!(events & IRQ0STATUS_CCDC_LSC_DONE_IRQ))
		return;

	/* LSC_DONE interrupt occur, there are two cases
	 * 1. stopping for reconfiguration
	 * 2. stopping because of STREAM OFF command
	 */
	spin_lock_irqsave(&ccdc->lsc.req_lock, flags);

	if (ccdc->lsc.state == LSC_STATE_STOPPING)
		ccdc->lsc.state = LSC_STATE_STOPPED;

	if (__ispccdc_handle_stopping(ccdc, CCDC_EVENT_LSC_DONE))
		goto done;

	if (ccdc->lsc.state != LSC_STATE_RECONFIG)
		goto done;

	/* LSC is in STOPPING state, change to the new state */
	ccdc->lsc.state = LSC_STATE_STOPPED;

	/* This is an exception. Start of frame and LSC_DONE interrupt
	 * have been received on the same time. Skip this event and wait
	 * for better times.
	 */
	if (events & IRQ0STATUS_HS_VS_IRQ)
		goto done;

	/* The LSC engine is stopped at this point. Enable it if there's a
	 * pending request.
	 */
	if (ccdc->lsc.request == NULL)
		goto done;

	ispccdc_lsc_enable(ccdc);

done:
	spin_unlock_irqrestore(&ccdc->lsc.req_lock, flags);
}

static int ispccdc_isr_buffer(struct isp_ccdc_device *ccdc)
{
	struct isp_pipeline *pipe = to_isp_pipeline(&ccdc->subdev.entity);
	struct isp_device *isp = to_isp_device(ccdc);
	struct isp_buffer *buffer;
	int restart = 0;

	/* The CCDC generates VD0 interrupts even when disabled (the datasheet
	 * doesn't explicitly state if that's supposed to happen or not, so it
	 * can be considered as a hardware bug or as a feature, but we have to
	 * deal with it anyway). Disabling the CCDC when no buffer is available
	 * would thus not be enough, we need to handle the situation explicitly.
	 */
	if (list_empty(&ccdc->video_out.dmaqueue))
		goto done;

	/* We're in continuous mode, and memory writes were disabled due to a
	 * buffer underrun. Reenable them now that we have a buffer. The buffer
	 * address has been set in ccdc_video_queue.
	 */
	if (ccdc->state == ISP_PIPELINE_STREAM_CONTINUOUS && ccdc->underrun) {
		restart = 1;
		ccdc->underrun = 0;
		goto done;
	}

	if (ispccdc_sbl_wait_idle(ccdc, 1000)) {
		dev_info(isp->dev, "CCDC won't become idle!\n");
		goto done;
	}

	buffer = isp_video_buffer_next(&ccdc->video_out, ccdc->error);
	if (buffer != NULL) {
		ispccdc_set_outaddr(ccdc, buffer->isp_addr);
		restart = 1;
	}

	pipe->state |= ISP_PIPELINE_IDLE_OUTPUT;

	if (ccdc->state == ISP_PIPELINE_STREAM_SINGLESHOT &&
	    isp_pipeline_ready(pipe))
		isp_pipeline_set_stream(pipe,
					ISP_PIPELINE_STREAM_SINGLESHOT);

done:
	ccdc->error = 0;
	return restart;
}

/*
 * ispccdc_vd0_isr - Handle VD0 event
 * @ccdc: Pointer to ISP CCDC device.
 *
 * Executes LSC deferred enablement before next frame starts.
 */
static void ispccdc_vd0_isr(struct isp_ccdc_device *ccdc)
{
	unsigned long flags;
	int restart = 0;
	struct isp_device *isp = to_isp_device(ccdc);

	if (ccdc->output & CCDC_OUTPUT_MEMORY) {
		if (ccdc->syncif.bt_r656_en) {
			u32 fid;
			u32 syn_mode = isp_reg_readl(isp, OMAP3_ISP_IOMEM_CCDC,
					ISPCCDC_SYN_MODE);

			fid = syn_mode & ISPCCDC_SYN_MODE_FLDSTAT;
			/* toggle the software maintained fid */
			ccdc->syncif.fldstat ^= 1;
			if (fid == ccdc->syncif.fldstat) {
				if (fid == 0) {
					restart = ispccdc_isr_buffer(ccdc);
					goto done;
				}
			} else if (fid == 0) {
				ccdc->syncif.fldstat = fid;
			}
		} else {
			restart = ispccdc_isr_buffer(ccdc);
		}
	}
done:
	spin_lock_irqsave(&ccdc->lock, flags);
	if (__ispccdc_handle_stopping(ccdc, CCDC_EVENT_VD0)) {
		spin_unlock_irqrestore(&ccdc->lock, flags);
		return;
	}

	if (!ccdc->shadow_update)
		ccdc_apply_controls(ccdc);
	spin_unlock_irqrestore(&ccdc->lock, flags);

	if (restart)
		ispccdc_enable(ccdc);
}

/*
 * ispccdc_vd1_isr - Handle VD1 event
 * @ccdc: Pointer to ISP CCDC device.
 */
static void ispccdc_vd1_isr(struct isp_ccdc_device *ccdc)
{
	unsigned long flags;

	spin_lock_irqsave(&ccdc->lsc.req_lock, flags);

	/*
	 * Depending on the CCDC pipeline state, CCDC stopping should be
	 * handled differently. In SINGLESHOT we emulate an internal CCDC
	 * stopping because the CCDC hw works only in continuous mode.
	 * When CONTINUOUS pipeline state is used and the CCDC writes it's
	 * data to memory the CCDC and LSC are stopped immediately but
	 * without change the CCDC stopping state machine. The CCDC
	 * stopping state machine should be used only when user request
	 * for stopping is received (SINGLESHOT is an exeption).
	 */
	switch (ccdc->state) {
	case ISP_PIPELINE_STREAM_SINGLESHOT:
		ccdc->stopping = CCDC_STOP_REQUEST;
		break;

	case ISP_PIPELINE_STREAM_CONTINUOUS:
		if (ccdc->output & CCDC_OUTPUT_MEMORY) {
			if (ccdc->lsc.state != LSC_STATE_STOPPED)
				__ispccdc_lsc_enable(ccdc, 0);
			__ispccdc_enable(ccdc, 0);
		}
		break;

	case ISP_PIPELINE_STREAM_STOPPED:
		break;
	}

	if (__ispccdc_handle_stopping(ccdc, CCDC_EVENT_VD1))
		goto done;

	if (ccdc->lsc.request == NULL)
		goto done;

	/*
	 * LSC need to be reconfigured. Stop it here and on next LSC_DONE IRQ
	 * do the appropriate changes in registers
	 */
	if (ccdc->lsc.state == LSC_STATE_RUNNING) {
		__ispccdc_lsc_enable(ccdc, 0);
		ccdc->lsc.state = LSC_STATE_RECONFIG;
		goto done;
	}

	/* LSC has been in STOPPED state, enable it */
	if (ccdc->lsc.state == LSC_STATE_STOPPED)
		ispccdc_lsc_enable(ccdc);

done:
	spin_unlock_irqrestore(&ccdc->lsc.req_lock, flags);
}

/*
 * ispccdc_isr - Configure CCDC during interframe time.
 * @ccdc: Pointer to ISP CCDC device.
 * @events: CCDC events
 */
int ispccdc_isr(struct isp_ccdc_device *ccdc, u32 events)
{
	if (ccdc->state == ISP_PIPELINE_STREAM_STOPPED)
		return 0;

	if (!ccdc->syncif.bt_r656_en &&
			(events & IRQ0STATUS_CCDC_VD1_IRQ))
		ispccdc_vd1_isr(ccdc);

	ispccdc_lsc_isr(ccdc, events);

	if (events & IRQ0STATUS_CCDC_VD0_IRQ)
		ispccdc_vd0_isr(ccdc);

	if (!ccdc->syncif.bt_r656_en &&
			(events & IRQ0STATUS_HS_VS_IRQ))
		ispccdc_hs_vs_isr(ccdc);

	return 0;
}

/* -----------------------------------------------------------------------------
 * ISP video operations
 */

static int ccdc_video_queue(struct isp_video *video, struct isp_buffer *buffer)
{
	struct isp_ccdc_device *ccdc = &video->isp->isp_ccdc;

	if (!(ccdc->output & CCDC_OUTPUT_MEMORY))
		return -ENODEV;

	ispccdc_set_outaddr(ccdc, buffer->isp_addr);

	/* We now have a buffer queued on the output, restart the pipeline in
	 * on the next CCDC interrupt if running in continuous mode (or when
	 * starting the stream).
	 */
	ccdc->underrun = 1;

	return 0;
}

static const struct isp_video_operations ccdc_video_ops = {
	.queue = ccdc_video_queue,
};

/* -----------------------------------------------------------------------------
 * V4L2 subdev operations
 */

/*
 * ccdc_ioctl - CCDC module private ioctl's
 * @sd: ISP CCDC V4L2 subdevice
 * @cmd: ioctl command
 * @arg: ioctl argument
 *
 * Return 0 on success or a negative error code otherwise.
 */
static long ccdc_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	struct isp_ccdc_device *ccdc = v4l2_get_subdevdata(sd);
	int ret;

	switch (cmd) {
	case VIDIOC_OMAP3ISP_CCDC_CFG:
		mutex_lock(&ccdc->ioctl_lock);
		ret = ispccdc_config(ccdc, arg);
		mutex_unlock(&ccdc->ioctl_lock);
		break;

	default:
		return -ENOIOCTLCMD;
	}

	return ret;
}

static int ccdc_subscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
				struct v4l2_event_subscription *sub)
{
	if (sub->type != V4L2_EVENT_OMAP3ISP_HS_VS)
		return -EINVAL;

	return v4l2_event_subscribe(fh, sub);
}

static int ccdc_unsubscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
				  struct v4l2_event_subscription *sub)
{
	return v4l2_event_unsubscribe(fh, sub);
}

/*
 * ccdc_set_stream - Enable/Disable streaming on the CCDC module
 * @sd: ISP CCDC V4L2 subdevice
 * @enable: Enable/disable stream
 *
 * When writing to memory, the CCDC hardware can't be enabled without a memory
 * buffer to write to. As the s_stream operation is called in response to a
 * STREAMON call without any buffer queued yet, just update the enabled field
 * and return immediately. The CCDC will be enabled in ccdc_isr_buffer().
 *
 * When not writing to memory enable the CCDC immediately.
 */
static int ccdc_set_stream(struct v4l2_subdev *sd, int enable)
{
	struct isp_ccdc_device *ccdc = v4l2_get_subdevdata(sd);
	struct isp_device *isp = to_isp_device(ccdc);
	int ret = 0;

	if (ccdc->state == ISP_PIPELINE_STREAM_STOPPED) {
		if (enable == ISP_PIPELINE_STREAM_STOPPED)
			return 0;

		isp_subclk_enable(isp, OMAP3_ISP_SUBCLK_CCDC);
		isp_reg_set(isp, OMAP3_ISP_IOMEM_CCDC, ISPCCDC_CFG,
			    ISPCCDC_CFG_VDLC);

		ccdc_configure(ccdc);

		/* TODO: Don't configure the video port if all of its output
		 * links are inactive.
		 */
		ispccdc_config_vp(ccdc);
		ispccdc_enable_vp(ccdc, 0);
		ccdc->error = 0;
		ispccdc_print_status(ccdc);
	}

	switch (enable) {
	case ISP_PIPELINE_STREAM_CONTINUOUS:
		if (ccdc->output & CCDC_OUTPUT_MEMORY)
			isp_sbl_enable(isp, OMAP3_ISP_SBL_CCDC_WRITE);

		if (ccdc->underrun || !(ccdc->output & CCDC_OUTPUT_MEMORY))
			ispccdc_enable(ccdc);

		ccdc->underrun = 0;
		break;

	case ISP_PIPELINE_STREAM_SINGLESHOT:
		if (ccdc->output & CCDC_OUTPUT_MEMORY &&
		    ccdc->state != ISP_PIPELINE_STREAM_SINGLESHOT)
			isp_sbl_enable(isp, OMAP3_ISP_SBL_CCDC_WRITE);

		ispccdc_enable(ccdc);
		break;

	case ISP_PIPELINE_STREAM_STOPPED:
		ret = ispccdc_disable(ccdc);
		if (ccdc->output & CCDC_OUTPUT_MEMORY)
			isp_sbl_disable(isp, OMAP3_ISP_SBL_CCDC_WRITE);
		isp_subclk_disable(isp, OMAP3_ISP_SUBCLK_CCDC);
		ccdc->underrun = 0;
		break;
	}

	ccdc->state = enable;
	return ret;
}

static struct v4l2_mbus_framefmt *
__ccdc_get_format(struct isp_ccdc_device *ccdc, struct v4l2_subdev_fh *fh,
		  unsigned int pad, enum v4l2_subdev_format_whence which)
{
	if (which == V4L2_SUBDEV_FORMAT_TRY)
		return v4l2_subdev_get_try_format(fh, pad);
	else
		return &ccdc->formats[pad];
}

/*
 * ccdc_try_format - Try video format on a pad
 * @ccdc: ISP CCDC device
 * @fh : V4L2 subdev file handle
 * @pad: Pad number
 * @fmt: Format
 */
static void
ccdc_try_format(struct isp_ccdc_device *ccdc, struct v4l2_subdev_fh *fh,
		unsigned int pad, struct v4l2_mbus_framefmt *fmt,
		enum v4l2_subdev_format_whence which)
{
	struct v4l2_mbus_framefmt *format;
	const struct isp_format_info *info;
	unsigned int width = fmt->width;
	unsigned int height = fmt->height;
	unsigned int i;

	switch (pad) {
	case CCDC_PAD_SINK:
		/* TODO: If the CCDC output formatter pad is connected directly
		 * to the resizer, only YUV formats can be used.
		 */
		for (i = 0; i < ARRAY_SIZE(ccdc_fmts); i++) {
			if (fmt->code == ccdc_fmts[i])
				break;
		}

		/* If not found, use SGRBG10 as default */
		if (i >= ARRAY_SIZE(ccdc_fmts))
			fmt->code = V4L2_MBUS_FMT_SGRBG10_1X10;

		/* Clamp the input size. */
		fmt->width = clamp_t(u32, width, 32, 4096);
		fmt->height = clamp_t(u32, height, 32, 4096);
		break;

	case CCDC_PAD_SOURCE_OF:
		format = __ccdc_get_format(ccdc, fh, CCDC_PAD_SINK, which);
		memcpy(fmt, format, sizeof(*fmt));

		/* The data formatter truncates the number of horizontal output
		 * pixels to a multiple of 16. To avoid clipping data, allow
		 * callers to request an output size bigger than the input size
		 * up to the nearest multiple of 16.
		 */
		fmt->width = clamp_t(u32, width, 32, (fmt->width + 15) & ~15);
		fmt->width &= ~15;
		fmt->height = clamp_t(u32, height, 32, fmt->height);
		break;

	case CCDC_PAD_SOURCE_VP:
		format = __ccdc_get_format(ccdc, fh, CCDC_PAD_SINK, which);
		memcpy(fmt, format, sizeof(*fmt));

		/* The video port interface truncates the data to 10 bits. */
		info = isp_video_format_info(fmt->code);
		fmt->code = info->truncated;

		/* The number of lines that can be clocked out from the video
		 * port output must be at least one line less than the number
		 * of input lines.
		 */
		fmt->width = clamp_t(u32, width, 32, fmt->width);
		fmt->height = clamp_t(u32, height, 32, fmt->height - 1);
		break;
	}

	/* Data is written to memory unpacked, each 10-bit or 12-bit pixel is
	 * stored on 2 bytes.
	 */
	fmt->colorspace = V4L2_COLORSPACE_SRGB;
	fmt->field = V4L2_FIELD_NONE;
}

/*
 * ccdc_enum_mbus_code - Handle pixel format enumeration
 * @sd     : pointer to v4l2 subdev structure
 * @fh : V4L2 subdev file handle
 * @code   : pointer to v4l2_subdev_mbus_code_enum structure
 * return -EINVAL or zero on success
 */
static int ccdc_enum_mbus_code(struct v4l2_subdev *sd,
			       struct v4l2_subdev_fh *fh,
			       struct v4l2_subdev_mbus_code_enum *code)
{
	struct isp_ccdc_device *ccdc = v4l2_get_subdevdata(sd);
	struct v4l2_mbus_framefmt *format;

	switch (code->pad) {
	case CCDC_PAD_SINK:
		if (code->index >= ARRAY_SIZE(ccdc_fmts))
			return -EINVAL;

		code->code = ccdc_fmts[code->index];
		break;

	case CCDC_PAD_SOURCE_OF:
	case CCDC_PAD_SOURCE_VP:
		/* No format conversion inside CCDC */
		if (code->index != 0)
			return -EINVAL;

		format = __ccdc_get_format(ccdc, fh, CCDC_PAD_SINK,
					   V4L2_SUBDEV_FORMAT_TRY);

		code->code = format->code;
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static int ccdc_enum_frame_size(struct v4l2_subdev *sd,
				struct v4l2_subdev_fh *fh,
				struct v4l2_subdev_frame_size_enum *fse)
{
	struct isp_ccdc_device *ccdc = v4l2_get_subdevdata(sd);
	struct v4l2_mbus_framefmt format;

	if (fse->index != 0)
		return -EINVAL;

	format.code = fse->code;
	format.width = 1;
	format.height = 1;
	ccdc_try_format(ccdc, fh, fse->pad, &format, V4L2_SUBDEV_FORMAT_TRY);
	fse->min_width = format.width;
	fse->min_height = format.height;

	if (format.code != fse->code)
		return -EINVAL;

	format.code = fse->code;
	format.width = -1;
	format.height = -1;
	ccdc_try_format(ccdc, fh, fse->pad, &format, V4L2_SUBDEV_FORMAT_TRY);
	fse->max_width = format.width;
	fse->max_height = format.height;

	return 0;
}

/*
 * ccdc_get_format - Retrieve the video format on a pad
 * @sd : ISP CCDC V4L2 subdevice
 * @fh : V4L2 subdev file handle
 * @fmt: Format
 *
 * Return 0 on success or -EINVAL if the pad is invalid or doesn't correspond
 * to the format type.
 */
static int ccdc_get_format(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
			   struct v4l2_subdev_format *fmt)
{
	struct isp_ccdc_device *ccdc = v4l2_get_subdevdata(sd);
	struct v4l2_mbus_framefmt *format;

	format = __ccdc_get_format(ccdc, fh, fmt->pad, fmt->which);
	if (format == NULL)
		return -EINVAL;

	fmt->format = *format;
	return 0;
}

/*
 * ccdc_set_format - Set the video format on a pad
 * @sd : ISP CCDC V4L2 subdevice
 * @fh : V4L2 subdev file handle
 * @fmt: Format
 *
 * Return 0 on success or -EINVAL if the pad is invalid or doesn't correspond
 * to the format type.
 */
static int ccdc_set_format(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
			   struct v4l2_subdev_format *fmt)
{
	struct isp_ccdc_device *ccdc = v4l2_get_subdevdata(sd);
	struct v4l2_mbus_framefmt *format;

	format = __ccdc_get_format(ccdc, fh, fmt->pad, fmt->which);
	if (format == NULL)
		return -EINVAL;

	ccdc_try_format(ccdc, fh, fmt->pad, &fmt->format, fmt->which);
	*format = fmt->format;

	/* Propagate the format from sink to source */
	if (fmt->pad == CCDC_PAD_SINK) {
		format = __ccdc_get_format(ccdc, fh, CCDC_PAD_SOURCE_OF,
					   fmt->which);
		*format = fmt->format;
		ccdc_try_format(ccdc, fh, CCDC_PAD_SOURCE_OF, format,
				fmt->which);

		format = __ccdc_get_format(ccdc, fh, CCDC_PAD_SOURCE_VP,
					   fmt->which);
		*format = fmt->format;
		ccdc_try_format(ccdc, fh, CCDC_PAD_SOURCE_VP, format,
				fmt->which);
	}

	return 0;
}

/*
 * ccdc_init_formats - Initialize formats on all pads
 * @sd: ISP CCDC V4L2 subdevice
 * @fh: V4L2 subdev file handle
 *
 * Initialize all pad formats with default values. If fh is not NULL, try
 * formats are initialized on the file handle. Otherwise active formats are
 * initialized on the device.
 */
static int ccdc_init_formats(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct v4l2_subdev_format format;

	memset(&format, 0, sizeof(format));
	format.pad = CCDC_PAD_SINK;
	format.which = fh ? V4L2_SUBDEV_FORMAT_TRY : V4L2_SUBDEV_FORMAT_ACTIVE;
	format.format.code = V4L2_MBUS_FMT_SGRBG10_1X10;
	format.format.width = 4096;
	format.format.height = 4096;
	ccdc_set_format(sd, fh, &format);

	return 0;
}

/* V4L2 subdev core operations */
static const struct v4l2_subdev_core_ops ccdc_v4l2_core_ops = {
	.queryctrl = v4l2_subdev_queryctrl,
	.querymenu = v4l2_subdev_querymenu,
	.g_ctrl = v4l2_subdev_g_ctrl,
	.s_ctrl = v4l2_subdev_s_ctrl,
	.g_ext_ctrls = v4l2_subdev_g_ext_ctrls,
	.try_ext_ctrls = v4l2_subdev_try_ext_ctrls,
	.s_ext_ctrls = v4l2_subdev_s_ext_ctrls,
	.ioctl = ccdc_ioctl,
	.subscribe_event = ccdc_subscribe_event,
	.unsubscribe_event = ccdc_unsubscribe_event,
};

/* V4L2 subdev file operations */
static const struct v4l2_subdev_file_ops ccdc_v4l2_file_ops = {
	.open = ccdc_init_formats,
};

/* V4L2 subdev video operations */
static const struct v4l2_subdev_video_ops ccdc_v4l2_video_ops = {
	.s_stream = ccdc_set_stream,
};

/* V4L2 subdev pad operations */
static const struct v4l2_subdev_pad_ops ccdc_v4l2_pad_ops = {
	.enum_mbus_code = ccdc_enum_mbus_code,
	.enum_frame_size = ccdc_enum_frame_size,
	.get_fmt = ccdc_get_format,
	.set_fmt = ccdc_set_format,
};

/* V4L2 subdev operations */
static const struct v4l2_subdev_ops ccdc_v4l2_ops = {
	.core = &ccdc_v4l2_core_ops,
	.file = &ccdc_v4l2_file_ops,
	.video = &ccdc_v4l2_video_ops,
	.pad = &ccdc_v4l2_pad_ops,
};

/* -----------------------------------------------------------------------------
 * Media entity operations
 */

/*
 * ccdc_link_setup - Setup CCDC connections
 * @entity: CCDC media entity
 * @local: Pad at the local end of the link
 * @remote: Pad at the remote end of the link
 * @flags: Link flags
 *
 * return -EINVAL or zero on success
 */
static int ccdc_link_setup(struct media_entity *entity,
			   const struct media_pad *local,
			   const struct media_pad *remote, u32 flags)
{
	struct v4l2_subdev *sd = media_entity_to_v4l2_subdev(entity);
	struct isp_ccdc_device *ccdc = v4l2_get_subdevdata(sd);
	struct isp_device *isp = to_isp_device(ccdc);

	switch (local->index | media_entity_type(remote->entity)) {
	case CCDC_PAD_SINK | MEDIA_ENTITY_TYPE_V4L2_SUBDEV:
		/* Read from the sensor (parallel interface), CCP2, CSI2a or
		 * CSI2c.
		 */
		if (!(flags & MEDIA_LINK_FLAG_ENABLED)) {
			ccdc->input = CCDC_INPUT_NONE;
			break;
		}

		if (ccdc->input != CCDC_INPUT_NONE)
			return -EBUSY;

		if (remote->entity == &isp->isp_ccp2.subdev.entity)
			ccdc->input = CCDC_INPUT_CCP2B;
		else if (remote->entity == &isp->isp_csi2a.subdev.entity)
			ccdc->input = CCDC_INPUT_CSI2A;
		else if (remote->entity == &isp->isp_csi2c.subdev.entity)
			ccdc->input = CCDC_INPUT_CSI2C;
		else
			ccdc->input = CCDC_INPUT_PARALLEL;

		break;

	/*
	 * The ISP core doesn't support pipelines with multiple video outputs.
	 * Revisit this when it will be implemented, and return -EBUSY for now.
	 */

	case CCDC_PAD_SOURCE_VP | MEDIA_ENTITY_TYPE_V4L2_SUBDEV:
		/* Write to preview engine, histogram and H3A. When none of
		 * those links are active, the video port can be disabled.
		 */
		if (flags & MEDIA_LINK_FLAG_ENABLED) {
			if (ccdc->output & ~CCDC_OUTPUT_PREVIEW)
				return -EBUSY;
			ccdc->output |= CCDC_OUTPUT_PREVIEW;
		} else {
			ccdc->output &= ~CCDC_OUTPUT_PREVIEW;
		}
		break;

	case CCDC_PAD_SOURCE_OF | MEDIA_ENTITY_TYPE_DEVNODE:
		/* Write to memory */
		if (flags & MEDIA_LINK_FLAG_ENABLED) {
			if (ccdc->output & ~CCDC_OUTPUT_MEMORY)
				return -EBUSY;
			ccdc->output |= CCDC_OUTPUT_MEMORY;
		} else {
			ccdc->output &= ~CCDC_OUTPUT_MEMORY;
		}
		break;

	case CCDC_PAD_SOURCE_OF | MEDIA_ENTITY_TYPE_V4L2_SUBDEV:
		/* Write to resizer */
		if (flags & MEDIA_LINK_FLAG_ENABLED) {
			if (ccdc->output & ~CCDC_OUTPUT_RESIZER)
				return -EBUSY;
			ccdc->output |= CCDC_OUTPUT_RESIZER;
		} else {
			ccdc->output &= ~CCDC_OUTPUT_RESIZER;
		}
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

/* media operations */
static const struct media_entity_operations ccdc_media_ops = {
	.link_setup = ccdc_link_setup,
};

/*
 * isp_ccdc_init_entities - Initialize V4L2 subdev and media entity
 * @ccdc: ISP CCDC module
 *
 * Return 0 on success and a negative error code on failure.
 */
static int isp_ccdc_init_entities(struct isp_ccdc_device *ccdc)
{
	struct v4l2_subdev *sd = &ccdc->subdev;
	struct media_pad *pads = ccdc->pads;
	struct media_entity *me = &sd->entity;
	int ret;

	ccdc->input = CCDC_INPUT_NONE;

	v4l2_subdev_init(sd, &ccdc_v4l2_ops);
	strlcpy(sd->name, "OMAP3 ISP CCDC", sizeof(sd->name));
	sd->grp_id = 1 << 16;	/* group ID for isp subdevs */
	v4l2_set_subdevdata(sd, ccdc);
	sd->flags |= V4L2_SUBDEV_FL_HAS_EVENTS | V4L2_SUBDEV_FL_HAS_DEVNODE;
	sd->nevents = OMAP3ISP_CCDC_NEVENTS;

	v4l2_ctrl_handler_init(&ccdc->ctrls, 1);
	sd->ctrl_handler = &ccdc->ctrls;

	pads[CCDC_PAD_SINK].flags = MEDIA_PAD_FLAG_INPUT;
	pads[CCDC_PAD_SOURCE_VP].flags = MEDIA_PAD_FLAG_OUTPUT;
	pads[CCDC_PAD_SOURCE_OF].flags = MEDIA_PAD_FLAG_OUTPUT;

	me->ops = &ccdc_media_ops;
	ret = media_entity_init(me, CCDC_PADS_NUM, pads, 0);
	if (ret < 0)
		return ret;

	ccdc_init_formats(sd, NULL);

	ccdc->video_out.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ccdc->video_out.ops = &ccdc_video_ops;
	ccdc->video_out.isp = to_isp_device(ccdc);
	ccdc->video_out.capture_mem = PAGE_ALIGN(4096 * 4096) * 3;
	ccdc->video_out.alignment = 32;

	ret = isp_video_init(&ccdc->video_out, "CCDC");
	if (ret < 0)
		return ret;

	/* Connect the CCDC subdev to the video node. */
	ret = media_entity_create_link(&ccdc->subdev.entity, CCDC_PAD_SOURCE_OF,
			&ccdc->video_out.video.entity, 0, 0);
	if (ret < 0)
		return ret;

	return 0;
}

void isp_ccdc_unregister_entities(struct isp_ccdc_device *ccdc)
{
	media_entity_cleanup(&ccdc->subdev.entity);

	v4l2_device_unregister_subdev(&ccdc->subdev);
	v4l2_ctrl_handler_free(&ccdc->ctrls);
	isp_video_unregister(&ccdc->video_out);
}

int isp_ccdc_register_entities(struct isp_ccdc_device *ccdc,
	struct v4l2_device *vdev)
{
	int ret;

	/* Register the subdev and video node. */
	ret = v4l2_device_register_subdev(vdev, &ccdc->subdev);
	if (ret < 0)
		goto error;

	ret = isp_video_register(&ccdc->video_out, vdev);
	if (ret < 0)
		goto error;

	return 0;

error:
	isp_ccdc_unregister_entities(ccdc);
	return ret;
}

/* -----------------------------------------------------------------------------
 * ISP CCDC initialisation and cleanup
 */

/*
 * isp_ccdc_init - CCDC module initialization.
 * @dev: Device pointer specific to the OMAP3 ISP.
 *
 * TODO: Get the initialisation values from platform data.
 *
 * Return 0 on success or a negative error code otherwise.
 */
int isp_ccdc_init(struct isp_device *isp)
{
	struct isp_ccdc_device *ccdc = &isp->isp_ccdc;

	spin_lock_init(&ccdc->lock);
	init_waitqueue_head(&ccdc->wait);
	mutex_init(&ccdc->ioctl_lock);

	ccdc->stopping = CCDC_STOP_NOT_REQUESTED;

	INIT_WORK(&ccdc->lsc.table_work, ispccdc_lsc_free_table_work);
	ccdc->lsc.state = LSC_STATE_STOPPED;
	INIT_LIST_HEAD(&ccdc->lsc.free_queue);
	spin_lock_init(&ccdc->lsc.req_lock);

	ccdc->syncif.ccdc_mastermode = 0;
	ccdc->syncif.datapol = 0;
	ccdc->syncif.datsz = 0;
	ccdc->syncif.fldmode = 0;
	ccdc->syncif.fldout = 0;
	ccdc->syncif.fldpol = 0;
	ccdc->syncif.fldstat = 0;
	ccdc->syncif.hdpol = 0;
	ccdc->syncif.vdpol = 0;

	ccdc->clamp.oblen = 0;
	ccdc->clamp.dcsubval = 64;

	ccdc->vpcfg.pixelclk = 0;

	ccdc->update = 0;
	ccdc_apply_controls(ccdc);

	return isp_ccdc_init_entities(ccdc);
}

/*
 * isp_ccdc_cleanup - CCDC module cleanup.
 * @dev: Device pointer specific to the OMAP3 ISP.
 */
void isp_ccdc_cleanup(struct isp_device *isp)
{
	struct isp_ccdc_device *ccdc = &isp->isp_ccdc;

	/* Free LSC requests. As the CCDC is stopped there's no active request,
	 * so only the pending request and the free queue need to be handled.
	 */
	ispccdc_lsc_free_request(ccdc, ccdc->lsc.request);
	cancel_work_sync(&ccdc->lsc.table_work);
	ispccdc_lsc_free_queue(ccdc, &ccdc->lsc.free_queue);

	if (ccdc->fpc.fpcaddr != 0)
		iommu_vfree(isp->iommu, ccdc->fpc.fpcaddr);
}
