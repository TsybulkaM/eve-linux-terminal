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
#include "logo.h"

#define DEBUG 1
#if DEBUG
    #define DEBUG_PRINT(fmt, ...) fprintf(stderr, "\x1B[35mDEBUG:\x1B[m " fmt , ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...)
#endif

#define INFO_PRINT(fmt, ...) fprintf(stderr, "\x1B[32mINFO:\x1B[m " fmt , ##__VA_ARGS__)
#define ERROR_PRINT(fmt, ...) fprintf(stderr, "\x1B[31mERROR:\x1B[m " fmt , ##__VA_ARGS__)

#define FIFO_PATH "/tmp/eve_pipe"
#define MAX_LINES 20
#define MAX_LENGTH 512

#define DEFAULT_FONT 16
#define DEFAULT_OPTION 16
#define DEFAULT_COLOR_R 255
#define DEFAULT_COLOR_G 255
#define DEFAULT_COLOR_B 255
#define DEFAULT_BG_COLOR 0x000000

#define MAX_STATIC_TEXTS 150

bool isEveInitialized = false;

typedef struct {
    int x, y, font;
    uint8_t r, g, b;
    uint8_t line;
    uint32_t bg;
    char text[MAX_LENGTH];
} StaticText;

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

StaticText staticTexts[MAX_STATIC_TEXTS];
uint16_t staticTextCount = 0;

StaticText savedStaticTexts[MAX_STATIC_TEXTS];
uint16_t savedStaticTextCount = 0;
uint16_t saved_x = 0, saved_y = 0;

void ClearLine() {
    int j = 0;
    for (int i = 0; i < staticTextCount; i++) {
        if (staticTexts[i].line != actual_word.line) {
            staticTexts[j++] = staticTexts[i];
        } else {
            DEBUG_PRINT("Cleared Line %d: %s\n", staticTexts[i].line, staticTexts[i].text);
        }
    }
    staticTextCount = j;
}

void ClearLineAfterX() {
    int j = 0;
    for (int i = 0; i < staticTextCount; i++) {
        if (staticTexts[i].line != actual_word.line || staticTexts[i].x <= actual_word.x) {
            staticTexts[j++] = staticTexts[i];
        } else {
            DEBUG_PRINT("Cleared After X=%d: %s\n", actual_word.x, staticTexts[i].text);
        }
    }
    staticTextCount = j;
}

void ClearLineBeforeX() {
    int j = 0;
    for (int i = 0; i < staticTextCount; i++) {
        if (staticTexts[i].line != actual_word.line || staticTexts[i].x > actual_word.x) {
            staticTexts[j++] = staticTexts[i];
        } else {
            DEBUG_PRINT("Cleared Before X=%d: %s\n", actual_word.x, staticTexts[i].text);
        }
    }
    staticTextCount = j;
}

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

void ResetScreen(void) {
    ClearScreen();
    PrepareScreen();
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
        INFO_PRINT("Monitor disconnected! Waiting...\n");
        isEveInitialized = false;
        sleep(1);
    }

    if (!isEveInitialized) {
        if (EVE_Init(DEMO_DISPLAY, DEMO_BOARD, DEMO_TOUCH) <= 1) {
            ERROR_PRINT("EVE initialization failed\n");
            return -1;
        }
        isEveInitialized = true;
        INFO_PRINT("Monitor connected! Initializing EVE...\n");
        DrawLogoPNG();
        ClearScreen();   
    }

    return OpenPipe();
}

