#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

static struct usb_device *device = NULL;
static struct tty_driver *tty_driver;
static struct class *cdc_acm_class;

// This function sends a message to the connected USB device
// the message is sent over the bulk OUT endpoint of the USB device
static int cdc_acm_send_message(const char *message) {
    int retval;
    int message_length = strlen(message);

    if (!device) {
        printk(KERN_ERR "cdc_acm: No device connected\n");
        return -ENODEV;
    }

    retval = usb_bulk_msg(device, usb_sndbulkpipe(device, 0x04), (void *)message, message_length, &message_length, 15000);
    if (retval) {
        printk(KERN_ERR "cdc_acm: Failed to send message, error %d\n", retval);
        return retval;
    }

    printk(KERN_INFO "cdc_acm: Message sent successfully\n");
    return 0;
}

// Define a class attribute for the message file in sysfs
// this attribute is write-only, meaning that it can be written 
// to but not read from
static ssize_t message_store(const struct class *class, const struct class_attribute *attr, const char *buf, size_t count) {
    cdc_acm_send_message(buf);
    return count;
}

// Define a class attribute for the message file in sysfs
CLASS_ATTR_WO(message);

// This function is called by the USB core when a device matching
// the USB_DEVICE macro is connected to the system.
// it is responsible for initializing the device and setting up any
static int cdc_acm_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    if (device) {
        printk(KERN_ERR "cdc_acm: Device already in use\n");
        return -EBUSY;
    }
    device = interface_to_usbdev(interface);
    printk(KERN_INFO "cdc_acm: Arduino Uno (%04X:%04X) plugged\n", id->idVendor, id->idProduct);
    cdc_acm_send_message("1");   
    return 0;
}

// This function is called by the USB core when a device matching
// the USB_DEVICE macro is disconnected from the system.
// It is responsible for cleaning up any resources allocated during probe.
static void cdc_acm_disconnect(struct usb_interface *interface) {
    printk(KERN_INFO "cdc_acm: Arduino Uno removed\n");
}

// Define the USB device ID table
static struct usb_device_id cdc_acm_table[] = {
    { USB_DEVICE(0x2341, 0x0043) }, // Arduino Uno
    {}
};
MODULE_DEVICE_TABLE(usb, cdc_acm_table);

// Define the USB driver structure
// this structure is used by the USB core to manage the driver
static struct usb_driver cdc_acm_driver = {
    .name = "cdc_acm",
    .id_table = cdc_acm_table,
    .probe = cdc_acm_probe,
    .disconnect = cdc_acm_disconnect,
};

// This function is called when data is written to the tty device
// it is responsible for processing the data and sending it to the USB device
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

// This function is called when the tty device is opened
// it is responsible for initializing the device
static int cdc_acm_open(struct tty_struct *tty, struct file *file) {
    printk(KERN_INFO "cdc_acm: tty open\n");
    return 0;
}

// This function is called when the tty device is closed
// it is responsible for cleaning up any resources allocated during open
static void cdc_acm_close(struct tty_struct *tty, struct file *file) {
    printk(KERN_INFO "cdc_acm: tty close\n");
}

// This function returns the number of bytes that can be written to the tty device
static unsigned int cdc_acm_write_room(struct tty_struct *tty) {
    return 255;
}

// Define the tty operations structure
static const struct tty_operations cdc_acm_ops = {
    .open = cdc_acm_open,
    .close = cdc_acm_close,
    .write = cdc_acm_write,
    .write_room = cdc_acm_write_room,
};

// This function is called when the module is loaded
static int __init cdc_acm_init(void) {
    int result;

    printk(KERN_INFO "cdc_acm: Initializing the cdc_acm module\n");
    // Allocate a tty driver structure
    // this structure is used by the tty core to manage the tty devices
    tty_driver = tty_alloc_driver(1, TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV);
    if (IS_ERR(tty_driver)) {
        printk(KERN_ERR "cdc_acm: Failed to allocate tty driver\n");
        return PTR_ERR(tty_driver);
    }
    // Initialize the tty driver structure
    tty_driver->owner = THIS_MODULE;
    tty_driver->driver_name = "cdc_acm";
    tty_driver->name = "ttyACM1";
    tty_driver->major = 0;
    tty_driver->type = TTY_DRIVER_TYPE_SERIAL;
    tty_driver->subtype = SERIAL_TYPE_NORMAL;
    tty_driver->init_termios = tty_std_termios;
    tty_set_operations(tty_driver, &cdc_acm_ops);

    // Register the tty driver with the tty core
    result = tty_register_driver(tty_driver);
    if (result) {
        printk(KERN_ERR "cdc_acm: Failed to register tty driver\n");
        tty_driver_kref_put(tty_driver);
        return result;
    }

    // Register the USB driver with the USB core
    result = usb_register(&cdc_acm_driver);
    if (result) {
        printk(KERN_ERR "cdc_acm: usb_register failed. Error number %d\n", result);
        tty_unregister_driver(tty_driver);
        tty_driver_kref_put(tty_driver);
        return result;
    }

    // Create a sysfs class for the cdc_acm module
    cdc_acm_class = class_create("cdc_acm");
    if (IS_ERR(cdc_acm_class)) {
        printk(KERN_ERR "cdc_acm: Failed to create class\n");
        return PTR_ERR(cdc_acm_class);
    }

    // Create a sysfs file for the message attribute
    result = class_create_file(cdc_acm_class, &class_attr_message);
    if (result) {
        printk(KERN_ERR "cdc_acm: Failed to create sysfs file\n");
        class_destroy(cdc_acm_class);
        return result;
    }

    return 0;
}

// This function is called when the module is unloaded
static void __exit cdc_acm_exit(void) {
    printk(KERN_INFO "cdc_acm: Exiting the cdc_acm module\n");
    // Unregister the USB driver from the USB core
    usb_deregister(&cdc_acm_driver);
    tty_unregister_driver(tty_driver);
    tty_driver_kref_put(tty_driver);
    class_remove_file(cdc_acm_class, &class_attr_message);
    class_destroy(cdc_acm_class);
}

module_init(cdc_acm_init);
module_exit(cdc_acm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DueSan");
MODULE_DESCRIPTION("Arduino Uno CDC-ACM modded driver");
MODULE_VERSION("1");
