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
#include <locale.h>
#include <wchar.h>
#ifdef _MSC_VER
#include <conio.h>
#endif

#define FIFO_PATH "/tmp/eve_pipe"

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

#define MAX_LINES 20
#define MAX_LENGTH 512
#define DEFAULT_FONT 2
#define DEFUALT_CHAR_WIDTH 10
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

int check_ftdi_device(void);

// Font parameters
#define CHANK_SIZE 4096

extern const unsigned char ibm_plex_mono_12_ASTC_xfont[];
extern const unsigned char ibm_plex_mono_16_ASTC_xfont[];
extern const unsigned char ibm_plex_mono_20_ASTC_xfont[];
extern const unsigned char ibm_plex_mono_24_ASTC_xfont[];
extern const unsigned int ibm_plex_mono_ASTC_xfont_len;

extern const unsigned char ibm_plex_mono_12_20_24_ASTC_glyph[];
extern const unsigned int ibm_plex_mono_12_20_24_ASTC_glyph_len;

extern const unsigned char ibm_plex_mono_16_ASTC_glyph[];
extern const unsigned int ibm_plex_mono_16_ASTC_glyph_len;

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

int GetCharWidth(uint32_t);
int GetTextWidth(const char *, int);
int GetFontHeight(int font);

size_t utf8_char_length(uint8_t);
bool colors_are_equal(Color a, Color b);

void SetActualNewLine(uint16_t line);

void AppendCharToActualWord(const char *bytes_to_append, size_t num_bytes);
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