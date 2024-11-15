#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

#define LED_PIN 13

static struct usb_device *device;

static int arduino_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    printk(KERN_INFO "Arduino device (%04X:%04X) plugged\n", id->idVendor, id->idProduct);
    device = interface_to_usbdev(interface);

    // Turn on LED
    usb_control_msg(device, usb_sndctrlpipe(device, 4), USB_REQ_SET_CONFIGURATION, USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE, LED_PIN, 1, NULL, 0, 0);

    return 0;
}

static void arduino_disconnect(struct usb_interface *interface)
{
    printk(KERN_INFO "Arduino device removed\n");
    device = NULL;
}

static struct usb_device_id arduino_table[] = {
    { USB_DEVICE(0x2341, 0x0043) }, // Arduino Uno R3
    {} /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, arduino_table);

static struct usb_driver arduino_driver = {
    .name = "arduino_driver",
    .id_table = arduino_table,
    .supports_autosuspend = 1,
    .probe = arduino_probe,
    .disconnect = arduino_disconnect,
};

static int __init arduino_init(void)
{
    return usb_register(&arduino_driver);
}

static void __exit arduino_exit(void)
{
    usb_deregister(&arduino_driver);
}

module_init(arduino_init);
module_exit(arduino_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("GitHub Copilot");
MODULE_DESCRIPTION("Arduino Uno R3 LED Control Driver");