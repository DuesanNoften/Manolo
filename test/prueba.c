#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

#define VENDOR_ID 0x2341  // Replace with your Arduino's vendor ID
#define PRODUCT_ID 0x0043 // Replace with your Arduino's product ID

int main() {
    libusb_device_handle *handle;
    libusb_context *ctx = NULL;
    int r;
    unsigned char command = '1'; // Command to turn on the LED
    int actual_length;

    r = libusb_init(&ctx);
    if (r < 0) {
        fprintf(stderr, "Failed to initialize libusb\n");
        return 1;
    }

    libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_DEBUG);

    handle = libusb_open_device_with_vid_pid(ctx, VENDOR_ID, PRODUCT_ID);
    if (!handle) {
        fprintf(stderr, "Cannot open device\n");
        libusb_exit(ctx);
        return 1;
    }

    if (libusb_kernel_driver_active(handle, 0) == 1) {
        libusb_detach_kernel_driver(handle, 0);
    }

    r = libusb_claim_interface(handle, 0);
    if (r < 0) {
        fprintf(stderr, "Cannot claim interface\n");
        libusb_close(handle);
        libusb_exit(ctx);
        return 1;
    }

    // Use the correct Bulk OUT endpoint (0x04)
    r = libusb_bulk_transfer(handle, 0x04, &command, 1, &actual_length, 5000);
    if (r == 0 && actual_length == 1) {
        printf("Command sent successfully\n");
    } else {
        fprintf(stderr, "Error in bulk transfer: %d\n", r);
    }

    libusb_release_interface(handle, 0);
    libusb_close(handle);
    libusb_exit(ctx);

    return 0;
}
