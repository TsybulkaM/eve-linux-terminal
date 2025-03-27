#include "eveld.h"

bool LINE_FEED = false;
bool isEveInitialized = false;
bool mutex_second_buffer = false;
char breakdown_ansi[BD_ANSI_LEN] = {0}; 

StaticText actual_word = {
    .x = 0,
    .y = 0,
    .font = DEFAULT_FONT,
    .text_color = (Color){DEFAULT_COLOR_R, DEFAULT_COLOR_G, DEFAULT_COLOR_B},
    .bg_color = (Color){DEFAULT_COLOR_BG_R, DEFAULT_COLOR_BG_G, DEFAULT_COLOR_BG_B},
    .line = 0,
    .width = 0,
};

StaticText saved_word = {
    .x = 0,
    .y = 0,
    .font = DEFAULT_FONT,
    .text_color = (Color){DEFAULT_COLOR_R, DEFAULT_COLOR_G, DEFAULT_COLOR_B},
    .bg_color = (Color){DEFAULT_COLOR_BG_R, DEFAULT_COLOR_BG_G, DEFAULT_COLOR_BG_B},
    .line = 0,
    .width = 0,
};

uint16_t actual_word_len = 0;
uint16_t staticTextCount = 0;
uint16_t savedStaticTextCount = 0;

StaticText staticTexts[MAX_STATIC_TEXTS];
StaticText savedStaticTexts[MAX_STATIC_TEXTS];

int main() {
    DEBUG_PRINT("Привет\n");

    if (access(FIFO_PATH, F_OK) == -1) {
        if (mkfifo(FIFO_PATH, 0666) == -1) {
            perror("Error creating FIFO");
            return -1;
        }
    }
    
    ListenToFIFO();

    HAL_Close();
    return 0;
}
