#include "eveld.h"

bool isEveInitialized = false;
bool tmp_flag_nl = false;

StaticText actual_word = {
    .x = 0,
    .y = 0,
    .font = DEFAULT_FONT,
    .r = DEFAULT_COLOR_R,
    .g = DEFAULT_COLOR_G,
    .b = DEFAULT_COLOR_B,
    .line = 0,
    .bg = DEFAULT_BG_COLOR,
};
uint16_t actual_word_len = 0;
uint32_t actual_word_width = 0;
uint16_t staticTextCount = 0;
uint16_t savedStaticTextCount = 0;
uint16_t saved_x = 0;
uint16_t saved_y = 0;

StaticText staticTexts[MAX_STATIC_TEXTS];
StaticText savedStaticTexts[MAX_STATIC_TEXTS];

int main() {
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
