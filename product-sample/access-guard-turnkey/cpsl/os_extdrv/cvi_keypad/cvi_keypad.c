#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/delay.h>

typedef struct
{
	struct input_dev *input_keypd_dev;
	struct delayed_work work;
	spinlock_t lock;
    #if 0
	int Kb1;
	int	Kb2;
	int irq;
    #endif
}fake_keypad_s;


static ssize_t fakekeypad_sysfs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("%s: %d \n", __FUNCTION__, __LINE__);
	return 0;
}   

static ssize_t fakekeypad_sysfs_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    char * const delim = " ";
    char *cur = (char *)buf;
    char *token_name;
    int key_code;
    fake_keypad_s *keypad_data = dev->driver_data;

    token_name = strsep(&cur, delim);
    #if 0
	if (strncmp(token_name, "key-", 4)) {
		dev_info(dev, "input invalid %s\n", token_name);
		return -ENODEV;
	}
    #endif

    if (!strncmp(token_name, "0", sizeof("0") - 1)) {
        key_code = KEY_0;
    } else if (!strncmp(token_name, "1", sizeof("1") - 1)) {
        key_code =KEY_1;
    } else if (!strncmp(token_name, "2", sizeof("2") - 1)) {
        key_code =KEY_2;
    } else if (!strncmp(token_name, "3", sizeof("3") - 1)) {
        key_code =KEY_3;
    } else if (!strncmp(token_name, "4", sizeof("4") - 1)) {
        key_code =KEY_4;
    } else if (!strncmp(token_name, "5", sizeof("5") - 1)) {
        key_code =KEY_5;
    } else if (!strncmp(token_name, "6", sizeof("6") - 1)) {
        key_code =KEY_6;
    } else if (!strncmp(token_name, "7", sizeof("7") - 1)) {
        key_code =KEY_7;
    } else if (!strncmp(token_name, "8", sizeof("8") - 1)) {
        key_code =KEY_8;
    } else if (!strncmp(token_name, "9", sizeof("9") - 1)) {
        key_code =KEY_9;
    } else if (!strncmp(token_name, "ok", sizeof("ok") - 1)) {
        key_code =KEY_ENTER;
    } else if (!strncmp(token_name, "esc", sizeof("esc") - 1)) {
        key_code =KEY_ESC;
    } else if (!strncmp(token_name, "left", sizeof("left") - 1)) {
        key_code =KEY_LEFT;
    } else if (!strncmp(token_name, "right", sizeof("right") - 1)) {
        key_code =KEY_RIGHT;
    } else if (!strncmp(token_name, "up", sizeof("up") - 1)) {
        key_code =KEY_UP;
    } else if (!strncmp(token_name, "down", sizeof("down") - 1)) {
        key_code =KEY_DOWN;
    } else if (!strncmp(token_name, "menu", sizeof("menu") - 1)) {
        key_code =KEY_MENU;
    } else {
        dev_info(dev, "input key invalid %s\n", token_name);
        return -ENODEV;
    }

    dev_info(dev, "linux input key value 0x%08X \n", key_code);

    input_report_key(keypad_data->input_keypd_dev, key_code, 1); //key pressed
    input_sync(keypad_data->input_keypd_dev); 
    msleep(500);
    input_report_key(keypad_data->input_keypd_dev, key_code, 0); //key released
    input_sync(keypad_data->input_keypd_dev); 

    return size;
}

static DEVICE_ATTR(fakekeypad, 0660, fakekeypad_sysfs_show, fakekeypad_sysfs_store);


