#include "eveld.h"

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

int OpenPipe() {
    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) {
        perror("Error opening FIFO");
        sleep(1);
    }

    return fd;
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
                    DEBUG_PRINT("Clear from cursor to end of line\n");
                    ClearLineAfterX();
                    break;
                case 1:
                    DEBUG_PRINT("Clear from cursor to beginning of line\n");
                    ClearLineBeforeX();
                    break;
                case 2: 
                    DEBUG_PRINT("Clear entire line\n");
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
        if (actual_word.y + GetFontHeight(actual_word.font) >= Display_Height()) { 
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
                tmp_flag_nl = true;
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
    DrawStaticTexts();
}

void ListenToFIFO() {
    char buffer[MAX_LENGTH];

    int fd = -1;

    while (1) {
        fd = InitializeScreen(fd);

        size_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);

        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';

            PrepareScreen();

            parse_ansi(buffer);

        } else if (bytesRead == 0) {
            INFO_PRINT("FIFO closed, reopening...\n");
            sleep(1);
        } else if (bytesRead == -1) {
            INFO_PRINT("FIFO error, reopening...\n");
            sleep(1);
        }
    }
}
