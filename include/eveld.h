#ifndef EVELD_H
#define EVELD_H

#include "eve.h"
#include "hw_api.h"
#include <ctype.h>
#include <fcntl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <wchar.h>
#ifdef _MSC_VER
#include <conio.h>
#endif

#define VERSION "1.0.5"

// Configuration of the EVE display
#define DEMO_DISPLAY DISPLAY_43_480x272 // 4.3" 480x272 display
#define DEMO_BOARD BOARD_EVE3           // EVE3 board
#define DEMO_TOUCH TOUCH_TPR           // Touch Panel Resistive

#define FIFO_PATH "/tmp/eve_pipe"
#define FIFO_OUT_PATH "/tmp/eve_pipe_out"

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

#define DEBUG 1
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

#define MAX_LENGTH 1024
#define DEFAULT_FONT 1
#define DEFUALT_CHAR_HIGHT 24
#define DEFAULT_OPTION 0

#define DEFAULT_COLOR_R 255
#define DEFAULT_COLOR_G 255
#define DEFAULT_COLOR_B 255

#define DEFAULT_COLOR_BG_R 0
#define DEFAULT_COLOR_BG_G 0
#define DEFAULT_COLOR_BG_B 0

#define BD_ANSI_LEN 64

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
  uint16_t symbol_len;
  char text[MAX_LENGTH];
} StaticText;

extern StaticText actual_word;
extern StaticText saved_word;
extern uint16_t actual_word_bytes;

extern StaticText staticTexts[MAX_STATIC_TEXTS];
extern uint16_t staticTextCount;

extern StaticText savedStaticTexts[MAX_STATIC_TEXTS];
extern uint16_t savedStaticTextCount;
extern uint16_t saved_x, saved_y;

// Font parameters
#define CHANK_SIZE 4096

typedef struct
{
  uint8_t id;
  uint8_t size;
  uint8_t width;
  uint8_t height;
  size_t xfont_size;
  size_t glyph_size;
  const unsigned char *xfont;
  const unsigned char *glyph;
} font_t;

extern const font_t fonts[];
extern const size_t fonts_len;
const font_t *get_font_by_id(uint8_t);

// operations
// eveld_ops.c
int InitializeScreen(int);
int OpenPipe(void);
int OpenOutPipe(void);
void parse_meta_notation(char *);
void handle_escape_sequence(const char **);
void parse_ansi(char *);
void ListenToFIFO();

// framebuffer operations
// eveld_fb.c
void PrepareScreen(void);
void ClearScreen(void);
void DisplayFrame(void);
void ResetScreen(void);

uint8_t GetCharWidth(uint8_t);
int GetTextWidth(const char *, uint8_t, int);
int GetFontHeight(int);

size_t utf8_char_length(uint8_t);
bool colors_are_equal(Color, Color);

void SetActualNewLine(uint16_t);

void AppendCharToActualWord(const char *, size_t);
void DeleteCharH(void);
void AddOrMergeActualTextStatic(void);

void DrawStaticTexts(void);

void DeleteChatH(uint16_t);

void ClearLineBeforeX(void);
void ClearLineAfterX(void);
void ClearAfterX(void);
void ClearBeforeX(void);
void ClearLine(void);
void ClearPlaceForActual(void);
void ClearPlaceForActualDev(void);

// HAL функции
bool check_ftdi_device(void);

// initial logo
// logo.c
void DrawLogoPNG(void);

#endif // EVELD_H