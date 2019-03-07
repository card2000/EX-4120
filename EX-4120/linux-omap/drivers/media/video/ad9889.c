/*
 * drivers/video/omap2/ad9889.c
 *
 * Copyright (C) 2011 JVC Kenwood corporation
 * Author: Keiichi Matsuhashi
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>

struct ad9889 {
	char			phys[32];
	struct i2c_client	*client;
};


//static inline int ad9889_write_word_data(struct ad9889 *dvi, u8 cmd, u16 data)
//{
//	u16 val;
//
//	val = swab16(data);
//	return i2c_smbus_write_word_data(dvi->client, cmd, val);
//}

//static inline int ad9889_write_cmd(struct ad9889 *dvi, u8 value)
//{
//	return i2c_smbus_write_byte(dvi->client, value);
//}


#if 0	/* deleted by Matsuhashi */
static int ad9889_write_register(struct ad9889 *dvi)
{
	int err;
/****************************************************************/
/*	int err;						*/
/*	int cmd, data;						*/
/*   ##### Write Command(single byte) #####			*/
/*	cmd = ;							*/
/*	err = ad9889_write_cmd(dvi, cmd);			*/
/*	if (err < 0)						*/
/*		return err;					*/
/*								*/
/*   ##### Write Command and Data(Double bytes) ##### 		*/
/*	cmd = ;							*/
/*	data = ;						*/
/*	err = ad9889_write_word_data(dvi, cmd, data);		*/
/*	if (err < 0)						*/
/*		return err;					*/
/****************************************************************/

	err = i2c_smbus_write_byte_data(dvi->client, 0x41, 0x10);
        if( 0 != err )
		printk(KERN_INFO "I2C write error:%d\n", err);
        i2c_smbus_write_byte_data(dvi->client, 0x17, 0x02);
        i2c_smbus_write_byte_data(dvi->client, 0xa2, 0x84);
        i2c_smbus_write_byte_data(dvi->client, 0xa3, 0x84);
        i2c_smbus_write_byte_data(dvi->client, 0xaf, 0x04);
        i2c_smbus_write_byte_data(dvi->client, 0xba, 0x60);
        i2c_smbus_write_byte_data(dvi->client, 0xbb, 0xff);

	return 0;
}
#endif

static int __devinit ad9889_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	struct ad9889 *dvi;
	int err;

	printk(KERN_INFO "ad9889_probe\n");
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_READ_BYTE_DATA)){
		printk(KERN_INFO "function check NG!\n");
		return -EIO;
	}

	dvi = kzalloc(sizeof(struct ad9889), GFP_KERNEL);
	if (!dvi) {
		err = -ENOMEM;
		goto err_free_mem;
	}

	dvi->client = client;

/* deleted by Matsuhashi */
	/* Prepare for dvi output */
/*	err = ad9889_write_register(dvi);
	if (err < 0)
		goto err_free_mem;
*/
	i2c_set_clientdata(client, dvi);

	return 0;

 err_free_mem:
	kfree(dvi);
	return err;
}

static int __devexit ad9889_remove(struct i2c_client *client)
{
	struct ad9889	*dvi = i2c_get_clientdata(client);

	kfree(dvi);

	return 0;
}

static struct i2c_device_id ad9889_idtable[] = {
	{ "ad9889", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, ad9889_idtable);

static struct i2c_driver ad9889_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "ad9889"
	},
	.id_table	= ad9889_idtable,
	.probe		= ad9889_probe,
	.remove		= __devexit_p(ad9889_remove),
};

static int __init ad9889_init(void)
{
	printk(KERN_INFO "ad9889_init\n");
	return i2c_add_driver(&ad9889_driver);
}

static void __exit ad9889_exit(void)
{
	i2c_del_driver(&ad9889_driver);
}

module_init(ad9889_init);
module_exit(ad9889_exit);

MODULE_AUTHOR("Keiichi Matsuhashi");
MODULE_DESCRIPTION("ad9889 DVI transmitter Driver");
MODULE_LICENSE("GPL");
