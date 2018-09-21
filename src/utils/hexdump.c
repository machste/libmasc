#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#include <masc/utils.h>


void hexdump(const void *data, size_t size) {
    char ascii[17];
    size_t i, j;
    ascii[16] = '\0';
    for (i = 0; i < size; i++) {
        uint8_t c = ((uint8_t *)data)[i];
        if (i % 16 == 0) {
            printf("%p: ", data + i);
        }
        printf("%02X ", c);
        if (isprint(c)) {
            ascii[i % 16] = c;
        } else {
            ascii[i % 16] = '.';
        }
        if ((i + 1) % 8 == 0 || i + 1 == size) {
            printf(" ");
            if ((i + 1) % 16 == 0) {
                printf("|  %s \n", ascii);
            } else if (i + 1 == size) {
                ascii[(i + 1) % 16] = '\0';
                if ((i + 1) % 16 <= 8) {
                    printf(" ");
                }
                for (j = (i + 1) % 16; j < 16; j++) {
                    printf("   ");
                }
                printf("|  %s \n", ascii);
            }
        }
    }
}
