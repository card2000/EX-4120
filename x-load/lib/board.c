/*
 * Copyright (C) 2005 Texas Instruments.
 *
 * (C) Copyright 2004
 * Jian Zhang, Texas Instruments, jzhang@ti.com.
 *
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>

extern int misc_init_r (void);
extern u32 get_mem_type(void);

#ifdef CFG_PRINTF
int print_info(void)
{
	printf("\n\nTexas Instruments X-Loader 1.51 ("
			__DATE__ " - " __TIME__ ")\n");
	return 0;
}
#endif

/**
 * Initialize available memory
 */
int mem_init(void)
{
#ifdef CONFIG_AM3517EVM
	emif_init();
#else
	sdrc_init();
#endif
	return 0;
}

typedef int (init_fnc_t) (void);

init_fnc_t *init_sequence[] = {
	cpu_init,		/* basic cpu dependent setup */
	board_init,		/* basic board dependent setup */
#ifdef CFG_PRINTF
 	serial_init,		/* serial communications setup */
	print_info,
#endif
	nand_init,		/* board specific nand init */
	mem_init,		/* board specific memory init */
  	NULL,
};

void start_armboot (void)
{
  	init_fnc_t **init_fnc_ptr;
 	int i;
	uchar *buf;
	unsigned int *trace;
	unsigned int boot_device;

   	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0) {
			hang ();
		}
	}

	misc_init_r();
	buf =  (uchar*) CFG_LOADADDR;

	/* Checking boot device */
	trace = (unsigned int *)0x4020FFB4;
	boot_device = *trace;
	if(0 != (boot_device & BOOT_DEV_NAND)) {
		printf("Starting X-loader on NAND \n");
	} else if(0 != (boot_device & BOOT_DEV_MMC1)) {
		printf("Starting X-loader on MMC \n");
	}

	/* Always first try mmc without checking boot pins */
#ifndef CONFIG_OMAP3_BEAGLE
	if ((get_mem_type() == MMC_ONENAND) || (get_mem_type() == MMC_NAND))
#endif	/* CONFIG_OMAP3_BEAGLE */
		buf += mmc_boot(buf);

	if (buf == (uchar *)CFG_LOADADDR) {
		if ((get_mem_type() == GPMC_NAND) || (get_mem_type() == MMC_NAND)){
#ifdef CFG_PRINTF
			printf("Booting from nand . . .\n");
#endif
			for (i = NAND_UBOOT_START; i < NAND_UBOOT_END; i+= NAND_BLOCK_SIZE){
				if (!nand_read_block(buf, i))
					buf += NAND_BLOCK_SIZE; /* advance buf ptr */
			}
		}
	}

	if (buf == (uchar *)CFG_LOADADDR)
		hang();

	/* go run U-Boot and never return */
  	printf("Starting OS Bootloader...\n");
 	((init_fnc_t *)CFG_LOADADDR)();

	/* should never come here */
}

void hang (void)
{
	/* call board specific hang function */
	board_hang();

	/* if board_hang() returns, hange here */
	printf("X-Loader hangs\n");
	for (;;);
}
