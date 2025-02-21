#include "eveld.h"

uint32_t monitor_display_list_memory(void) {
    uint32_t current_ptr = rd32(RAM_CMD);
    uint32_t used = current_ptr - RAM_CMD;
    return used;
}

void check_display_list_memory(void) {
    uint32_t used = monitor_display_list_memory();
    printf("Display list memory used: %lu bytes\n", (unsigned long) used);
    if (used >= FT_DL_SIZE) {
        printf("Warning: Display list memory overflow detected!\n");
    }
}