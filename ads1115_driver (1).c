#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/delay.h>

#define DRIVER_NAME     "driver_ads1115"
#define CLASS_NAME      "ads1115"
#define DEVICE_NAME     "ads1115"
#define IOCTL_MAGIC     'a'

#define ADC1_R _IOR(IOCTL_MAGIC, 1, int)
#define ADC2_R _IOR(IOCTL_MAGIC, 2, int)
#define ADC3_R _IOR(IOCTL_MAGIC, 3, int)
#define ADC4_R _IOR(IOCTL_MAGIC, 4, int)

static int major_number;
static struct class* ads1115_class = NULL;
static struct device* ads1115_device = NULL;
static struct i2c_client *ads1115_client;

static int ads1115_configure(struct i2c_client *client, int pga, int mux)
{
    u16 config = 0;
    u8 buf[2];  // Declare the buffer at the beginning of the function

    config |= (1 << 15);                // Start single conversion
    config |= ((mux & 0x07) << 12);     // MUX config
    config |= ((pga & 0x07) << 9);      // Programmable gain
    config |= (1 << 8);                 // Single-shot mode
    config |= (4 << 5);                 // Data rate = 128 SPS

    buf[0] = (config >> 8) & 0xFF;
    buf[1] = config & 0xFF;

    return i2c_smbus_write_i2c_block_data(client, 0x01, 2, buf); // Config register
}


static int ads1115_read_conversion(struct i2c_client *client)
{
    int ret;
    s32 raw;

    msleep(10); // Wait for conversion

    ret = i2c_smbus_write_byte(client, 0x00); // Point to conversion register
    if (ret < 0)
        return ret;

    raw = i2c_smbus_read_word_data(client, 0x00);
    if (raw < 0)
        return raw;

    return ((raw & 0xFF) << 8) | ((raw >> 8) & 0xFF); // Swap byte order
}

static int ads1115_read_channel(struct i2c_client *client, int pga, int channel)
{
    int ret = ads1115_configure(client, pga, channel);
    if (ret < 0)
        return ret;

    return ads1115_read_conversion(client);
}

static long ads1115_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int channel = -1;
    int value;
    int err;

    switch (cmd) {
        case ADC1_R: channel = 0; break;
        case ADC2_R: channel = 1; break;
        case ADC3_R: channel = 2; break;
        case ADC4_R: channel = 3; break;
        default: return -EINVAL;
    }

    value = ads1115_read_channel(ads1115_client, 1, channel + 4); // AINx vs GND
    if (value < 0)
        return value;

    err = copy_to_user((int __user *)arg, &value, sizeof(value));
    return err ? -EFAULT : 0;
}

static struct file_operations fops = {
    .unlocked_ioctl = ads1115_ioctl,
    .owner = THIS_MODULE,
};

static int ads1115_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    ads1115_client = client;

    major_number = register_chrdev(0, DRIVER_NAME, &fops);
    if (major_number < 0)
        return major_number;

    ads1115_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(ads1115_class)) {
    unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(ads1115_class);
    }

    ads1115_device = device_create(ads1115_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(ads1115_device)) {
        class_destroy(ads1115_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(ads1115_device);
    }

    pr_info("ADS1115: device initialized at 0x%02x\n", client->addr);
    return 0;
}

static int ads1115_remove(struct i2c_client *client)
{
    device_destroy(ads1115_class, MKDEV(major_number, 0));
    class_unregister(ads1115_class);
    class_destroy(ads1115_class);
    unregister_chrdev(major_number, DRIVER_NAME);
    pr_info("ADS1115: device removed\n");
    return 0;
}

static const struct of_device_id ads1115_dt_ids[] = {
    { .compatible = "mycompany,ads1115" },
    { .compatible = "ti,ads1115" },
    { }
};
MODULE_DEVICE_TABLE(of, ads1115_dt_ids);

static const struct i2c_device_id ads1115_id[] = {
    { "ads1115", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, ads1115_id);

static struct i2c_driver ads1115_driver = {
    .driver = {
        .name = "ads1115",
        .of_match_table = ads1115_dt_ids,
    },
    .probe = ads1115_probe,
    .remove = ads1115_remove,
    .id_table = ads1115_id,
};

static int __init ads1115_init(void)
{
    int ret = i2c_add_driver(&ads1115_driver);
    if (ret)
        pr_err("ADS1115: failed to register I2C driver (%d)\n", ret);
    else
        pr_info("ADS1115: I2C driver registered\n");
    return ret;
}

static void __exit ads1115_exit(void)
{
    pr_info("ADS1115: driver exit\n");
    i2c_del_driver(&ads1115_driver);
}

module_init(ads1115_init);
module_exit(ads1115_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bill");
MODULE_DESCRIPTION("Minimal ADS1115 I2C Driver with ioctl support");
