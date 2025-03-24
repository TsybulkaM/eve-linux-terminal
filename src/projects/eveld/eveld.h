#ifndef EVELD_H
#define EVELD_H

#include "eve.h"
#include "hw_api.h"
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef _MSC_VER
#include <conio.h>
#endif

#define FIFO_PATH "/tmp/eve_pipe"

#define DEBUG 0
#if DEBUG
#define DEBUG_PRINT(fmt, ...) fprintf(stderr, "\x1B[35mDEBUG:\x1B[m " fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)
#endif

// need TODO ansi commands
#define TODO 0
#if TODO
#define TODO_PRINT(fmt, ...) fprintf(stderr, "\x1B[33mTODO:\x1B[m " fmt, ##__VA_ARGS__)
#else
#define TODO_PRINT(fmt, ...)
#endif

#define INFO_PRINT(fmt, ...) fprintf(stderr, "\x1B[32mINFO:\x1B[m " fmt, ##__VA_ARGS__)
#define ERROR_PRINT(fmt, ...) fprintf(stderr, "\x1B[31mERROR:\x1B[m " fmt, ##__VA_ARGS__)

extern bool LINE_FEED;

#define MAX_LINES 20
#define MAX_LENGTH 512
#define DEFAULT_FONT 16
#define DEFAULT_OPTION 0

#define DEFAULT_COLOR_R 255
#define DEFAULT_COLOR_G 255
#define DEFAULT_COLOR_B 255

#define DEFAULT_COLOR_BG_R 0
#define DEFAULT_COLOR_BG_G 0
#define DEFAULT_COLOR_BG_B 0

#define BD_ANSI_LEN 10

typedef struct
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Color;

#define COLOR_FMT "%d, %d, %d"
#define COLOR_ARGS(c) (c).r, (c).g, (c).b

#define MAX_STATIC_TEXTS 200

extern bool isEveInitialized;
extern bool mutex_second_buffer;
extern char breakdown_ansi[BD_ANSI_LEN];

typedef struct
{
  uint16_t x;
  uint16_t y;
  uint16_t font;
  Color text_color;
  Color bg_color;
  uint16_t line;
  uint16_t width;
  char text[MAX_LENGTH];
} StaticText;

extern StaticText actual_word;
extern StaticText saved_word;
extern uint16_t actual_word_len;

extern StaticText staticTexts[MAX_STATIC_TEXTS];
extern uint16_t staticTextCount;

extern StaticText savedStaticTexts[MAX_STATIC_TEXTS];
extern uint16_t savedStaticTextCount;
extern uint16_t saved_x, saved_y;

int check_ftdi_device(void);

// operations
// eveld_ops.c
int InitializeScreen(int fd);
int OpenPipe(void);
void handle_escape_sequence(const char **ptr);
void parse_ansi(const char *buffer);
void ListenToFIFO();

// framebuffer operations
// eveld_fb.c
void PrepareScreen(void);
void ClearScreen(void);
void DisplayFrame(void);
void ResetScreen(void);

int GetCharWidth(uint16_t, char);
int GetTextWidth(const char *, int);
int GetFontHeight(int font);

bool is_valid_utf8(const char **bytes);
bool colors_are_equal(Color a, Color b);

void SetActualNewLine(uint16_t line);

void AppendCharToActualWord(char ch);
void DeleteCharH(void);
void AddOrMergeActualTextStatic(void);

void DrawStaticTexts(void);

void DeleteChatH(uint16_t count);

void ClearLineBeforeX(void);
void ClearLineAfterX(void);
void ClearLine(void);
void ClearPlaceForActual(void);
void ClearPlaceForActualDev(void);

// memory management
// eveld_mem.c
uint32_t monitor_display_list_memory(void);
void check_display_list_memory(void);

// initial logo
// logo.c
void DrawLogoPNG(void);

#endif // EVELD_H