#include "eveld.h"

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))


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


int GetCharWidthDev(uint16_t font_size, char ch) {
    switch (ch) {
        case 'a': return 8;
        case 'b': return 9;
        case 'c': return 8;
        case 'd': return 9;
        case 'e': return 8;
        case 'f': return 6;
        case 'g': return 9;
        case 'h': return 9;
        case 'i': return 4;
        case 'j': return 5;
        case 'k': return 9;
        case 'l': return 4;
        case 'm': return 14;
        case 'n': return 9;
        case 'o': return 9;
        case 'p': return 9;
        case 'q': return 9;
        case 'r': return 6;
        case 's': return 8;
        case 't': return 6;
        case 'u': return 9;
        case 'v': return 8;
        case 'w': return 12;
        case 'x': return 8;
        case 'y': return 8;
        case 'z': return 8;
        case 'A': return 12;
        case 'B': return 11;
        case 'C': return 11;
        case 'D': return 11;
        case 'E': return 10;
        case 'F': return 9;
        case 'G': return 12;
        case 'H': return 12;
        case 'I': return 5;
        case 'J': return 7;
        case 'K': return 11;
        case 'L': return 9;
        case 'M': return 14;
        case 'N': return 12;
        case 'O': return 12;
        case 'P': return 11;
        case 'Q': return 12;
        case 'R': return 11;
        case 'S': return 10;
        case 'T': return 10;
        case 'U': return 11;
        case 'V': return 11;
        case 'W': return 16;
        case 'X': return 11;
        case 'Y': return 11;
        case 'Z': return 10;
        case '0': return 9;
        case '1': return 9;
        case '2': return 9;
        case '3': return 9;
        case '4': return 9;
        case '5': return 9;
        case '6': return 9;
        case '7': return 9;
        case '8': return 9;
        case '9': return 9;
        case '!': return 5;
        case '?': return 9;
        case '.': return 5;
        case ',': return 5;
        case ':': return 5;
        case ';': return 5;
        case '(': return 6;
        case ')': return 6;
        case '[': return 6;
        case ']': return 6;
        case '{': return 6;
        case '}': return 6;
        case '<': return 9;
        case '>': return 9;
        case '+': return 9;
        case '-': return 9;
        case '*': return 9;
        case '/': return 9;
        case '\\': return 9;
        case '|': return 9;
        case '=': return 9;
        case '&': return 9;
        case '^': return 9;
        case '%': return 9;
        case '$': return 9;
        case '#': return 9;
        case '@': return 9;
        case '~': return 9;
        case '`': return 5;
        case '\'': return 5;
        case '"': return 7;
        case ' ': return 5;
        case '\n': return 0;
        default: return 9;
    }
}

