#ifndef EVELD_H
#define EVELD_H

#include "eve.h"
#include "hw_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _MSC_VER
#include <conio.h>
#endif


#define FIFO_PATH "/tmp/eve_pipe"

#define DEBUG 1
#if DEBUG
    #define DEBUG_PRINT(fmt, ...) fprintf(stderr, "\x1B[35mDEBUG:\x1B[m " fmt , ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...)
#endif

#define INFO_PRINT(fmt, ...) fprintf(stderr, "\x1B[32mINFO:\x1B[m " fmt , ##__VA_ARGS__)
#define ERROR_PRINT(fmt, ...) fprintf(stderr, "\x1B[31mERROR:\x1B[m " fmt , ##__VA_ARGS__)

#define MAX_LINES 20
#define MAX_LENGTH 512
#define DEFAULT_FONT 16
#define DEFAULT_OPTION 0
#define DEFAULT_COLOR_R 255
#define DEFAULT_COLOR_G 255
#define DEFAULT_COLOR_B 255
#define DEFAULT_BG_COLOR 0x000000
#define MAX_STATIC_TEXTS 60

extern bool isEveInitialized;
extern bool tmp_flag_nl;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t font;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint16_t line;
    uint32_t bg;
    char text[MAX_LENGTH];
} StaticText;

extern StaticText actual_word;
extern uint16_t actual_word_len;
extern uint32_t actual_word_width;

extern StaticText staticTexts[MAX_STATIC_TEXTS];
extern uint16_t staticTextCount;

extern StaticText savedStaticTexts[MAX_STATIC_TEXTS];
extern uint16_t savedStaticTextCount;
extern uint16_t saved_x, saved_y;


int check_ftdi_device();

// eveld_ops.c
int InitializeScreen(int fd);
int OpenPipe();
void handle_escape_sequence(const char **ptr);
void parse_ansi(const char* buffer);
void ListenToFIFO();

// eveld_fb.c
void PrepareScreen();
void ClearScreen();
void Display();
void ResetScreen();

int GetCharWidth(uint16_t, char);
int GetTextWidth(const char*, int);
int GetFontHeight(int font);
bool is_valid_utf8(const char *str);

void AppendCharToActualWord(char ch);
void AddActualTextStatic();

void DrawStaticTexts();

void ClearLineBeforeX();
void ClearLineAfterX();
void ClearLine();

// logo.c
void DrawLogoPNG();

#endif // EVELD_H