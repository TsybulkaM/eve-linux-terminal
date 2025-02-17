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
#define MAX_LENGTH 512

#define DEFAULT_FONT 30
#define DEFAULT_COLOR_R 255
#define DEFAULT_COLOR_G 255
#define DEFAULT_COLOR_B 255

#define MAX_STATIC_TEXTS 160

bool isEveInitialized = false;

typedef struct {
    int x, y, font;
    uint8_t r, g, b;
    uint32_t bg;
    char text[MAX_LENGTH];
} StaticText;

StaticText staticTexts[MAX_STATIC_TEXTS];
int staticTextCount = 0;

uint16_t x = 0, y = 0;
uint16_t saved_x = 0, saved_y = 0;
uint16_t font = 16, options = 0;
uint8_t r = 255, g = 255, b = 255;
uint32_t bg = 0xFFFFFF;
int charWidth;

void PrepareScreen() {
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

int OpenPipe() {
    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) {
        perror("Error opening FIFO");
        sleep(1);
    }

    return fd;
}

int InitializeScreen(int fd) {
    while (!check_ftdi_device()) {
        printf("Monitor disconnected! Waiting...\n");
        isEveInitialized = false;
        sleep(1);
    }

    if (!isEveInitialized) {
        if (EVE_Init(DEMO_DISPLAY, DEMO_BOARD, DEMO_TOUCH) <= 1) {
            printf("EVE initialization failed\n");
            return -1;
        }
        isEveInitialized = true;
        printf("Monitor connected! Initializing EVE...\n");
    }

    return OpenPipe();
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

int GetFontHeight(int font) {
    return font;
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

        Cmd_BGcolor(staticTexts[i].bg);
        
        Cmd_Text(
            staticTexts[i].x, staticTexts[i].y, 
            staticTexts[i].font, 0, 
            staticTexts[i].text
        );
    }

    Display();
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

void AddStaticText(int x, int y, int font, uint8_t r, uint8_t g, uint8_t b, uint32_t bg, char* text) {
    if (staticTextCount >= MAX_STATIC_TEXTS) {
        printf("Maximum number of static texts reached\n");
        ClearScreen();
        PrepareScreen();
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
    staticTexts[staticTextCount].bg = bg;
    strncpy(staticTexts[staticTextCount].text, text, MAX_LENGTH);
    staticTextCount++;
}

void parse_ansi(const char* buffer) {
    const char *ptr = buffer;
    char word[MAX_LENGTH];
    int word_len = 0;
    bool is_nl = false;

    printf("Parsing ANSI: '%s'\n", buffer);

    while (*ptr) {

        if (y + GetFontHeight(font) >= Display_Height()) { // TODO : Scroll
            ClearScreen();
            PrepareScreen();
            x = 0; y = 0;
        }

        is_nl = x + GetTextWidth(word, font) >= Display_Width();

        if (*ptr == '\n' || *ptr == ' ' || *ptr == '\0' || *ptr == '\t' || is_nl) {
            if (word_len > 0) {
                word[word_len] = '\0';
                AddStaticText(x, y, font, r, g, b, bg, word);
                printf("DEBUG: Adding word: '%s' at %d, %d\n", word, x, y);
                printf("DEBUG: staticTextCount: %d\n", staticTextCount);
                x += GetTextWidth(word, font);
                word_len = 0;
            }

            if (*ptr == '\n' || is_nl) {
                y += GetFontHeight(font);
                x = 0;
            } else if (*ptr == ' ') {
                x += GetCharWidth(font, ' ');
            } else if (*ptr == '\t') {
                x += GetCharWidth(font, ' ') * 4;
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

                if (*ptr == 'H') {  // ESC [ H - move cursor to (0,0)
                    x = 0;
                    y = 0;
                } else if (*ptr == 'J') {  // ESC [ n J - clearing the screen
                    int code = atoi(seq);
                    if (code == 2) {
                        ClearScreen(); // Full screen clearance
                        PrepareScreen();
                    }
                } else if (*ptr == 'K') {  // ESC [ K - clear line
                    int code = atoi(seq);
                    if (code == 0) {
                        // Clear from cursor to end of line
                    } else if (code == 1) {
                        // Clear from beginning of line to cursor
                    } else if (code == 2) {
                        // Clear entire line
                    }
                } else if (*ptr == 'm') { // ESC [ n m - colour and style
                    char* token = strtok(seq, ";");
                    while (token) {
                        int code = atoi(token);
                        switch (code) {
                            case 0:  r = 255; g = 255; b = 255; bg = 0x000000 ; options = 0; break; // Reset
                            case 1: ; break;   // Bold
                            case 4: ; break;   // Underlined
                            case 7: ; break;   // Inverse
                            case 30: r = 0; g = 0; b = 0; break;   // Black
                            case 31: r = 255; g = 0; b = 0; break; // Red
                            case 32: r = 0; g = 255; b = 0; break; // Green
                            case 33: r = 255; g = 255; b = 0; break; // Yellow
                            case 34: r = 0; g = 0; b = 255; break; // Blue
                            case 35: r = 255; g = 0; b = 255; break; // Magenta
                            case 36: r = 0; g = 255; b = 255; break; // Cyan
                            case 37: r = 255; g = 255; b = 255; break; // White
                            case 39: r = 255; g = 255; b = 255; break; // Resetting the front colour
                            case 40: bg = 0x000000; break; // Black background
                            case 41: bg = 0xFF0000; break; // Red background
                            case 42: bg = 0x00FF00; break; // Green background
                            case 43: bg = 0xFFFF00; break; // Yellow background
                            case 44: bg = 0x0000FF; break; // Blue background
                            case 45: bg = 0xFF00FF; break; // Magenta background
                            case 46: bg = 0x00FFFF; break; // Cyan background
                            case 47: bg = 0xFFFFFF; break; // White background
                            case 49: bg = 0xFFFFFF; break; // Reset background
                        }
                        token = strtok(NULL, ";");
                    }
                } else if (*ptr == 'A') {  // ESC [ n A - move cursor up by n
                    int n = atoi(seq);
                    y -= n * GetFontHeight(font);
                } else if (*ptr == 'B') {  // ESC [ n B - move cursor down by n
                    int n = atoi(seq);
                    y += n * GetFontHeight(font);
                } else if (*ptr == 'C') {  // ESC [ n C - move cursor right by n
                    int n = atoi(seq);
                    x += n * GetCharWidth(font, ' ');
                } else if (*ptr == 'D') {  // ESC [ n D - move cursor left by n
                    int n = atoi(seq);
                    x -= n * GetCharWidth(font, ' ');
                } else if (*ptr == 'G') {  // ESC [ n G - move cursor to column n
                    int n = atoi(seq);
                    x = n * GetCharWidth(font, ' ');
                } else if (*ptr == 'd') {  // ESC [ n d - move cursor to line n
                    int n = atoi(seq);
                    y = n * GetFontHeight(font);
                } else if (*ptr == 's') {  // ESC [ s - save cursor position
                    saved_x = x;
                    saved_y = y;
                } else if (*ptr == 'u') {  // ESC [ u - restore cursor position
                    x = saved_x;
                    y = saved_y;
                } else if (*ptr == 'h') { // ESC [ ? n h - activation of flags
                    if (strcmp(seq, "?1049") == 0) {
                        printf("Enable alternate screen buffer\n");
                    } else if (strcmp(seq, "?25") == 0) {
                        printf("Show cursor\n");
                    } else if (strcmp(seq, "?1006;1000") == 0) {
                        printf("Enable mouse tracking\n");
                    }
                } else if (*ptr == 'l') { // ESC [ ? n l - switching off the flags
                    if (strcmp(seq, "?25") == 0) {
                        printf("Hide cursor\n");
                    } else if (strcmp(seq, "4") == 0) {
                        // Deactivating underscores
                    }
                } else if (*ptr == 'r') { // ESC [ t;b r - setting the scroll area
                    int top = 0, bottom = 0;
                    sscanf(seq, "%d;%d", &top, &bottom);
                    printf("Set scroll region: %d-%d\n", top, bottom);
                } else if (*ptr == 'L') {  // ESC [ n L - insert n lines
                    int n = atoi(seq);
                    // Insert n lines at current cursor position
                } else if (*ptr == 'M') {  // ESC [ n M - delete n lines
                    int n = atoi(seq);
                    // Delete n lines at current cursor position
                } else if (*ptr == 'P') {  // ESC [ n P - delete n characters
                    int n = atoi(seq);
                    // Delete n characters at current cursor position
                } else if (*ptr == 'S') {  // ESC [ n S - scroll up n lines
                    int n = atoi(seq);
                    // Scroll up n lines
                } else if (*ptr == 'T') {  // ESC [ n T - scroll down n lines
                    int n = atoi(seq);
                    // Scroll down n lines
                } else if (*ptr == 'X') {  // ESC [ n X - erase n characters
                    int n = atoi(seq);
                    // Erase n characters at current cursor position
                } else if (*ptr == 'Z') {  // ESC [ n Z - move cursor to previous tab stop
                    int n = atoi(seq);
                    // Move cursor to previous tab stop
                } else {
                    printf("Unknown ANSI sequence: ESC [ %s %c\n", seq, *ptr);
                }
                ptr++;
                continue;
            }
        }

        if (is_valid_utf8(ptr)) {
            word[word_len++] = *ptr;
        } else {
            word[word_len++] = ' ';
            printf("ERROR : during adding static text: invalid UTF-8 encoding\n");
            return;
        }

        ptr++;
    }

    if (word_len > 0) {
        word[word_len] = '\0';
        AddStaticText(x, y, font, r, g, b, bg, word);
        x += GetTextWidth(word, font);
        printf("DEBUG: Adding word: '%s' at %d, %d\n", word, x, y);
        printf("DEBUG: staticTextCount: %d\n", staticTextCount);
    }
}

void ListenToFIFO() {
    char buffer[MAX_LENGTH];

    int fd = -1;

    while (1) {
        fd = InitializeScreen(fd);

        ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);

        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';

            PrepareScreen();

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