#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _MSC_VER
#include <conio.h>
#endif
#include "eve.h"
#include "hw_api.h"

#define FIFO_PATH "/tmp/eve_pipe"
#define MAX_LINES 20
#define MAX_LENGTH 256

int GetCharWidth(char c, uint16_t font) {
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
    Wait4CoProFIFOEmpty();
    HAL_Delay(1000);
}


void ClearScreen(void) {
    Send_CMD(CMD_DLSTART);
    Send_CMD(CLEAR_COLOR_RGB(0, 0, 0));
    Send_CMD(CLEAR(1, 1, 1));
    Send_CMD(DISPLAY());
    Send_CMD(CMD_SWAP);
    UpdateFIFO();
    Wait4CoProFIFOEmpty();
    HAL_Delay(1000);
}

void ListenToFIFO() {
    char buffer[512];

    while (1) {
        int fd = open(FIFO_PATH, O_RDONLY);
        if (fd == -1) {
            perror("Error opening FIFO");
            sleep(1); // Подождем перед повторной попыткой
            continue;
        }

        ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);

        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';

            int x, y, font;
            char text[MAX_LENGTH];

            if (sscanf(buffer, "text %d %d %d %[^\n]", &x, &y, &font, text) == 4) {
                printf("Received text command: x=%d, y=%d, font=%d, text=%s\n", x, y, font, text);
                DisplayText(x, y, font, text);
            } else if (strcmp(buffer, "clear\n") == 0) {
                printf("Received clear command\n");
                ClearScreen();
            } else if (strcmp(buffer, "exit\n") == 0) {
                printf("Received exit command\n");
                break;
            } else {
                printf("Unknown command: %s\n", buffer);
            }
        } else if (bytesRead == 0) {
            printf("FIFO closed, reopening...\n");
            sleep(1); // Подождем перед повторной попыткой
        } else if (bytesRead == -1) {
            perror("Error reading from FIFO");
            sleep(1); // Подождем перед повторной попыткой
        }
    }
}


int main() {
    if (access(FIFO_PATH, F_OK) == -1) {
        if (mkfifo(FIFO_PATH, 0666) == -1) {
            perror("Error creating FIFO");
            return -1;
        }
    }

    if (EVE_Init(DEMO_DISPLAY, DEMO_BOARD, DEMO_TOUCH) <= 1) {
        printf("ERROR: Eve not detected.\n");
        return -1;
    }

    ClearScreen();
    ListenToFIFO();

    HAL_Close();
    return 0;
}
