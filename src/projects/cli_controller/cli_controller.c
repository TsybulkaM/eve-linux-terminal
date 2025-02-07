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

#define DEFAULT_FONT 30
#define DEFAULT_COLOR_R 255
#define DEFAULT_COLOR_G 255
#define DEFAULT_COLOR_B 255

#define MAX_STATIC_TEXTS 400
#define MAX_STATIC_TEXTS 400

bool isEveInitialized = false;

typedef struct {
    int x, y, font;
    uint8_t r, g, b;
    char text[MAX_LENGTH];
} StaticText;

StaticText staticTexts[MAX_STATIC_TEXTS];
int staticTextCount = 0;

int InitializeScreen(int fd) {
    while (!check_ftdi_device(0x1b3d, 0x0200)) {
        printf("Monitor disconnected! Waiting...\n");
        isEveInitialized = false;
        sleep(1);
    }

    if (!isEveInitialized) {
        EVE_Init(DEMO_DISPLAY, DEMO_BOARD, DEMO_TOUCH) <= 1;
        isEveInitialized = true;
        printf("Monitor connected! Initializing EVE...\n");
    }

    return OpenPipe();
}

int OpenPipe() {
    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) {
        perror("Error opening FIFO");
        sleep(1);
    }

    return fd;
}

int GetCharWidth(uint16_t font) {
    return 16;
}


int GetFontHeight() {
    return 35;
}


int GetTextWidth(const char* text, int font) {
    int width = 0;
    while (*text) {
        width += GetCharWidth(font);
        text++;
    }
    return width;
}


void DrawStaticTexts() {
    for (int i = 0; i < staticTextCount; i++) {
        Send_CMD(COLOR_RGB(
            staticTexts[i].r, 
            staticTexts[i].g, 
            staticTexts[i].b
        ));
        
        Cmd_Text(
            staticTexts[i].x, staticTexts[i].y, 
            staticTexts[i].font, 0, 
            staticTexts[i].text
        );
    }

    Send_CMD(DISPLAY());
    Send_CMD(CMD_SWAP);
    UpdateFIFO();
    Wait4CoProFIFOEmpty();
}


void AddStaticText(int x, int y, int font, uint8_t r, uint8_t g, uint8_t b, const char* text) {
    if (staticTextCount >= MAX_STATIC_TEXTS) {
        printf("Maximum number of static texts reached\n");
        return;
    }

    if (font > 32 || font < 15) {
        printf("Error during add static text: invalid font size\n");
        return;
    }

    if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
        printf("Error during add static text: invalid color\n");
        return;
    }

    if (y + GetFontHeight(DEFAULT_FONT) >= Display_Height()) {
        y = Display_Height() - GetFontHeight(DEFAULT_FONT);
    }

    if (x + GetTextWidth(text, font) >= Display_Width()) {
        x = Display_Width() - GetTextWidth(text, font);
    }

    staticTexts[staticTextCount].x = x;
    staticTexts[staticTextCount].y = y;
    staticTexts[staticTextCount].font = font;
    staticTexts[staticTextCount].r = r;
    staticTexts[staticTextCount].g = g;
    staticTexts[staticTextCount].b = b;
    strncpy(staticTexts[staticTextCount].text, text, MAX_LENGTH);
    staticTextCount++;
}


