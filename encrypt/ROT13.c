#include "ROT13.h"

void applyROT13(char *message) {
    char *ptr = message;
    while (*ptr != '\0') {
        char c = *ptr;
        if ((c >= 'a' && c <= 'z')) {
            *ptr = (((c - 'a') + 13) % 26) + 'a';
        } else if ((c >= 'A' && c <= 'Z')) {
            *ptr = (((c - 'A') + 13) % 26) + 'A';
        }
        ptr++;
    }
}