#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <linux/bcd.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/semaphore.h>
#include <linux/miscdevice.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

struct atsh204a {
	u8			regs[11];
	struct i2c_client	*client;
	struct enc_device	*enc;
	struct work_struct	work;
};

static const struct i2c_device_id atsh204a_id[] = {
	{ "enc-atsh204a", 0 },
	{}
};

MODULE_DEVICE_TABLE(i2c, atsh204a_id);

struct i2c_adapter *adapter;
#define  ENC_ADDR    	 0xC8>>1


#define DRV_ENC_CMD_BASE           0x04
#define DRV_ENC_CMD_WRITE          _IOW(DRV_ENC_CMD_BASE, 1, unsigned int)
#define DRV_ENC_CMD_READ           _IOR(DRV_ENC_CMD_BASE, 2, unsigned int)


static struct i2c_client *atsh204a_client;
static int adapter_id = 0;
module_param(adapter_id, int, S_IRUSR);

int atsh204a_open(struct inode *inode, struct file *file)
{
    file->private_data = NULL;
    return 0;
}

int atsh204a_close(struct inode *inode, struct file *file)
{
    return 0;
}

unsigned char hi_atsh204a_i2c_read(unsigned char dev_addr, unsigned char reg_addr, unsigned char *data,int data_num)
{	
	int ret;
	static struct i2c_msg msg[2];
	unsigned char buf[128];
	memset(buf,0x0,sizeof(buf));
	msg[0].addr = dev_addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = buf;

	msg[1].addr = dev_addr;
	msg[1].flags = 0;
	msg[1].flags |= I2C_M_RD;
	msg[1].len = data_num;
	msg[1].buf = buf;
	
	ret = i2c_transfer(adapter,&msg[0],2);
	if(ret != 2)
	{
		printk("line = %d, ret = %d\n",__LINE__, ret);
		return ret;
	}
	else
	{
		memcpy(data,buf,data_num);
		ret = 0;
	}
		
	return ret;
}

int hi_atsh204a_i2c_write(unsigned char dev_addr,unsigned char reg_addr, unsigned char *data,int data_num)
{
	unsigned char tmp_buf[128];
	int ret = 0;
    int idx = 0;
	int i;
	struct i2c_client *client = atsh204a_client;

    atsh204a_client->addr = dev_addr;

    tmp_buf[idx++] = reg_addr;

	for(i=0;i<data_num;i++)
	{
		tmp_buf[idx++] = data[i];
	}

	ret = i2c_master_send(client, tmp_buf, idx);

	return ret;   
}

long atsh204a_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	switch(cmd)
	{
		case DRV_ENC_CMD_WRITE:
		{
			unsigned char buf[128];
			int len;
			memset(buf,0x0,sizeof(buf));
			ret = copy_from_user(&buf, (void __user *)arg, sizeof(buf));
            if(ret != 0)
            {
                return -1;
            }
			if(buf[0] <= 2)
			{
				len = buf[0];
				if(0x00 == buf[1]) //buf[1] is register addr, wake: 0x00
				{
					ret = hi_atsh204a_i2c_write(0x00,buf[1],&buf[2],len-1);
				}
				else //buf[1] is register addr, idle: 0x02
				{
					ret = hi_atsh204a_i2c_write(ENC_ADDR,buf[1],&buf[2],len-1);
				}
			}
			else
			{
				len = buf[2];
				ret = hi_atsh204a_i2c_write(ENC_ADDR,buf[1],&buf[2],len);
			}
		}
		break;	

		case DRV_ENC_CMD_READ:
		{
			unsigned char buf[128];
			memset(buf,0x0,sizeof(buf));
			ret = copy_from_user(&buf, (void __user *)arg, sizeof(buf));
            if(ret != 0)
            {
                return -1;
            }
			ret = hi_atsh204a_i2c_read(ENC_ADDR,buf[1],buf,buf[0]);
			if(ret != 0)
            {
				printk("DRV_ENC_CMD_READ ERROR ret = %d\n",ret);
                return -1;
            }
			ret = copy_to_user((void __user *)arg, buf, sizeof(buf));
            if(ret != 0)
            {
                return -1;
            }
			ret = 2;
        }		
		break;

		default:
		{
			printk("ENC DRIVER ERROR CMD cmd =0x%x\n",cmd);
			return -1;
		}
	}
	
	return ret;
}

static int atsh204a_probe(struct i2c_client *client,const struct i2c_device_id *id)
{	
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct atsh204a *atsh204a;
	u8 *buf;
	
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
	{
		printk("line = %d\n",__LINE__);
		return -EIO;
	}

	atsh204a = kzalloc(sizeof(struct atsh204a), GFP_KERNEL);
	if (!atsh204a)
	{
		printk("line = %d\n",__LINE__);
		return -EIO;
	}
	
	i2c_set_clientdata(client, atsh204a);

	atsh204a->client = client;
	buf = atsh204a->regs;

	device_set_wakeup_capable(&client->dev, 1);
	
	return 0;
}

static int atsh204a_remove(struct i2c_client *client)
{
	struct atsh204a *atsh204a = i2c_get_clientdata(client);

	cancel_work_sync(&atsh204a->work);
	kfree(atsh204a);

	return 0;
}

static struct i2c_driver atsh204a_driver = {
	.driver = {
		.name	= "enc-atsh204a",
	},
	.probe		= atsh204a_probe,
	.remove		= atsh204a_remove,
	.id_table	= atsh204a_id,
};

static struct i2c_board_info hi_info = {
	     I2C_BOARD_INFO("enc-atsh204a", 0xC8>>1),
};


static struct file_operations atsh204a_fops = {
    .owner          = THIS_MODULE,
	.open           = atsh204a_open,
    .release        = atsh204a_close,
    .unlocked_ioctl = atsh204a_ioctl,
    .compat_ioctl = atsh204a_ioctl,

};

static struct miscdevice atsh204a_dev = {
    .minor      = MISC_DYNAMIC_MINOR,
    .name       = "atsh204a-enc",
    .fops       = &atsh204a_fops,
};

static int __init atsh204a_init(void)
{
	int ret;
	adapter = i2c_get_adapter(adapter_id);
	if (!adapter) 
	{
		printk("%s : can't get i2c adapter %d\n",__func__,__LINE__);
	}

	atsh204a_client = i2c_new_device(adapter, &hi_info);
								
	i2c_put_adapter(adapter);
	if (!atsh204a_client) 
	 {
		printk("%s : can't add i2c device \n",__func__);
	}

	ret = i2c_add_driver(&atsh204a_driver);
	if (0 != ret)
    {
        printk("i2c_add_driver fail!\n");
        return -1;
    }

	ret = misc_register(&atsh204a_dev);
    if (0 != ret)
    {
        printk("misc_register fail!\n");
        return -1;
    }
	
	//printk("date = %s time = %s  atsh204a_init OK \n",__DATE__,__TIME__);
	printk("atsh204a_init OK \n");
	return 0;

}

module_init(atsh204a_init);

static void __exit atsh204a_exit(void)
{
	misc_deregister(&atsh204a_dev);
	i2c_unregister_device(atsh204a_client);
	i2c_del_driver(&atsh204a_driver);
}
module_exit(atsh204a_exit);

MODULE_DESCRIPTION("ENC driver for atsh204a chip");
MODULE_LICENSE("GPL");
