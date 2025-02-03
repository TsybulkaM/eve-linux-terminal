#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <conio.h>
#endif
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


int GetCharWidth(char c, uint16_t font) {
    /*if (font < 16) return 8;
    if (font >= 16 && font <= 31) {
        static const int fontWidths[] = {8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38};
        return fontWidths[font - 16];
    }*/
    return 17;
}


int GetTextWidth(const char* text, int font) {
    int width = 0;
    while (*text) {
        width += GetCharWidth(*text, font);
        text++;
    }
    return width;
}


int GetFontHeight() {
    return 30;
}


void DisplayText(int x, int y, int font, const char* text) {
    int lineHeight = GetFontHeight(font);
    int yOffset = 0;
    int maxWidth = Display_Width();
    char line[256] = "";
    char tempLine[256] = "";

    Send_CMD(CMD_DLSTART);
    Send_CMD(VERTEXFORMAT(0));
    Send_CMD(COLOR_RGB(255, 255, 255));
    Send_CMD(CLEAR(1, 1, 1));

    if (x < 0) x = 0;
    if (y < 0) y = 0;

    int lineLength = 0;
    int currentWidth = 0;

    for (int i = 0; text[i] != '\0'; i++) {
        tempLine[lineLength] = text[i];
        tempLine[lineLength + 1] = '\0';

        int tempWidth = GetTextWidth(tempLine, font);

        if (y + tempWidth > maxWidth) {
            Cmd_Text(x, y + yOffset, font, 0, line);
            yOffset += lineHeight;

            line[0] = text[i];
            line[1] = '\0';
            lineLength = 1;
        } else {
            line[lineLength] = text[i];
            line[lineLength + 1] = '\0';
            lineLength++;
        }
    }

    if (lineLength > 0) {
        Cmd_Text(x, y + yOffset, font, 0, line);
    }

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
        int front = atoi(argv[4]);
        const char* text = argv[5];
        DisplayText(x, y, front, text);
    } else {
        printf("ERROR: Invalid arguments.\n");
    }
}


int main(int argc, char *argv[]) {
    if (EVE_Init(DEMO_DISPLAY, DEMO_BOARD, DEMO_TOUCH) <= 1) {
        printf("ERROR: Eve not detected.\n");
        return -1;
    }

    ProcessArguments(argc, argv);

    HAL_Close();
    return 0;
}
