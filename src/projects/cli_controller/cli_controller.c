#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _MSC_VER
#include <conio.h>
#endif
#include "eve.h"
#include "hw_api.h"

#define FIFO_PATH "/tmp/eve_pipe"
#define MAX_LINES 20
#define MAX_LENGTH 500

#define DEFAULT_FONT 30
#define DEFAULT_COLOR_R 255
#define DEFAULT_COLOR_G 255
#define DEFAULT_COLOR_B 255

#define MAX_STATIC_TEXTS 2000

bool isEveInitialized = false;

typedef struct {
    int x, y, font;
    uint8_t r, g, b;
    char text[MAX_LENGTH];
} StaticText;

StaticText staticTexts[MAX_STATIC_TEXTS];
int staticTextCount = 0;

uint16_t x = 0, y = 0;
uint16_t font = 15, options = 0;
uint8_t r = 255, g = 255, b = 255;
int charWidth;

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

int GetCharWidth(uint16_t font_size, char ch) {
    int baseWidth = font_size * 0.5;

    if (ch >= 'A' && ch <= 'Z' ) {
        return baseWidth * 1.5;
    }

    if (ch == 'M') {
        return baseWidth * 2;
    }

    if ( ch == 'W') {
        return baseWidth * 3;
    }

    if (ch == 'I') {
        return baseWidth * 0.1;
    }

    if (ch == 'i' || ch == 'j' || ch == 'l' || ch == '!' || ch == '|' || ch == '.' || ch == ',' || ch == '\'' || ch == '\"') {
        return baseWidth * 0.5;
    }

    if (ch == 'f'|| ch == 't') {
        return baseWidth * 0.6;
    }

    if (ch == 'r') {
        return baseWidth * 0.7;
    }

    if (ch == 'b' || ch == 'p' || ch == 'v') {
        return baseWidth * 1.2;
    }

    if (ch == 'm' || ch == 'w') {
        return baseWidth * 1.5;
    }

    if (ch == ' ') {
        return baseWidth;
    }

    return baseWidth * 1.0;
}


int GetFontHeight() {
    return 30;
}


int GetTextWidth(const char* text, int font) {
    int width = 0;
    while (*text) {
        width += GetCharWidth(font, *text);
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

bool is_valid_utf8(const char *str) {
    const uint8_t *bytes = (const uint8_t *)str;
    while (*bytes) {
        if ((*bytes & 0x80) == 0) {
            // ASCII
            bytes++;
        } else if ((*bytes & 0xE0) == 0xC0) {
            // 2-byte
            if ((bytes[1] & 0xC0) != 0x80) return false;
            bytes += 2;
        } else if ((*bytes & 0xF0) == 0xE0) {
            // 3-byte
            if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80) return false;
            bytes += 3;
        } else if ((*bytes & 0xF8) == 0xF0) {
            // 4-byte
            if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80 || (bytes[3] & 0xC0) != 0x80) return false;
            bytes += 4;
        } else {
            return false;
        }
    }
    return true;
}



void AddStaticText(int x, int y, int font, uint8_t r, uint8_t g, uint8_t b, const char* text) {
    if (staticTextCount >= MAX_STATIC_TEXTS) {
        printf("Maximum number of static texts reached\n");
        return;
    }

    if (!is_valid_utf8(text)) {
        printf("Error during add static text: invalid UTF-8 encoding\n");
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

void parse_ansi(const char* buffer) {
    const char *ptr = buffer;

    printf("Parsing ANSI: '%s'\n", buffer);

    while (*ptr) {
        if (*ptr == '\n') {
            y += GetFontHeight(font);
            x = 0;
            ptr++;
            continue;
        }

        if (*ptr == '\r') { // Carriage return
            x = 0;
            ptr++;
            continue;
        }

        if (*ptr == '\b') { // Backspace
            if (x > 0) {
                x -= GetCharWidth(font, ' '); // Move the cursor to the left
            }
            ptr++;
            continue;
        }
        
        if (*ptr == '^' && *(ptr + 1) == '[') { // ESC
            ptr += 2;

            if (*ptr == '(' && *(ptr + 1) == 'B') { // ESC ( B - Set character set to ASCII
                ptr += 2;
                printf("Set character set to ASCII\n");
                continue;
            }

            if (*ptr == '[') {
                ptr++;
                char seq[32] = {0};
                int i = 0;

                while (isdigit(*ptr) || *ptr == ';' || *ptr == '?') {
                    if (i < (int)sizeof(seq) - 1) {
                        seq[i++] = *ptr;
                    }
                    ptr++;
                }
                seq[i] = '\0';

                if (*ptr == 'H') {  // `ESC [ H` - move cursor to (0,0)
                    x = 0;
                    y = 0;
                } else if (*ptr == 'J') {  // `ESC [ n J` - clearing the screen
                    int code = atoi(seq);
                    if (code == 2) {
                        ClearScreen(); // Full screen clearance
                        PrepareSceen();
                    }
                } else if (*ptr == 'r') { // `ESC [ t;b r` - setting the scroll area
                    int top = 0, bottom = 0;
                    sscanf(seq, "%d;%d", &top, &bottom);
                    printf("Set scroll region: %d-%d\n", top, bottom);
                } else if (*ptr == 'm') { // `ESC [ n m` - colour and style
                    char* token = strtok(seq, ";");
                    while (token) {
                        int code = atoi(token);
                        switch (code) {
                            case 0:  r = 255; g = 255; b = 255; options = 0; break; // Reset
                            case 1: break;   // Greasy
                            case 4: break;   // Underlined
                            case 7: break;   // Inverse
                            case 30: r = 0; g = 0; b = 0; break;   // Black
                            case 31: r = 255; g = 0; b = 0; break; // Red
                            case 32: r = 0; g = 255; b = 0; break; // Green
                            case 33: r = 255; g = 255; b = 0; break; // Yellow
                            case 34: r = 0; g = 0; b = 255; break; // Blue
                            case 35: r = 255; g = 0; b = 255; break; // Magenta
                            case 36: r = 0; g = 255; b = 255; break; // Blue
                            case 37: r = 255; g = 255; b = 255; break; // White
                            case 39: r = 255; g = 255; b = 255; break; // Resetting the front colour
                            case 49: break; // Reset background colour (ignore)
                        }
                        token = strtok(NULL, ";");
                    }
                } else if (*ptr == 'h') { // `ESC [ ? n h` - activation of flags
                    if (strcmp(seq, "?1049") == 0) {
                        printf("Enable alternate screen buffer\n");
                    } else if (strcmp(seq, "?25") == 0) {
                        printf("Show cursor\n");
                    } else if (strcmp(seq, "?1006;1000") == 0) {
                        printf("Enable mouse tracking\n");
                    }
                } else if (*ptr == 'l') { // `ESC [ ? n l` - switching off the flags
                    if (strcmp(seq, "?25") == 0) {
                        printf("Hide cursor\n");
                    } else if (strcmp(seq, "4") == 0) {
                        // Deactivating underscores
                    }
                }
                ptr++;
                continue;
            }
        }

        // A symbol
        char text[2] = {*ptr, '\0'};

        charWidth = GetCharWidth(font, *ptr);

        if (x + charWidth >= Display_Width()) {
            x = 0;
            y += GetFontHeight();
        }

        if (y >= Display_Height()) { // TODO : Scroll
            ClearScreen();
            PrepareSceen();
            x = 0; y = 0;
        }

        AddStaticText(x, y, font, r, g, b, text);
        printf("Adding text: '%s' at %d, %d\n", text, x, y);
        x += charWidth;
        ptr++;
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

            parse_ansi(buffer);
            
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