int GetCharWidth(uint16_t font_size, char ch) {
    int baseWidth = font_size * 0.5;
    
    switch (ch) {
        case 'I': return baseWidth * 0.1;
        case 'e': return baseWidth * 0.7;
        case 'i': case '!': case '\'': case '"': case ',': case '.':
            return baseWidth * 0.8;
        case 'f': case '[': case 't': case 'j': return baseWidth * 0.9;
        case '|': case ':': case '0': case '%': case 'b': case 'p': case 'v': case 'm': case 'w': return baseWidth * 1.2;
        case 'M': return baseWidth * 1.4;
        case 'W': return baseWidth * 1.5;
    }
    
    if (ch >= 'A' && ch <= 'Z') return baseWidth * 1.4;
    
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


bool is_valid_utf8(const char **ptr) {
    const unsigned char *bytes = (const unsigned char *)*ptr;

    if (bytes[0] == 'M' && bytes[1] == '-' && bytes[2] == 'b') {
        *ptr += 10;
        return false;
    }

    if (bytes[0] <= 0x7F) {
        // 1-byte character (ASCII)
        return true;
    } else if ((bytes[0] & 0xE0) == 0xC0) {
        // 2-byte character
        if ((bytes[1] & 0xC0) == 0x80) {
            *ptr += 1;
            return true;
        }
    } else if ((bytes[0] & 0xF0) == 0xE0) {
        // 3-byte character
        if ((bytes[1] & 0xC0) == 0x80 && (bytes[2] & 0xC0) == 0x80) {
            *ptr += 2;
            return true;
        }
    } else if ((bytes[0] & 0xF8) == 0xF0) {
        // 4-byte character
        if ((bytes[1] & 0xC0) == 0x80 && (bytes[2] & 0xC0) == 0x80 && (bytes[3] & 0xC0) == 0x80) {
            *ptr += 3;
            return true;
        }
    }

    *ptr += 1;
    return false;
}


bool colors_are_equal(Color a, Color b) {
    return a.r == b.r && a.g == b.g && a.b == b.b;
} 


void SetActualNewLine(uint16_t line) {
    DEBUG_PRINT("New line, y = %d x = %d\n", actual_word.y, actual_word.x);
    actual_word.line = line;
}

void AppendCharToActualWord(char ch) {
    if (actual_word_len < MAX_LENGTH - 1) {
        //DEBUG_PRINT("Append char: %c\n", ch);
        
        actual_word.text[actual_word_len++] = ch;
        actual_word.width += GetCharWidth(actual_word.font, ch);
        actual_word.text[actual_word_len] = '\0';
    } else {
        ERROR_PRINT("Error during append char: maximum length reached\n");
    }
}

void DeleteCharH(void) {
    if (actual_word_len > 0) {
        uint8_t last_char_width = GetCharWidth(actual_word.font, actual_word.text[actual_word_len - 1]);
        actual_word.width -= last_char_width;
        actual_word.x -= last_char_width;
        actual_word_len--;
    } else {
        StaticText last = staticTexts[staticTextCount - 1];
        uint8_t last_len = strlen(last.text);
        uint8_t last_char_width = GetCharWidth(last.font, last.text[last_len - 1]);
        staticTexts[staticTextCount - 1].width = last_char_width;
        staticTexts[staticTextCount - 1].text[last_len] = '\0';
    }
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
            (staticTexts[i].x + staticTexts[i].width <= actual_word.x || 
             staticTexts[i].x >= actual_word.x + actual_word.width)) {
            staticTexts[j++] = staticTexts[i];
        } else {
            DEBUG_PRINT("Cleared Place X=%d: %s\n", actual_word.x, staticTexts[i].text);
        }
    }
    staticTextCount = j;
}


int GetTextOffset(StaticText *word, int xPos) {
    int offset = 0, px = word->x;
    while (px < xPos && word->text[offset] != '\0') {
        int charWidth = GetCharWidth(word->font, word->text[offset]);
        if (px + charWidth > xPos) {
            break;
        }
        px += charWidth;
        offset++;
    }
    return offset;
}

StaticText CreateSubText(StaticText src, int newX, int newWidth) {
    StaticText subText = src;
    subText.x = newX;
    subText.width = newWidth;
    
    int charOffset = GetTextOffset(&src, newX);
    if (charOffset >= strlen(src.text)) {
        subText.text[0] = '\0';
        return subText;
    }
    
    int px = newX;
    int textLen = 0;
    for (int i = charOffset; src.text[i] != '\0' && px < newX + newWidth; i++) {
        int charWidth = GetCharWidth(src.font, src.text[i]);
        if (px + charWidth > newX + newWidth) {
            break;
        }
        subText.text[textLen++] = src.text[i];
        px += charWidth;
    }
    subText.text[textLen] = '\0';
    
    if (textLen == 0) {
        subText.text[0] = '\0';
    }
    return subText;
}

bool IsOnlySpaces(const char *text) {
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] != ' ') {
            return false;
        }
    }
    return true;
}