void DisplayText(int x, int y, const char* text) {
    int lineHeight = GetFontHeight(DEFAULT_FONT);
    int yOffset = 0;
    int maxWidth = Display_Width();
    char line[256] = "";
    char tempLine[256] = "";

    Send_CMD(COLOR_RGB(
        DEFAULT_COLOR_R, 
        DEFAULT_COLOR_G, 
        DEFAULT_COLOR_B
    ));

    if (x < 0) x = 0;
    if (y < 0) y = 0;

    int lineLength = 0;
    int currentWidth = 0;

    for (int i = 0; text[i] != '\0'; i++) {
        tempLine[lineLength] = text[i];
        tempLine[lineLength + 1] = '\0';

        int tempWidth = GetTextWidth(tempLine, DEFAULT_FONT);

        if (x + tempWidth > maxWidth) {
            Cmd_Text(x, y + yOffset, DEFAULT_FONT, 0, line);
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
        Cmd_Text(x, y + yOffset, DEFAULT_FONT, 0, line);
    }
}


void DisplayFormatText(int x, int y, int font, uint8_t r, uint8_t g, uint8_t b, const char* text) {
    int lineHeight = GetFontHeight(font);
    int yOffset = 0;
    int maxWidth = Display_Width();
    char line[256] = "";
    char tempLine[256] = "";

    Send_CMD(COLOR_RGB(r, g, b));

    if (x < 0) x = 0;
    if (y < 0) y = 0;

    int lineLength = 0;
    int currentWidth = 0;

    for (int i = 0; text[i] != '\0'; i++) {
        tempLine[lineLength] = text[i];
        tempLine[lineLength + 1] = '\0';

        int tempWidth = GetTextWidth(tempLine, font);

        if (x + tempWidth > maxWidth) {
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
}

void PrepareSceen() {
    Send_CMD(CMD_DLSTART);
    Send_CMD(CLEAR_COLOR_RGB(0, 0, 0));
    Send_CMD(CLEAR(1, 1, 1));
}

void Display() {
    Send_CMD(DISPLAY());
    Send_CMD(CMD_SWAP);
    UpdateFIFO();
    Wait4CoProFIFOEmpty();
}

void ClearScreen(void) {
    Display();
    staticTextCount = 0;
}

void ProcessCommand(char* buffer) {
    int x, y, font;
    uint8_t r, g, b;
    char text[MAX_LENGTH];

    if (sscanf(buffer, "custText %d %d %d %hhu %hhu %hhu %[^\n]", &x, &y, &font, &r, &g, &b, text) == 7) {
        printf("Received text command: x=%d, y=%d, font=%d, color=(%d, %d, %d), text=%s\n", x, y, font, r, g, b, text);
        DisplayFormatText(x, y, font, r, g, b, text);
    } else if (sscanf(buffer, "text %d %d %[^\n]", &x, &y, text) == 3) {
        printf("Received text command: x=%d, y=%d, text=%s\n", x, y, text);
        DisplayText(x, y, text);
    } else if (sscanf(buffer, "staticText %d %d %d %hhu %hhu %hhu %[^\n]", &x, &y, &font, &r, &g, &b, text) == 7) {
        printf("Received static text command: static=%d x=%d, y=%d, font=%d, color=(%d, %d, %d), text=%s\n", staticTextCount, x, y, font, r, g, b, text);
        AddStaticText(x, y, font, r, g, b, text);
    } else if (strcmp(buffer, "clear\n") == 0) {
        printf("Received clear command\n");
        ClearScreen();
    } else if (strcmp(buffer, "exit\n") == 0) {
        printf("Received exit command\n");
        HAL_Close();
        exit(0);
    } else {
        printf("Unknown command: %s\n", buffer);
    }
}

void ListenToFIFO() {
    char buffer[512];

    int fd = -1;

    while (1) {
        int fd = InitializeScreen(fd);

        ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);

        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';

            PrepareSceen();

            char *command = strtok(buffer, "&");
            while (command != NULL) {
                while (*command == ' ') command++;
                printf("Processing: '%s'\n", command);
                ProcessCommand(command);
                command = strtok(NULL, "&");
            }
            
            DrawStaticTexts();

        } else if (bytesRead == 0) {
            printf("FIFO closed, reopening...\n");
            sleep(1);
        } else if (bytesRead == -1) {
            printf("FIFO error, reopening...\n");
            sleep(1);
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
    
    ListenToFIFO();

    HAL_Close();
    return 0;
}
