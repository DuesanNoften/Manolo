#include <stdio.h>
#include <stdlib.h>

void send_message_to_kernel(const char *message) {
    FILE *fp;

    // Open the sysfs file for writing (where the kernel module listens for messages)
    fp = fopen("/sys/class/cdc_acm/message", "w");
    if (!fp) {
        perror("Failed to open sysfs file");
        return;
    }

    // Write the message to the sysfs file
    fprintf(fp, "%s", message);

    // Close the file
    fclose(fp);

    printf("Message sent to kernel: %s\n", message);
}