void AddOrMergeActualTextStatic(void) {
    if (actual_word_len <= 0) {
        DEBUG_PRINT("Skipping empty text\n");
        return;
    }

    actual_word.text[actual_word_len] = '\0';

    if (IsOnlySpaces(actual_word.text)) {
        DEBUG_PRINT("Text contains only spaces, moving x to %d\n", actual_word.x + actual_word.width);
        actual_word.x += actual_word.width;
        actual_word_len = 0;
        actual_word.width = 0;
        return;
    }

    if (actual_word.font > 32 || actual_word.font < 15) {
        ERROR_PRINT("Error during add static text: invalid font size %d\n", actual_word.font);
        return;
    }

    actual_word.y = max(0, actual_word.y);
    actual_word.y = min(actual_word.y, Display_Height() - GetFontHeight(actual_word.font));
    actual_word.x = max(0, actual_word.x);
    actual_word.width = min(actual_word.width, Display_Width() - actual_word.x);

    DEBUG_PRINT("Actual word: '%s' at [%d, %d] width: %d, font: %d\n", 
        actual_word.text, actual_word.x, actual_word.y, actual_word.width, actual_word.font);

    StaticText newStaticTexts[MAX_STATIC_TEXTS];
    int newCount = 0;
    bool merged = false;
    StaticText *intersectingWords[MAX_STATIC_TEXTS];
    int intersectCount = 0;

    for (int i = 0; i < staticTextCount; i++) {
        StaticText *word = &staticTexts[i];

        if (word->line != actual_word.line || word->x + word->width < actual_word.x || word->x > actual_word.x + actual_word.width) {
            newStaticTexts[newCount++] = *word;
        } else {
            DEBUG_PRINT("Intersecting word: '%s' at [%d, %d] width: %d\n", word->text, word->x, word->y, word->width);
            intersectingWords[intersectCount++] = word;
        }
    }

    if (intersectCount == 0) {
        DEBUG_PRINT("No intersections, adding new word: '%s'\n", actual_word.text);
        newStaticTexts[newCount++] = actual_word;
    } else {
        StaticText *word;
        for (int i = 0; i < intersectCount; i++) {
            word = intersectingWords[i];

            if (word->font == actual_word.font &&
                colors_are_equal(word->text_color, actual_word.text_color) &&
                colors_are_equal(word->bg_color, actual_word.bg_color)) {
            
                DEBUG_PRINT("Merging text '%s' with '%s'\n", word->text, actual_word.text);
            
                uint8_t actual_index_start = 0;
                uint16_t current_x = word->x;
                uint8_t word_length = strlen(word->text);

                if (word->x + word->width == actual_word.x) {
                    actual_index_start = word_length;
                    current_x = word->x + word->width;
                } else {
                    for (int i = 0; i < word_length; i++) {
                        if (current_x >= actual_word.x) {
                            break;
                        }
                        DEBUG_PRINT("Char '%c' at %d\n", word->text[i], current_x);
                        current_x += GetCharWidth(word->font, word->text[i]);
                        actual_index_start++;
                    }
                }
            
                DEBUG_PRINT("Calculated actual_index_start: %d\n", actual_index_start);
            
                if (actual_index_start + actual_word_len > MAX_LENGTH) {
                    ERROR_PRINT("Text merge out of bounds\n");
                    return;
                }
            
                if (actual_index_start >= 0 && actual_index_start < MAX_LENGTH) {
                    DEBUG_PRINT("actual_index_start: %d\n", actual_index_start);
                
                    int max_copy_len = min(actual_word_len, MAX_LENGTH - actual_index_start);
                
                    for (int i = 0; i < max_copy_len; i++) {
                        word->text[actual_index_start + i] = actual_word.text[i];
                    }
                
                    int new_length = max(word_length, actual_index_start + max_copy_len);
                
                    if (new_length < MAX_LENGTH) {
                        word->text[new_length] = '\0';
                    } else {
                        word->text[MAX_LENGTH - 1] = '\0';
                    }
                }
            
                word->x = min(word->x, actual_word.x);
                word->width = GetTextWidth(word->text, word->font);
            
                DEBUG_PRINT("Merged result: '%s' at [%d, %d] width: %d\n", word->text, word->x, word->y, word->width);
            
                merged = true;
            } else {
                DEBUG_PRINT("Splitting word: '%s' at [%d, %d]\n", word->text, word->x, word->y);

                StaticText leftPart = CreateSubText(*word, word->x, actual_word.x - word->x);
                StaticText rightPart = CreateSubText(*word, actual_word.x + actual_word.width, (word->x + word->width) - (actual_word.x + actual_word.width));

                if (leftPart.text[0] != '\0') {
                    DEBUG_PRINT("Adding left part: '%s' at [%d, %d] width: %d\n", leftPart.text, leftPart.x, leftPart.y, leftPart.width);
                    newStaticTexts[newCount++] = leftPart;
                }

                if (rightPart.text[0] != '\0') {
                    DEBUG_PRINT("Adding right part: '%s' at [%d, %d] width: %d\n", rightPart.text, rightPart.x, rightPart.y, rightPart.width);
                    newStaticTexts[newCount++] = rightPart;
                }
            }
        }

        if (merged) {
            newStaticTexts[newCount++] = *word;
        } else {
            DEBUG_PRINT("Adding new word: '%s' at [%d, %d] width: %d\n", actual_word.text, actual_word.x, actual_word.y, actual_word.width);
            newStaticTexts[newCount++] = actual_word;
        }
    }

    if (newCount > MAX_STATIC_TEXTS) {
        ERROR_PRINT("Too many static texts! Possible memory corruption, clearing line\n");
        ClearLine();
        return;
    }

    memcpy(staticTexts, newStaticTexts, newCount * sizeof(StaticText));
    staticTextCount = newCount;

    actual_word.x += actual_word.width;
    actual_word_len = 0;
    actual_word.width = 0;
}