#include "eveld.h"


int OpenPipe(void) {
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
        close(fd);
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
        ResetScreen();
    }

    return OpenPipe();
}

void _return_to_stand_buffer() {
    memcpy(staticTexts, savedStaticTexts, sizeof(StaticText) * MAX_STATIC_TEXTS);
    
    staticTextCount = savedStaticTextCount;
    actual_word.x = saved_word.x;
    actual_word.y = saved_word.y;
    actual_word.line = saved_word.line;

    savedStaticTextCount = 0;
    saved_word.x = 0;
    saved_word.y = 0;
    saved_word.line = 0;

    PrepareScreen();
    DrawStaticTexts();
    DisplayFrame();
}

void handle_escape_sequence(const char **ptr) {
    if (**ptr == '(' && *(*ptr + 1) == 'B') {
        (*ptr) += 2;
        TODO_PRINT("Set character set to ASCII\n");
        return;
    } else if (**ptr == '=') {
        (*ptr)++;
        TODO_PRINT("Set alternate keypad mode\n");
        return;
    } else if (**ptr == '>') {
        (*ptr)++;
        TODO_PRINT("Set numeric keypad mode\n");
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
                memcpy(savedStaticTexts, staticTexts, sizeof(StaticText) * MAX_STATIC_TEXTS);

                savedStaticTextCount = staticTextCount;
                saved_word.x = actual_word.x;
                saved_word.y = actual_word.y;
                saved_word.line = actual_word.line;

                actual_word.x = 0;
                actual_word.y = 0;
                actual_word.line = 0;

                ResetScreen();               
            } else if (**ptr == 'l') {
                _return_to_stand_buffer();
            }
            (*ptr)++;
        } else if (strcmp(seq, "?1") == 0) { 
            if (**ptr == 'h') {
                TODO_PRINT("Set cursor keys to application mode\n");
            } else if (**ptr == 'l') {
                TODO_PRINT("Set cursor keys to cursor mode\n");
            }
            else {
                ERROR_PRINT("Unknown CSI ? sequence: %s\n", seq);
            }
            (*ptr)++;
        } else if (strcmp(seq, "?25") == 0) {
            if (**ptr == 'h') {
                TODO_PRINT("Show cursor\n");
            } else if (**ptr == 'l') {
                TODO_PRINT("Hide cursor\n");
            }
            (*ptr)++;
        } else {
            ERROR_PRINT("Unknown CSI ? sequence: %s\n", seq);
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
            int row = 0, col = 0;

            if (seq[0] != '\0') {
                sscanf(seq, "%d;%d", &row, &col);
            }

            AddActualTextStatic();

            actual_word.y = (row > 0) ? row*GetFontHeight(actual_word.font)  : 0;
            actual_word.x = (col > 0) ? col*GetCharWidth(actual_word.font, ' ') : 0;

            SetActualNewLine(actual_word.y/GetFontHeight(actual_word.font));
            DEBUG_PRINT("Sequence: %s, Move to row %d, column %d, X = %d, Y = %d\n", seq, row, col, actual_word.x, actual_word.y);
            break;
        case 'd':
            AddActualTextStatic();
            actual_word.y = (seq[0] != '\0') ? atoi(seq)*GetFontHeight(actual_word.font) : 0;
            SetActualNewLine(atoi(seq));
            actual_word.x = 0;
            DEBUG_PRINT("Move to line: %d, Y = %d\n", atoi(seq), actual_word.y);
            break;
        case 'A':
            AddActualTextStatic();
            actual_word.y -= (seq[0] != '\0') ? atoi(seq)*GetFontHeight(actual_word.font) : GetFontHeight(actual_word.font);
            SetActualNewLine(atoi(seq));
            DEBUG_PRINT("Move up %d lines, Y = %d\n", atoi(seq), actual_word.y);
            break;
        case 'B':
            AddActualTextStatic();
            actual_word.y += (seq[0] != '\0') ? atoi(seq)*GetFontHeight(actual_word.font) : GetFontHeight(actual_word.font);
            SetActualNewLine(atoi(seq));
            DEBUG_PRINT("Move down %d lines, Y = %d\n", atoi(seq), actual_word.y);
            break;
        case 'C':
            AddActualTextStatic();
            actual_word.x += (seq[0] != '\0') ? atoi(seq)*GetCharWidth(actual_word.font, ' ') : GetCharWidth(actual_word.font, ' ');
            DEBUG_PRINT("Move right %d spaces, X = %d\n", atoi(seq), actual_word.x);
            break;
        case 'D':
            AddActualTextStatic();
            actual_word.x -= (seq[0] != '\0') ? atoi(seq)*GetCharWidth(actual_word.font, ' ') : GetCharWidth(actual_word.font, ' ');
            DEBUG_PRINT("Move left %d spaces, X = %d\n", atoi(seq), actual_word.x);
            break;
        case 'P':
            AddActualTextStatic();
            uint16_t count = (seq[0] != '\0') ? atoi(seq) : 0;
            DeleteChatH(count);
            DEBUG_PRINT("Delete %d characters\n", count);
            break;
        case 'G':
            AddActualTextStatic();
            actual_word.x = (seq[0] != '\0') ? atoi(seq)*GetCharWidth(actual_word.font, ' ') : 0;
            DEBUG_PRINT("Move to column %d, X = %d\n", atoi(seq), actual_word.x);
            break;
        case 'J':
            if (atoi(seq) == 2) {
                ResetScreen();
                DEBUG_PRINT("Clear screen\n");
            }
            break;
        case 'K': {
            int code = atoi(seq);
            switch (code) {
                case 0:
                    ClearLineAfterX();
                    break;
                case 1:
                    ClearLineBeforeX();
                    break;
                case 2:
                    ClearLine();
                    break;
                default:
                    ERROR_PRINT("Unknown ESC [ n K code: %d\n", code);
                    break;
            }
            break;
        }
        case '?':  
            break;
        case 'm': {
            StaticText next_word = {
                .text_color = actual_word.text_color,
                .bg_color = actual_word.bg_color,
                .font = actual_word.font,
            };
        
            bool reverse = false;
        
            char *token = strtok(seq, ";");

            if (!token) {
                next_word.text_color = (Color){DEFAULT_COLOR_R, DEFAULT_COLOR_G, DEFAULT_COLOR_B};
                next_word.bg_color = (Color){DEFAULT_COLOR_BG_R, DEFAULT_COLOR_BG_G, DEFAULT_COLOR_BG_B};
            }

            while (token) {
                int code = atoi(token);
                switch (code) {
                    case 0:
                        next_word.text_color = (Color){DEFAULT_COLOR_R, DEFAULT_COLOR_G, DEFAULT_COLOR_B};
                        next_word.bg_color = (Color){DEFAULT_COLOR_BG_R, DEFAULT_COLOR_BG_G, DEFAULT_COLOR_BG_B};
                        reverse = false;
                        break;
                    case 1:
                        //next_word.font = 18;
                        TODO_PRINT("Set bold text\n");
                        break;
                    case 2:
                        TODO_PRINT("Set dim text\n");
                        break;
                    case 3:
                        TODO_PRINT("Set italic text\n");
                        break;
                    case 4:
                        TODO_PRINT("Set underline text\n");
                        break;
                    case 5:
                        TODO_PRINT("Set slow blink text\n");
                        break;
                    case 6:
                        TODO_PRINT("Set rapid blink text\n");
                        break;
                    case 7:
                        reverse = true;
                        break;
                    case 22:
                        TODO_PRINT("Set normal intensity\n");
                        //next_word.font = DEFAULT_FONT;
                        break;
                    // Normal text colors (30-37)
                    case 30: 
                        next_word.text_color = (Color){0, 0, 0};
                        break; // Black
                    case 31:
                        next_word.text_color = (Color){255, 0, 0};
                        break; // Red
                    case 32:
                        next_word.text_color = (Color){0, 255, 0};
                        break; // Green
                    case 33:
                        next_word.text_color = (Color){255, 225, 0};
                        break; // Yellow
                    case 34:
                        next_word.text_color = (Color){0, 0, 255};
                        break; // Blue
                    case 35:
                        next_word.text_color = (Color){255, 0, 255};
                        break; // Magenta
                    case 36:
                        next_word.text_color = (Color){0, 255, 255};
                        break; // Cyan
                    case 37:
                        next_word.text_color = (Color){255, 255, 255};
                        break; // White
                    case 39: 
                        next_word.text_color = (Color){DEFAULT_COLOR_R, DEFAULT_COLOR_G, DEFAULT_COLOR_B};
                        break; // Default color

                    // Bright text colors (90-97)
                    case 90:
                        next_word.text_color = (Color){128, 128, 128};
                        break; // Gray
                    case 91:
                        next_word.text_color = (Color){255, 128, 128};
                        break; // Light red
                    case 92: 
                        next_word.text_color = (Color){128, 255, 128};
                        break; // Light green
                    case 93: 
                        next_word.text_color = (Color){255, 255, 128};
                        break; // Light yellow
                    case 94: 
                        next_word.text_color = (Color){128, 128, 255};
                        break; // Light blue
                    case 95: 
                        next_word.text_color = (Color){255, 128, 255};
                        break; // Light magenta
                    case 96: 
                        next_word.text_color = (Color){128, 255, 255};
                        break; // Light cyan
                    case 97: 
                        next_word.text_color = (Color){255, 255, 255};
                        break; // Bright white

                    // Normal background colors (40-47)
                    case 40:
                        next_word.bg_color = (Color){0, 0, 0};
                        break; // Black background
                    case 41: 
                        next_word.bg_color = (Color){255, 0, 0};
                        break; // Red background
                    case 42: 
                        next_word.bg_color = (Color){0, 255, 0};
                        break; // Green background
                    case 43: 
                        next_word.bg_color = (Color){255, 255, 0};
                        break; // Yellow background
                    case 44: 
                        next_word.bg_color = (Color){0, 0, 255};
                        break; // Blue background
                    case 45: 
                        next_word.bg_color = (Color){255, 0, 255};
                        break; // Magenta background
                    case 46: 
                        next_word.bg_color = (Color){0, 255, 255};
                        break; // Cyan background
                    case 47: 
                        next_word.bg_color = (Color){255, 255, 255};
                        break; // White background
                    case 49: 
                        next_word.bg_color = (Color){DEFAULT_COLOR_BG_R, DEFAULT_COLOR_BG_G, DEFAULT_COLOR_BG_B};
                        break; // Default background

                    // Bright background colors (100-107)
                    case 100: 
                        next_word.bg_color = (Color){128, 128, 128};
                        break; // Gray background
                    case 101: 
                        next_word.bg_color = (Color){255, 128, 128};
                        break; // Light red background
                    case 102: 
                        next_word.bg_color = (Color){128, 255, 128};
                        break; // Light green background
                    case 103: 
                        next_word.bg_color = (Color){255, 255, 128};
                        break; // Light yellow background
                    case 104: 
                        next_word.bg_color = (Color){128, 128, 255};
                        break; // Light blue background
                    case 105: 
                        next_word.bg_color = (Color){255, 128, 255};
                        break; // Light magenta background
                    case 106: 
                        next_word.bg_color = (Color){128, 255, 255};
                        break; // Light cyan background
                    case 107: 
                        next_word.bg_color = (Color){255, 255, 255};
                        break; // Bright white background

                    default:
                        ERROR_PRINT("Unknown ANSI color code: %d\n", code);
                        break;
                }
                token = strtok(NULL, ";");
            }
        
            if (reverse) {
                Color tmp_color = next_word.text_color;
                next_word.text_color = (Color){
                    next_word.bg_color.r & 0xFF,
                    next_word.bg_color.g & 0xFF,
                    next_word.bg_color.b & 0xFF
                };
                next_word.bg_color = tmp_color;
            }
        
            if (!colors_are_equal(actual_word.text_color, next_word.text_color) || 
                !colors_are_equal(actual_word.bg_color, next_word.bg_color) ||
                actual_word.font != next_word.font) {
                AddActualTextStatic();
            }
        
            actual_word.text_color = next_word.text_color;
            actual_word.bg_color = next_word.bg_color;
            actual_word.font = next_word.font;
            break;
        }
        default:
            ERROR_PRINT("Unknown ANSI sequence: ESC [ %s %c\n", seq, **ptr);
            break;
    }
    (*ptr)++;
}

