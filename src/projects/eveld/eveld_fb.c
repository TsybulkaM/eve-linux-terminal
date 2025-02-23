#include "eveld.h"

void PrepareScreen() {
    Send_CMD(CMD_DLSTART);
    Send_CMD(VERTEXFORMAT(0));
    Send_CMD(CLEAR_COLOR_RGB(0, 0, 0));
    Send_CMD(CLEAR(1, 1, 1));
}

void DisplayFrame() {
    Send_CMD(DISPLAY());
    Send_CMD(CMD_SWAP);
    UpdateFIFO();
    Wait4CoProFIFOEmpty();
}

void ClearScreen(void) {
    DisplayFrame();
    staticTextCount = 0;
}

void ResetScreen(void) {
    ClearScreen();
    PrepareScreen();
}


int GetCharWidth(uint16_t font_size, char ch) {
    int baseWidth = font_size * 0.5;
    
    switch (ch) {
        case 'M': return baseWidth * 1.4;
        case 'W': return baseWidth * 1.5;
        case 'I': return baseWidth * 0.1;
        case 'i': case 'l': case '!': case '|': case '.': case ',': case '\'': case '"':
            return baseWidth * 0.8;
        case 'f': case 't': case 'j': return baseWidth * 0.9;
        case 'b': case 'p': case 'v': case 'm': case 'w': return baseWidth * 1.2;
        case ' ': return baseWidth;
    }
    
    if (ch >= 'A' && ch <= 'Z') return baseWidth * 1.2;
    
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

bool colors_are_equal(Color a, Color b) {
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

void SetActualNewLine(uint16_t line) {
    actual_word.line = line;
    mutex_newline = true;
}


void AppendCharToActualWord(char ch) {
    if (actual_word_len < MAX_LENGTH - 1) {
        actual_word.text[actual_word_len] = ch;
        actual_word_len++;
        actual_word.text[actual_word_len] = '\0';
        actual_word.width += GetCharWidth(actual_word.font, ch);
    } else {
        ERROR_PRINT("Error during append char: maximum length reached\n");
    }
}

void AddActualTextStatic(void) {
    if (actual_word_len <= 0) {
        return;
    }

    if (staticTextCount >= MAX_STATIC_TEXTS) {
        DEBUG_PRINT("Maximum number of static texts reached\n");
        ClearLine();
        return;
    }

    if (mutex_newline) {
        ClearLine();
        mutex_newline = false;
    }

    if (actual_word.font > 32 || actual_word.font < 15) {
        ERROR_PRINT("Error during add static text: invalid font size\n");
        return;
    }

    if (actual_word.y < 0) {
        DEBUG_PRINT("Y position is too low, setting to 0\n");
        actual_word.y = 0;
    }

    if (actual_word.y + GetFontHeight(actual_word.font) >= Display_Height()) {
        actual_word.y = Display_Height() - GetFontHeight(actual_word.font);
        DEBUG_PRINT("Y position is too high, setting to %d\n", actual_word.y);
    }

    if (actual_word.x < 0) {
        actual_word.x = 0;
    }

    if (actual_word.x + actual_word.width >= Display_Width()) {
        actual_word.x = Display_Width() - actual_word.width;
    }

    DEBUG_PRINT("Adding word: '%s' with width %d fg color " COLOR_FMT " and bg color " COLOR_FMT " at %d, %d position\n", 
        actual_word.text, 
        actual_word.width,
        COLOR_ARGS(actual_word.text_color),
        COLOR_ARGS(actual_word.bg_color),
        actual_word.x, actual_word.y
    );

    actual_word.text[actual_word_len] = '\0';

    staticTexts[staticTextCount++] = actual_word;

    actual_word.x += actual_word.width;
    actual_word_len = 0;
    actual_word.width = 0;
}


void DrawStaticTexts(void) {
    for (int i = 0; i < staticTextCount; i++) {

        if (!colors_are_equal(staticTexts[i].bg_color, 
            (Color){DEFAULT_COLOR_BG_R, DEFAULT_COLOR_BG_G, DEFAULT_COLOR_BG_B})) {

            Send_CMD(COLOR_RGB(
                staticTexts[i].bg_color.r,
                staticTexts[i].bg_color.g,
                staticTexts[i].bg_color.b
            ));

            Send_CMD(BEGIN(RECTS));
            Send_CMD(VERTEX2F(
                staticTexts[i].x, 
                staticTexts[i].y
            ));
            Send_CMD(VERTEX2F(
                staticTexts[i].x + GetTextWidth(staticTexts[i].text, staticTexts[i].font) - 5, 
                staticTexts[i].y + GetFontHeight(staticTexts[i].font) - 10
            ));
            Send_CMD(END());
        }

        Send_CMD(COLOR_RGB(
            staticTexts[i].text_color.r,
            staticTexts[i].text_color.g,
            staticTexts[i].text_color.b
        ));
        
        Cmd_Text(
            staticTexts[i].x, staticTexts[i].y, 
            staticTexts[i].font, DEFAULT_OPTION, 
            staticTexts[i].text
        );
    }

    DEBUG_PRINT("staticTextCount: %d\n", staticTextCount);
    DisplayFrame();
}


void DeleteChatH(uint16_t count) {
    for (int i = 0; i < staticTextCount; i++) {
        if (staticTexts[i].line != actual_word.line || staticTexts[i].x <= actual_word.x) {
            staticTexts[i].x -= count*GetCharWidth(staticTexts[i].font, ' ');
        }
    }
} 


void ClearLine(void) {
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

void ClearLineAfterX(void) {
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

void ClearLineBeforeX(void) {
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

void ClearPlaceForActual(void) {
    int j = 0;
    for (int i = 0; i < staticTextCount; i++) {
        if (staticTexts[i].line != actual_word.line || 
            !(staticTexts[i].x <= actual_word.x && staticTexts[i].x + staticTexts[i].width >= actual_word.x + actual_word.width)) {
            staticTexts[j++] = staticTexts[i];
        } else {
            DEBUG_PRINT("Cleared Place X=%d: %s\n", actual_word.x, staticTexts[i].text);
        }
    }
    staticTextCount = j;
}