int GetCharWidth(uint16_t font_size, char ch) {
    int baseWidth = font_size * 0.5;
    
    switch (ch) {
        case 'M': return baseWidth * 2;
        case 'W': return baseWidth * 3;
        case 'I': return baseWidth * 0.1;
        case 'i': case 'j': case 'l': case '!': case '|': case '.': case ',': case '\'': case '"':
            return baseWidth * 0.5;
        case 'f': case 't': return baseWidth * 0.6;
        case 'r': return baseWidth * 0.7;
        case 'b': case 'p': case 'v': return baseWidth * 1.2;
        case 'm': case 'w': return baseWidth * 1.5;
        case ' ': return baseWidth;
    }
    
    if (ch >= 'A' && ch <= 'Z') return baseWidth * 1.5;
    
    return baseWidth;
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
        Wait4CoProFIFOEmpty();
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

void AppendCharToActualWord(char ch) {
    if (actual_word_len < MAX_LENGTH - 1) {
        actual_word.text[actual_word_len] = ch;
        actual_word_len++;
        actual_word.text[actual_word_len] = '\0';
        actual_word_width += GetCharWidth(actual_word.font, ch);
    } else {
        ERROR_PRINT("actual_word.text overflow!\n");
    }
}

void AddActualTextStatic() {
    if (actual_word_len <= 0) {
        //ERROR_PRINT("Error during add static text: invalid word length\n");
        return;
    }

    if (staticTextCount >= MAX_STATIC_TEXTS) {
        DEBUG_PRINT("Maximum number of static texts reached\n");
        ResetScreen();
        return;
    }

    if (actual_word.font > 32 || actual_word.font < 15) {
        ERROR_PRINT("Error during add static text: invalid font size\n");
        return;
    }

    if (actual_word.r < 0 || actual_word.r > 255 || actual_word.g < 0 || actual_word.g > 255 || actual_word.b < 0 || actual_word.b > 255) {
        ERROR_PRINT("Error during add static text: invalid color\n");
        return;
    }

    if (actual_word.y + GetFontHeight(actual_word.font) >= Display_Height()) {
        actual_word.y = Display_Height() - GetFontHeight(actual_word.font);
    }

    if (actual_word.x + actual_word_width >= Display_Width()) {
        actual_word.x = Display_Width() - actual_word_width;
    }

    DEBUG_PRINT("Adding word: '%s' with color %d, %d, %d at %d, %d position\n", actual_word.text, 
        actual_word.r, actual_word.g, actual_word.b, 
        actual_word.x, actual_word.y);
    DEBUG_PRINT("staticTextCount: %d\n", staticTextCount);

    actual_word.text[actual_word_len] = '\0';

    staticTexts[staticTextCount++] = actual_word;

    actual_word.x += actual_word_width;
    actual_word_len = 0;
    actual_word_width = 0;
}


void handle_escape_sequence(const char **ptr) {
    if (**ptr == '(' && *(*ptr + 1) == 'B') {
        (*ptr) += 2;
        DEBUG_PRINT("Set character set to ASCII\n");
        return;
    }

    if (**ptr != '[') return;
    (*ptr)++;

    if (**ptr == '?') {
        char seq[32] = {0};
        int i = 0;
        while (isdigit(**ptr) || **ptr == ';' || **ptr == '?') {
            if (i < (int)sizeof(seq) - 1) {
                seq[i++] = **ptr;
            }
            (*ptr)++;
        }
        seq[i] = '\0';

        if (strcmp(seq, "?1049") == 0) {
            if (**ptr == 'h') {
                memcpy(savedStaticTexts, staticTexts, sizeof(staticTexts));
                savedStaticTextCount = staticTextCount;
                saved_x = actual_word.x;
                saved_y = actual_word.y;
                actual_word.x = 0;
                actual_word.y = 0;
                actual_word.line = 0;
                ResetScreen();               
            } else if (**ptr == 'l') {
                memcpy(staticTexts, savedStaticTexts, sizeof(savedStaticTexts));
                staticTextCount = savedStaticTextCount;
                actual_word.x = saved_x;
                actual_word.y = saved_y;
                ResetScreen();
            }
            (*ptr)++;
        } else if (strcmp(seq, "?1") == 0) { 
            if (**ptr == 'h') {
                DEBUG_PRINT("Set cursor keys to application mode\n");
            } else if (**ptr == 'l') {
                DEBUG_PRINT("Set cursor keys to cursor mode\n");
            }
            (*ptr)++;
        }

        return;
    }
    
    char seq[32] = {0};
    int i = 0;
    while (isdigit(**ptr) || **ptr == ';' || **ptr == '?') {
        if (i < (int)sizeof(seq) - 1) {
            seq[i++] = **ptr;
        }
        (*ptr)++;
    }
    seq[i] = '\0';

    switch (**ptr) {
        case 'H':
            //ResetScreen();
            actual_word.x = 0; 
            actual_word.y = 0;
            actual_word.line = 0;
            break;
        case 'J':
            if (atoi(seq) == 2) {
                ResetScreen();
            }
            break;
        case 'K': {
            int code = atoi(seq);
            switch (code) {
                case 0: 
                    // Clear from cursor to end of line
                    ClearLineAfterX();
                    break;
                case 1: 
                    // Clear from start of line to cursor
                    ClearLineBeforeX();
                    break;
                case 2: 
                    // Clear entire line
                    ClearLine();
                    break;
                default:
                    ERROR_PRINT("Unknown ESC [ n K code: %d\n", code);
                    break;
            }
            break;
        }
        case '?':  // ESC [ ? n h/l - DEC private mode set/reset
            
            break;
        case 'm': {
            AddActualTextStatic();
            char *token = strtok(seq, ";");
            while (token) {
                int code = atoi(token);
                switch (code) {
                    case 0: 
                        actual_word.r = DEFAULT_COLOR_R;
                        actual_word.g = DEFAULT_COLOR_G;
                        actual_word.b = DEFAULT_COLOR_B;
                        actual_word.bg = DEFAULT_BG_COLOR;  
                        break;
                    case 30: 
                        actual_word.r = actual_word.g = actual_word.b = 0; 
                        break;
                    case 31: 
                        actual_word.r = 255; 
                        actual_word.g = 0; 
                        actual_word.b = 0; 
                        break;
                    case 32: 
                        actual_word.r = 0; 
                        actual_word.g = 255; 
                        actual_word.b = 0; 
                        break;
                    case 33: 
                        actual_word.r = 255; 
                        actual_word.g = 255; 
                        actual_word.b = 0; 
                        break;
                    case 34: 
                        actual_word.r = 0; 
                        actual_word.g = 0; 
                        actual_word.b = 255; 
                        break;
                    case 35: 
                        actual_word.r = 255; 
                        actual_word.g = 0; 
                        actual_word.b = 255; 
                        break;
                    case 36: 
                        actual_word.r = 0; 
                        actual_word.g = 255; 
                        actual_word.b = 255; 
                        break;
                    case 37: 
                        actual_word.r = actual_word.g = actual_word.b = 255; 
                        break;
                    default: 
                        actual_word.r = DEFAULT_COLOR_R;
                        actual_word.g = DEFAULT_COLOR_G;
                        actual_word.b = DEFAULT_COLOR_B;
                        actual_word.bg = DEFAULT_BG_COLOR;
                        break;
                }
                token = strtok(NULL, ";");
            }
            //DEBUG_PRINT("Set new color: %d %d %d\n", actual_word.r, actual_word.g, actual_word.b);
            break;
        }
        default:
            ERROR_PRINT("Unknown ANSI sequence: ESC [ %s %c\n", seq, **ptr);
    }
    (*ptr)++;
}

void parse_ansi(const char* buffer) {
    const char *ptr = buffer;

    DEBUG_PRINT("Parsing ANSI: '%s'\n", buffer);

    while (*ptr) {
        if (actual_word.y + GetFontHeight(actual_word.font) >= Display_Height()) { // TODO : Scroll
            ResetScreen();
            actual_word.x = 0; 
            actual_word.y = 0;
            actual_word.line = 0;
        }

        if (*ptr == '\n' || actual_word.x + actual_word_width >= Display_Width()) {
            AddActualTextStatic();
            if (*ptr == '\n' || actual_word.x + actual_word_width >= Display_Width()) {
                actual_word.y += GetFontHeight(actual_word.font);
                actual_word.line++;
                actual_word.x = 0;
            }

            if (*ptr == '\n') {
                ptr++;
            }

            continue;
        }
        if (*ptr == '^' && *(ptr + 1) == '[') {
            ptr += 2;
            handle_escape_sequence(&ptr);
            continue;
        }
        if (is_valid_utf8(ptr)) {
            AppendCharToActualWord(*ptr);
        } else {
            ERROR_PRINT("Invalid UTF-8 encoding\n");
            return;
        }
        ptr++;
    }
    AddActualTextStatic();
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
            INFO_PRINT("FIFO closed, reopening...\n");
            sleep(1);
        } else if (bytesRead == -1) {
            INFO_PRINT("FIFO error, reopening...\n");
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