void parse_ansi(const char* buffer) {
    const char *ptr = buffer;

    DEBUG_PRINT("Parsing ANSI: '%s'\n", buffer);

    while (*ptr) {
        // TODO: Scroll
        if (actual_word.y + GetFontHeight(actual_word.font) >= Display_Height()) { 
            ResetScreen();
            actual_word.x = 0; 
            actual_word.y = 0;
            SetActualNewLine(0);
        }

        if (*ptr == '\n' || actual_word.x + actual_word.width >= Display_Width()) {
            AddActualTextStatic();
            if (*ptr == '\n' || actual_word.x + actual_word.width >= Display_Width()) {
                actual_word.y += GetFontHeight(actual_word.font);
                DEBUG_PRINT("New line, Y = %d\n", actual_word.y);
                SetActualNewLine(actual_word.line+1);
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

        if (is_valid_utf8(&ptr)) {
            AppendCharToActualWord(*ptr);
            ptr++;
        } else {
            ERROR_PRINT("Invalid UTF-8 encoding %c\n", *ptr);
            return;
        }
    }

    AddActualTextStatic();
    DrawStaticTexts();
}

void ListenToFIFO(void) {
    char buffer[MAX_LENGTH];

    int fd = -1;

    while (1) {
        fd = InitializeScreen(fd);

        size_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
        
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            PrepareScreen();
            parse_ansi(buffer);
        } else if (bytesRead == 0) {
            INFO_PRINT("FIFO closed, reopening...\n");

            if (staticTextCount > 0) {
                //_return_to_stand_buffer();
            }
            //sleep(1);
        } else if (bytesRead == -1) {
            INFO_PRINT("FIFO error, reopening...\n");
            sleep(1);
        }
    }
}
