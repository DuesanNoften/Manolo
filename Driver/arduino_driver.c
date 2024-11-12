#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>

static struct usb_device *device;

static int arduino_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    device = interface_to_usbdev(interface);
    printk(KERN_INFO "Arduino device (%04X:%04X) plugged\n", id->idVendor, id->idProduct);
    return 0;
}

static void arduino_disconnect(struct usb_interface *interface) {
    printk(KERN_INFO "Arduino device removed\n");
}

static struct usb_device_id arduino_table[] = {
    { USB_DEVICE(0x2341, 0x0043) }, // Arduino Uno R3
    {} // Terminating entry
};
MODULE_DEVICE_TABLE(usb, arduino_table);

static struct usb_driver arduino_driver = {
    .name = "arduino_driver",
    .id_table = arduino_table,
    .probe = arduino_probe,
    .disconnect = arduino_disconnect,
};

static int __init arduino_init(void) {
    int result = usb_register(&arduino_driver);
    if (result)
        printk(KERN_ERR "usb_register failed. Error number %d", result);
    return result;
}

static void __exit arduino_exit(void) {
    usb_deregister(&arduino_driver);
}

module_init(arduino_init);
module_exit(arduino_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Arduino USB Driver");
