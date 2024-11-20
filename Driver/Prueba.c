#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/slab.h>

static struct usb_device *device = NULL;
static struct tty_driver *tty_driver;

static int cdc_acm_send_message(const char *message) {
    int retval;
    int message_length = strlen(message);

    if (!device) {
        printk(KERN_ERR "cdc_acm: No device connected\n");
        return -ENODEV;
    }

    retval = usb_bulk_msg(device, usb_sndbulkpipe(device, 0x04), (void *)message, message_length, &message_length, 10000);
    if (retval) {
        printk(KERN_ERR "cdc_acm: Failed to send message, error %d\n", retval);
        return retval;
    }

    printk(KERN_INFO "cdc_acm: Message sent successfully\n");
    return 0;
}

static int cdc_acm_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    if (device) {
        printk(KERN_ERR "cdc_acm: Device already in use\n");
        return -EBUSY;
    }
    device = interface_to_usbdev(interface);
    printk(KERN_INFO "cdc_acm: Arduino Uno (%04X:%04X) plugged\n", id->idVendor, id->idProduct);
    cdc_acm_send_message("1");
    device = NULL;
    printk(KERN_INFO "cdc_acm: Arduino Uno removed\n");
    return 0;
}

static void cdc_acm_disconnect(struct usb_interface *interface) {
    printk(KERN_INFO "cdc_acm: Arduino Uno removed\n");
}

static struct usb_device_id cdc_acm_table[] = {
    { USB_DEVICE(0x2341, 0x0043) }, // Arduino Uno
    {}
};
MODULE_DEVICE_TABLE(usb, cdc_acm_table);

static struct usb_driver cdc_acm_driver = {
    .name = "cdc_acm",
    .id_table = cdc_acm_table,
    .probe = cdc_acm_probe,
    .disconnect = cdc_acm_disconnect,
};

static ssize_t cdc_acm_write(struct tty_struct *tty, const unsigned char *buffer, size_t count) {
    char *cmd;
    char *msg;
    char *buf = kmalloc(count + 1, GFP_KERNEL);

    if (!buf) {
        printk(KERN_ERR "cdc_acm: Failed to allocate memory\n");
        return -ENOMEM;
    }

    memcpy(buf, buffer, count);
    buf[count] = '\0';

    cmd = strsep(&buf, " ");
    if (cmd && strcmp(cmd, "sendmsg") == 0) {
        msg = buf;
        if (msg) {
            cdc_acm_send_message(msg);
        } else {
            printk(KERN_ERR "cdc_acm: No message provided\n");
        }
    } else {
        printk(KERN_ERR "cdc_acm: Invalid command\n");
    }

    kfree(buf);
    return count;
}

static int cdc_acm_open(struct tty_struct *tty, struct file *file) {
    printk(KERN_INFO "cdc_acm: tty open\n");
    return 0;
}

static void cdc_acm_close(struct tty_struct *tty, struct file *file) {
    printk(KERN_INFO "cdc_acm: tty close\n");
}

static unsigned int cdc_acm_write_room(struct tty_struct *tty) {
    return 255;
}

static const struct tty_operations cdc_acm_ops = {
    .open = cdc_acm_open,
    .close = cdc_acm_close,
    .write = cdc_acm_write,
    .write_room = cdc_acm_write_room,
};

static int __init cdc_acm_init(void) {
    int result;

    printk(KERN_INFO "cdc_acm: Initializing the cdc_acm module\n");
    tty_driver = tty_alloc_driver(1, TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV);
    if (IS_ERR(tty_driver)) {
        printk(KERN_ERR "cdc_acm: Failed to allocate tty driver\n");
        return PTR_ERR(tty_driver);
    }

    tty_driver->owner = THIS_MODULE;
    tty_driver->driver_name = "cdc_acm";
    tty_driver->name = "ttyACM1";
    tty_driver->major = 0;
    tty_driver->type = TTY_DRIVER_TYPE_SERIAL;
    tty_driver->subtype = SERIAL_TYPE_NORMAL;
    tty_driver->init_termios = tty_std_termios;
    tty_set_operations(tty_driver, &cdc_acm_ops);

    result = tty_register_driver(tty_driver);
    if (result) {
        printk(KERN_ERR "cdc_acm: Failed to register tty driver\n");
        tty_driver_kref_put(tty_driver);
        return result;
    }

    result = usb_register(&cdc_acm_driver);
    if (result) {
        printk(KERN_ERR "cdc_acm: usb_register failed. Error number %d\n", result);
        tty_unregister_driver(tty_driver);
        tty_driver_kref_put(tty_driver);
        return result;
    }

    return 0;
}

static void __exit cdc_acm_exit(void) {
    printk(KERN_INFO "cdc_acm: Exiting the cdc_acm module\n");
    usb_deregister(&cdc_acm_driver);
    tty_unregister_driver(tty_driver);
    tty_driver_kref_put(tty_driver);
}

module_init(cdc_acm_init);
module_exit(cdc_acm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DueSan");
MODULE_DESCRIPTION("A simple USB CDC ACM driver");
MODULE_VERSION("0.1");