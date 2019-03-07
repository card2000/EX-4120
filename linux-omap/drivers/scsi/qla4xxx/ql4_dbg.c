/*
 * QLogic iSCSI HBA Driver
 * Copyright (c)  2003-2006 QLogic Corporation
 *
 * See LICENSE.qla4xxx for copyright and licensing details.
 */

#include "ql4_def.h"
#include "ql4_glbl.h"
#include "ql4_dbg.h"
#include "ql4_inline.h"

void qla4xxx_dump_buffer(void *b, uint32_t size)
{
	uint32_t cnt;
	uint8_t *c = b;

	printk(" 0   1   2   3   4   5   6   7   8   9  Ah  Bh  Ch  Dh  Eh  "
	       "Fh\n");
	printk("------------------------------------------------------------"
	       "--\n");
	for (cnt = 0; cnt < size; c++) {
		printk(KERN_INFO "%02x", *c);
		if (!(++cnt % 16))
			printk(KERN_INFO "\n");

		else
			printk(KERN_INFO "  ");
	}
	printk(KERN_INFO "\n");
}

void qla4xxx_dump_registers(struct scsi_qla_host *ha)
{
	uint8_t i;

	if (is_qla8022(ha)) {
		for (i = 1; i < MBOX_REG_COUNT; i++)
			printk(KERN_INFO "mailbox[%d]     = 0x%08X\n",
			    i, readl(&ha->qla4_8xxx_reg->mailbox_in[i]));
		return;
	}

	for (i = 0; i < MBOX_REG_COUNT; i++) {
		printk(KERN_INFO "0x%02X mailbox[%d]      = 0x%08X\n",
		    (uint8_t) offsetof(struct isp_reg, mailbox[i]), i,
		    readw(&ha->reg->mailbox[i]));
	}

	printk(KERN_INFO "0x%02X flash_address            = 0x%08X\n",
	    (uint8_t) offsetof(struct isp_reg, flash_address),
	    readw(&ha->reg->flash_address));
	printk(KERN_INFO "0x%02X flash_data               = 0x%08X\n",
	    (uint8_t) offsetof(struct isp_reg, flash_data),
	    readw(&ha->reg->flash_data));
	printk(KERN_INFO "0x%02X ctrl_status              = 0x%08X\n",
	    (uint8_t) offsetof(struct isp_reg, ctrl_status),
	    readw(&ha->reg->ctrl_status));

	if (is_qla4010(ha)) {
		printk(KERN_INFO "0x%02X nvram            = 0x%08X\n",
		    (uint8_t) offsetof(struct isp_reg, u1.isp4010.nvram),
		    readw(&ha->reg->u1.isp4010.nvram));
	} else if (is_qla4022(ha) | is_qla4032(ha)) {
		printk(KERN_INFO "0x%02X intr_mask        = 0x%08X\n",
		    (uint8_t) offsetof(struct isp_reg, u1.isp4022.intr_mask),
		    readw(&ha->reg->u1.isp4022.intr_mask));
		printk(KERN_INFO "0x%02X nvram            = 0x%08X\n",
		    (uint8_t) offsetof(struct isp_reg, u1.isp4022.nvram),
		    readw(&ha->reg->u1.isp4022.nvram));
		printk(KERN_INFO "0x%02X semaphore	  = 0x%08X\n",
		    (uint8_t) offsetof(struct isp_reg, u1.isp4022.semaphore),
		    readw(&ha->reg->u1.isp4022.semaphore));
	}
	printk(KERN_INFO "0x%02X req_q_in                 = 0x%08X\n",
	    (uint8_t) offsetof(struct isp_reg, req_q_in),
	    readw(&ha->reg->req_q_in));
	printk(KERN_INFO "0x%02X rsp_q_out                = 0x%08X\n",
	    (uint8_t) offsetof(struct isp_reg, rsp_q_out),
	    readw(&ha->reg->rsp_q_out));

	if (is_qla4010(ha)) {
		printk(KERN_INFO "0x%02X ext_hw_conf      = 0x%08X\n",
		    (uint8_t) offsetof(struct isp_reg, u2.isp4010.ext_hw_conf),
		    readw(&ha->reg->u2.isp4010.ext_hw_conf));
		printk(KERN_INFO "0x%02X port_ctrl        = 0x%08X\n",
		    (uint8_t) offsetof(struct isp_reg, u2.isp4010.port_ctrl),
		    readw(&ha->reg->u2.isp4010.port_ctrl));
		printk(KERN_INFO "0x%02X port_status      = 0x%08X\n",
		    (uint8_t) offsetof(struct isp_reg, u2.isp4010.port_status),
		    readw(&ha->reg->u2.isp4010.port_status));
		printk(KERN_INFO "0x%02X req_q_out        = 0x%08X\n",
		    (uint8_t) offsetof(struct isp_reg, u2.isp4010.req_q_out),
		    readw(&ha->reg->u2.isp4010.req_q_out));
		printk(KERN_INFO "0x%02X gp_out           = 0x%08X\n",
		    (uint8_t) offsetof(struct isp_reg, u2.isp4010.gp_out),
		    readw(&ha->reg->u2.isp4010.gp_out));
		printk(KERN_INFO "0x%02X gp_in	          = 0x%08X\n",
		    (uint8_t) offsetof(struct isp_reg, u2.isp4010.gp_in),
		    readw(&ha->reg->u2.isp4010.gp_in));
		printk(KERN_INFO "0x%02X port_err_status  = 0x%08X\n", (uint8_t)
		    offsetof(struct isp_reg, u2.isp4010.port_err_status),
		    readw(&ha->reg->u2.isp4010.port_err_status));
	} else if (is_qla4022(ha) | is_qla4032(ha)) {
		printk(KERN_INFO "Page 0 Registers:\n");
		printk(KERN_INFO "0x%02X ext_hw_conf      = 0x%08X\n", (uint8_t)
		    offsetof(struct isp_reg, u2.isp4022.p0.ext_hw_conf),
		    readw(&ha->reg->u2.isp4022.p0.ext_hw_conf));
		printk(KERN_INFO "0x%02X port_ctrl        = 0x%08X\n", (uint8_t)
		    offsetof(struct isp_reg, u2.isp4022.p0.port_ctrl),
		    readw(&ha->reg->u2.isp4022.p0.port_ctrl));
		printk(KERN_INFO "0x%02X port_status      = 0x%08X\n", (uint8_t)
		    offsetof(struct isp_reg, u2.isp4022.p0.port_status),
		    readw(&ha->reg->u2.isp4022.p0.port_status));
		printk(KERN_INFO "0x%02X gp_out           = 0x%08X\n",
		    (uint8_t) offsetof(struct isp_reg, u2.isp4022.p0.gp_out),
		    readw(&ha->reg->u2.isp4022.p0.gp_out));
		printk(KERN_INFO "0x%02X gp_in            = 0x%08X\n",
		    (uint8_t) offsetof(struct isp_reg, u2.isp4022.p0.gp_in),
		    readw(&ha->reg->u2.isp4022.p0.gp_in));
		printk(KERN_INFO "0x%02X port_err_status  = 0x%08X\n", (uint8_t)
		    offsetof(struct isp_reg, u2.isp4022.p0.port_err_status),
		    readw(&ha->reg->u2.isp4022.p0.port_err_status));
		printk(KERN_INFO "Page 1 Registers:\n");
		writel(HOST_MEM_CFG_PAGE & set_rmask(CSR_SCSI_PAGE_SELECT),
		    &ha->reg->ctrl_status);
		printk(KERN_INFO "0x%02X req_q_out        = 0x%08X\n",
		    (uint8_t) offsetof(struct isp_reg, u2.isp4022.p1.req_q_out),
		    readw(&ha->reg->u2.isp4022.p1.req_q_out));
		writel(PORT_CTRL_STAT_PAGE & set_rmask(CSR_SCSI_PAGE_SELECT),
		    &ha->reg->ctrl_status);
	}
}