int fake_keypad_probe(struct platform_device *pdev)
{
	int err = 0;
	fake_keypad_s *keypad_data;
	struct input_dev *keypd_dev;
	struct device *dev = &pdev->dev;

	keypad_data = kzalloc(sizeof(fake_keypad_s), GFP_KERNEL);
	keypd_dev = devm_input_allocate_device(dev);
	
	if (!keypad_data || !keypd_dev) {
		dev_err(dev, "failed to allocate input device\n");
		err = -ENOMEM;
		goto err_free_mem;
	}

	keypd_dev->name = "cvitek-fakekeypad";
	//keypd_dev->phys = "cvitek-keypad/input0";
	keypd_dev->id.bustype = BUS_HOST;
	keypd_dev->id.vendor = 0x4585;
	keypd_dev->id.product = 0x3333;
	keypd_dev->id.version = 0x0001;

	//keypd_dev->evbit[0] = BIT_MASK(EV_KEY);
	__set_bit(EV_KEY, keypd_dev->evbit);
	__set_bit(KEY_0, keypd_dev->keybit);
	__set_bit(KEY_1, keypd_dev->keybit);
	__set_bit(KEY_2, keypd_dev->keybit);
	__set_bit(KEY_3, keypd_dev->keybit);
	__set_bit(KEY_4, keypd_dev->keybit);
	__set_bit(KEY_5, keypd_dev->keybit);
	__set_bit(KEY_6, keypd_dev->keybit);
	__set_bit(KEY_7, keypd_dev->keybit);
	__set_bit(KEY_8, keypd_dev->keybit);
	__set_bit(KEY_9, keypd_dev->keybit);
	__set_bit(KEY_ENTER, keypd_dev->keybit);
	__set_bit(KEY_ESC, keypd_dev->keybit);
	__set_bit(KEY_UP, keypd_dev->keybit);
	__set_bit(KEY_DOWN, keypd_dev->keybit);
	__set_bit(KEY_LEFT, keypd_dev->keybit);
	__set_bit(KEY_RIGHT, keypd_dev->keybit);
    __set_bit(KEY_MENU, keypd_dev->keybit);

	input_set_capability(keypd_dev, EV_MSC, MSC_SCAN);
	keypad_data->input_keypd_dev = keypd_dev ;
	
	platform_set_drvdata(pdev, keypad_data);
    err = device_create_file(&pdev->dev, &dev_attr_fakekeypad);
	if (err)
		goto err_free_mem;
	
	err = input_register_device(keypd_dev);
	
	if (err){
		goto err_free_mem;
	}
	return 0;

err_free_mem:
	input_free_device(keypd_dev);
	kfree(keypad_data);
	return err;
}

int fake_keypad_remove(struct platform_device *pdev)
{
	fake_keypad_s *keypad_data;
	
	keypad_data = platform_get_drvdata(pdev);

	input_unregister_device(keypad_data->input_keypd_dev);

	kfree(keypad_data);
	return 0;
}


static const struct platform_device_id fake_keypad_id_table[] = {
    {"fake-keypad", 0 },
	{"74x164-keypad", 0 },
	{ }
};
MODULE_DEVICE_TABLE(keypad, fake_keypad_id_table);

static struct platform_driver cvitek_keypad_driver = {
	.driver	= {
		.name= "cvitek-keypad",
		//.owner= THIS_MODULE,
		//.of_match_table = of_match_ptr(bm1880_keypad_74x164_dt_match),
	},
	.probe = fake_keypad_probe,
	.remove = fake_keypad_remove,
    .id_table = fake_keypad_id_table,
};

static struct platform_device *keypad_platform_device;

static int fake_keypad_init(void)
{
	int res;
    int error;
    printk("%s: %d \n", __FUNCTION__, __LINE__);
	res = platform_driver_register(&cvitek_keypad_driver);
	if (res)	{
		printk(KERN_INFO "fail : platrom driver %s (%d) \n", cvitek_keypad_driver.driver.name, res);
		return res;
	}

    keypad_platform_device = platform_device_alloc("fake-keypad", -1);
    if (!keypad_platform_device) {
        error = -ENOMEM;
        goto err_driver_unregister;
    }

    error = platform_device_add(keypad_platform_device);
    if (error)
        goto err_free_device;
    return 0;

err_free_device:
    platform_device_put(keypad_platform_device);
err_driver_unregister:
    platform_driver_unregister(&cvitek_keypad_driver);
    return error;
}

static void fake_keypad_exit(void)
{
	platform_driver_unregister(&cvitek_keypad_driver);
}
module_init(fake_keypad_init);
module_exit(fake_keypad_exit);


MODULE_AUTHOR("<liang.wang@cvitek.com>");
MODULE_DESCRIPTION("cvitek cv18xx keypad Driver");
MODULE_ALIAS("platform:Cvitek");
MODULE_LICENSE("GPL");