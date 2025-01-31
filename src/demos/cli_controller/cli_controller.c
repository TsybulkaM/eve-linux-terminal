#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <conio.h>
#endif
#include "MONOSPACE821BT_64_ASTC.glyph.h"
#include "MONOSPACE821BT_64_ASTC.xfont.h"
#include "eve.h"
#include "hw_api.h"

void ClearScreen() {
    Send_CMD(CMD_DLSTART);
    Send_CMD(VERTEXFORMAT(0));
    Send_CMD(CLEAR_COLOR_RGB(0, 0, 0));
    Send_CMD(CLEAR(1, 1, 1));
    Send_CMD(DISPLAY());
    Send_CMD(CMD_SWAP);
    UpdateFIFO();
}

void DisplayText(int x, int y, const char* text) {
    Send_CMD(CMD_DLSTART);
    Send_CMD(VERTEXFORMAT(0));
    Send_CMD(CLEAR_COLOR_RGB(0, 0, 0));
    Send_CMD(CLEAR(1, 1, 1));
    Send_CMD(COLOR_RGB(255, 255, 255));

    Cmd_SetFont2(1, RAM_G, 0);
    Cmd_Text(x, y, 1, OPT_CENTER, text);
    Send_CMD(DISPLAY());
    Send_CMD(CMD_SWAP);
    UpdateFIFO();
}

void ProcessArguments(int argc, char *argv[]) {
    if (argc < 2) {
        printf("ERROR: Not enough arguments.\n");
        return;
    }

    if (strcmp(argv[1], "clear") == 0) {
        ClearScreen();
    } else if (strcmp(argv[1], "text") == 0 && argc >= 5) {
        int x = atoi(argv[2]);
        int y = atoi(argv[3]);
        const char* text = argv[4];
        DisplayText(x, y, text);
    } else {
        printf("ERROR: Invalid arguments.\n");
    }
}

int main(int argc, char *argv[]) {
    if (EVE_Init(DEMO_DISPLAY, DEMO_BOARD, DEMO_TOUCH) <= 1) {
        printf("ERROR: Eve not detected.\n");
        return -1;
    }

    StartCoProTransfer(RAM_G, 0);
    HAL_SPI_WriteBuffer((uint8_t *)&MONOSPACE821BT_64_ASTC_xfont, sizeof(MONOSPACE821BT_64_ASTC_xfont));
    HAL_SPI_Disable();

    StartCoProTransfer(RAM_G + 4096, 0);
    HAL_SPI_WriteBuffer((uint8_t *)&MONOSPACE821BT_64_ASTC_glyph, sizeof(MONOSPACE821BT_64_ASTC_glyph));
    HAL_SPI_Disable();

    ProcessArguments(argc, argv);

    HAL_Close();
    return 0;
}
