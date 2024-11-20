#include <stdio.h>
#include <stdlib.h>

void send_message_to_kernel(const char *message, int number) {
    FILE *fp;

    // Create a buffer large enough to hold the message and the integer
    char combined_message[256];  // Adjust size as needed

    // Combine the string message and the integer into one message
    sprintf(combined_message, "%s %d", message, number);  // Concatenates the string and integer

    // Open the sysfs file for writing (where the kernel module listens for messages)
    fp = fopen("/sys/class/cdc_acm/message", "w");
    if (!fp) {
        perror("Failed to open sysfs file");
        return;
    }

    // Write the combined message to the sysfs file
    fprintf(fp, "%s", combined_message);

    // Close the file
    fclose(fp);

    // Optionally print the message sent to the kernel for debugging
    printf("Message sent to kernel: %s\n", combined_message);
}

