#include "eveld.h"

int OpenPipe(void)
{
  int fd = open(FIFO_PATH, O_RDONLY);
  if (fd == -1)
  {
    ERROR_PRINT("Error opening FIFO");
    sleep(1);
  }

  return fd;
}

int InitializeScreen(int fd)
{
  while (!check_ftdi_device())
  {
    INFO_PRINT("Monitor disconnected! Waiting...\n");
    isEveInitialized = false;
    close(fd);
    sleep(1);
  }

  if (!isEveInitialized)
  {
    if (EVE_Init(DEMO_DISPLAY, DEMO_BOARD, DEMO_TOUCH) <= 1)
    {
      ERROR_PRINT("EVE initialization failed\n");
      return -1;
    }

    isEveInitialized = true;
    INFO_PRINT("Monitor connected! Initializing EVE...\n");

    DrawLogoPNG();
    ResetScreen();

    // Load custom font
    StartCoProTransfer(RAM_G, 0);
    HAL_SPI_WriteBuffer((uint8_t *)&ibm_plex_mono_16_ASTC_xfont, ibm_plex_mono_16_ASTC_xfont_len);
    HAL_SPI_Disable();

    StartCoProTransfer(RAM_G + 4096, 0);
    HAL_SPI_WriteBuffer((uint8_t *)&ibm_plex_mono_16_ASTC_glyph, ibm_plex_mono_16_ASTC_glyph_len);
    HAL_SPI_Disable();
  }

  return OpenPipe();
}

void _return_to_stand_buffer()
{
  if (mutex_second_buffer) {
    mutex_second_buffer = false;
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
}

void _save_to_second_buffer()
{
  if (mutex_second_buffer)
  {
    ERROR_PRINT("Second buffer already in use\n");
    return;
  }
  mutex_second_buffer = true;
  memcpy(savedStaticTexts, staticTexts, sizeof(StaticText) * MAX_STATIC_TEXTS);

  savedStaticTextCount = staticTextCount;
  saved_word.x = actual_word.x;
  saved_word.y = actual_word.y;
  saved_word.line = actual_word.line;

  staticTextCount = 0;
  actual_word.x = 0;
  actual_word.y = 0;
  actual_word.line = 0;

  ResetScreen();
}

void handle_escape_sequence(const char **ptr)
{
  if (**ptr == '\0')
  {
    snprintf(breakdown_ansi, BD_ANSI_LEN - 1, "^[");
    return;
  }

  if (**ptr == '(' && *(*ptr + 1) == 'B')
  {
    (*ptr) += 2;
    TODO_PRINT("Set character set to ASCII\n");
    return;
  }
  else if (**ptr == '=')
  {
    (*ptr)++;
    TODO_PRINT("Set alternate keypad mode\n");
    return;
  }
  else if (**ptr == '>')
  {
    (*ptr)++;
    TODO_PRINT("Set numeric keypad mode\n");
    return;
  }

  if (**ptr == '(')
  {
    (*ptr)++;
    snprintf(breakdown_ansi, BD_ANSI_LEN - 1, "^[(");
    return;
  }

  if (**ptr != '[')
    return;
  (*ptr)++;

  if (**ptr == '\0')
  {
    snprintf(breakdown_ansi, BD_ANSI_LEN - 1, "^[[");
    return;
  }

  if (**ptr == '?')
  {
    char seq[32] = {0};
    int i = 0;
    while (isdigit(**ptr) || **ptr == ';' || **ptr == '?')
    {
      if (i < (int)sizeof(seq) - 1)
      {
        seq[i++] = **ptr;
      }
      (*ptr)++;
    }
    seq[i] = '\0';

    if (strcmp(seq, "?1049") == 0)
    {
      if (**ptr == 'h')
      {
        _save_to_second_buffer();
      }
      else if (**ptr == 'l')
      {
        _return_to_stand_buffer();
      }
      (*ptr)++;
    }
    else if (strcmp(seq, "?1") == 0)
    {
      if (**ptr == 'h')
      {
        TODO_PRINT("Set cursor keys to application mode\n");
      }
      else if (**ptr == 'l')
      {
        TODO_PRINT("Set cursor keys to cursor mode\n");
      }
      else
      {
        ERROR_PRINT("Unknown CSI ? sequence: %s\n", seq);
      }
      (*ptr)++;
    }
    else if (strcmp(seq, "?25") == 0)
    {
      if (**ptr == 'h')
      {
        TODO_PRINT("Show cursor\n");
      }
      else if (**ptr == 'l')
      {
        TODO_PRINT("Hide cursor\n");
      }
      (*ptr)++;
    }
    else if (strcmp(seq, "?7") == 0)
    {
      if (**ptr == 'h')
      {
        TODO_PRINT("Enable line wrap\n");
        //LINE_FEED = true;
      }
      else if (**ptr == 'l')
      {
        LINE_FEED = false;
      }
      (*ptr)++;

    }
    else if (strcmp(seq, "?1006;1000")) {
      if (**ptr == 'h') {
        TODO_PRINT("Enable mouse tracking\n");
      } else if (**ptr == 'l') {
        TODO_PRINT("Disable mouse tracking\n");
      }
      (*ptr)++;
    }
    else
    {
      ERROR_PRINT("Unknown CSI ? sequence: %s\n", seq);
    }

    return;
  }

  char seq[32] = {0};
  int i = 0;
  while (isdigit(**ptr) || **ptr == ';' || **ptr == '?')
  {
    if (i < (int)sizeof(seq) - 1)
    {
      seq[i++] = **ptr;
    }
    (*ptr)++;
  }
  seq[i] = '\0';

  switch (**ptr)
  {
  case 'H':
    int row = 0, col = 0;

    if (seq[0] != '\0')
    {
      sscanf(seq, "%d;%d", &row, &col);
    }

    AddOrMergeActualTextStatic();

    actual_word.y = (row > 0) ? row * GetFontHeight(actual_word.font) : 0;
    actual_word.x = (col > 0) ? (col - 1) * DEFUALT_CHAR_WIDTH : 0;

    SetActualNewLine(actual_word.y / GetFontHeight(actual_word.font));
    DEBUG_PRINT("Sequence: %s, Move to row %d, column %d, X = %d, Y = %d\n",
                seq,
                row,
                col,
                actual_word.x,
                actual_word.y);
    break;
  case 't':
    TODO_PRINT("Window manipulation\n");
    break;
  case 'r':
    TODO_PRINT("Set scrolling region\n");
    break;
  case 'l':
    TODO_PRINT("Reset mode\n");
    break;
  case 'd':
    AddOrMergeActualTextStatic();
    actual_word.y = (seq[0] != '\0') ? atoi(seq) * GetFontHeight(actual_word.font) : 0;
    SetActualNewLine(atoi(seq));
    actual_word.x = 0;
    DEBUG_PRINT("Move to line: %d, Y = %d\n", atoi(seq), actual_word.y);
    break;
  case 'A':
    AddOrMergeActualTextStatic();
    actual_word.y -= (seq[0] != '\0') ? atoi(seq) * GetFontHeight(actual_word.font)
                                      : GetFontHeight(actual_word.font);
    SetActualNewLine(atoi(seq));
    DEBUG_PRINT("Move up %d lines, Y = %d\n", atoi(seq), actual_word.y);
    break;
  case 'B':
    AddOrMergeActualTextStatic();
    actual_word.y += (seq[0] != '\0') ? atoi(seq) * GetFontHeight(actual_word.font)
                                      : GetFontHeight(actual_word.font);
    SetActualNewLine(atoi(seq));
    DEBUG_PRINT("Move down %d lines, Y = %d\n", atoi(seq), actual_word.y);
    break;
  case 'C':
    AddOrMergeActualTextStatic();
    actual_word.x += (seq[0] != '\0') ? atoi(seq) * DEFUALT_CHAR_WIDTH
                                      : DEFUALT_CHAR_WIDTH;
    DEBUG_PRINT("Move right %d spaces, X = %d\n", atoi(seq), actual_word.x);
    break;
  case 'D':
    AddOrMergeActualTextStatic();
    actual_word.x -= (seq[0] != '\0') ? atoi(seq) * DEFUALT_CHAR_WIDTH
                                      : DEFUALT_CHAR_WIDTH;
    DEBUG_PRINT("Move left %d spaces, X = %d\n", atoi(seq), actual_word.x);
    break;
  case 'P':
    AddOrMergeActualTextStatic();
    uint16_t count = (seq[0] != '\0') ? atoi(seq) : 0;
    DeleteChatH(count);
    DEBUG_PRINT("Delete %d characters\n", count);
    break;
  case 'G':
    AddOrMergeActualTextStatic();
    actual_word.x = (seq[0] != '\0') ? atoi(seq) * DEFUALT_CHAR_WIDTH : 0;
    DEBUG_PRINT("Move to column %d, X = %d\n", atoi(seq), actual_word.x);
    break;
  case 'J':
    if (atoi(seq) == 2)
    {
      ResetScreen();
      DEBUG_PRINT("Clear screen\n");
    }
    break;
  case 'K':
  {
    int code = atoi(seq);
    switch (code)
    {
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
  case 'm':
  {
    StaticText next_word = {
        .text_color = actual_word.text_color,
        .bg_color = actual_word.bg_color,
        .font = actual_word.font,
    };

    bool reverse = false;

    char *token = strtok(seq, ";");

    if (!token)
    {
      next_word.text_color = (Color){DEFAULT_COLOR_R, DEFAULT_COLOR_G, DEFAULT_COLOR_B};
      next_word.bg_color = (Color){DEFAULT_COLOR_BG_R, DEFAULT_COLOR_BG_G, DEFAULT_COLOR_BG_B};
    }

    while (token)
    {
      int code = atoi(token);
      switch (code)
      {
      case 0:
        next_word.text_color = (Color){DEFAULT_COLOR_R, DEFAULT_COLOR_G, DEFAULT_COLOR_B};
        next_word.bg_color = (Color){DEFAULT_COLOR_BG_R, DEFAULT_COLOR_BG_G, DEFAULT_COLOR_BG_B};
        reverse = false;
        break;
      case 1:
        // next_word.font = 18;
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
        // next_word.font = DEFAULT_FONT;
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
    if (reverse)
    {
      Color tmp_color = next_word.text_color;
      next_word.text_color = (Color){
          next_word.bg_color.r & 0xFF, next_word.bg_color.g & 0xFF, next_word.bg_color.b & 0xFF};
      next_word.bg_color = tmp_color;
    }

    if (!colors_are_equal(actual_word.text_color, next_word.text_color) ||
        !colors_are_equal(actual_word.bg_color, next_word.bg_color) ||
        actual_word.font != next_word.font)
    {
      AddOrMergeActualTextStatic();
    }

    actual_word.text_color = next_word.text_color;
    actual_word.bg_color = next_word.bg_color;
    actual_word.font = next_word.font;
    break;
  }
  default:
    if ('a' <= **ptr && **ptr <= 'z')
    {
      ERROR_PRINT("Unknown ANSI sequence: ESC [ %s\n", seq);
      return;
    }
    else if (**ptr == '\0')
    {
      snprintf(breakdown_ansi, BD_ANSI_LEN - 1, "^[[%s", seq);
      DEBUG_PRINT("Adding to breakdown_ansi: %s\n", breakdown_ansi);
      return;
    }
    break;
  }

  (*ptr)++;
}


void parse_meta_natation(char *buffer)
{
    char *ptr = buffer;
    char *write_ptr = buffer;
    
    while (*ptr != '\0')
    {
        size_t remaining = strlen(ptr);
        size_t consumed_len = 0;

        if (remaining >= 3 && *ptr == 'M' && *(ptr + 1) == '-') {
            if (remaining >= 4 && *(ptr + 2) == '^') { // M-^X → X & 0x1F + 128
                *write_ptr = (*(ptr + 3) & 0x1F) + 128;
                consumed_len = 4;
            } else if (remaining >= 3) { // M-X → X + 128
                *write_ptr = *(ptr + 2) + 128;
                consumed_len = 3;
            } else {
                strncpy(write_ptr, ptr, remaining);
                write_ptr += remaining;
                break;
            }
        } else {
            *write_ptr = *ptr;
            consumed_len = 1;
        }

        ptr += consumed_len;
        write_ptr++;
    }

    *write_ptr = '\0';
}


void parse_ansi(const char *buffer)
{  
  parse_meta_natation(buffer);

  size_t num_bytes = strlen(buffer);

  DEBUG_PRINT("Parsing buffer %d bytes : '%s'\n", num_bytes, buffer);

  const char *ptr = buffer;

  while (*ptr != '\0')
  {
    // TODO: Scroll
    if (actual_word.y + GetFontHeight(actual_word.font) > Display_Height())
    {
      DEBUG_PRINT("Scrolling screen\n");
      ResetScreen();
      actual_word.x = 0;
      actual_word.y = 0;
      SetActualNewLine(0);
    }

    if (*ptr == '\n' || (actual_word.x + actual_word.width >= Display_Width() && LINE_FEED))
    {
      AddOrMergeActualTextStatic();

      actual_word.y += GetFontHeight(actual_word.font);
      actual_word.x = 0;
      SetActualNewLine(actual_word.line + 1);
      
      if (*ptr == '\n')
      {
        ptr++;
      }

      continue;
    }

    if ((*ptr == '^' && *(ptr + 1) == '\0'))
    {
      ptr++;
      snprintf(breakdown_ansi, BD_ANSI_LEN - 1, "^");
      break;
    }

    if ((*ptr == '^' && *(ptr + 1) == 'M'))
    {
      AddOrMergeActualTextStatic();
      ptr += 2;
      actual_word.x = 0;
      continue; 
    }

    if ((*ptr == '^' && *(ptr + 1) == 'H'))
    {
      DeleteCharH();
      ptr += 2;
      continue;
    }

    if ((*ptr == '^' && *(ptr + 1) == '['))
    {
      ptr += 2;
      handle_escape_sequence(&ptr);
      continue;
    }

    size_t char_len = utf8_char_length((uint8_t)*ptr);
    if (char_len > num_bytes) {
      snprintf(breakdown_ansi, BD_ANSI_LEN - 1, ptr);
      ptr += char_len;
      break; 
    }
    AppendCharToActualWord(ptr, char_len);
    ptr += char_len;
    num_bytes -= char_len;
  }

  AddOrMergeActualTextStatic();
  DrawStaticTexts();
}

void ListenToFIFO(void)
{
  char buffer[MAX_LENGTH];

  int fd = -1;

  while (1)
  {
    fd = InitializeScreen(fd);

    size_t bytesRead = read(fd, buffer, sizeof(buffer) - BD_ANSI_LEN - 1);

    if (bytesRead > 0)
    {
      buffer[bytesRead] = '\0';

      if (breakdown_ansi[0] != '\0')
      {
        DEBUG_PRINT("Breakdown ANSI added to buffer: %s\n", breakdown_ansi);

        size_t ansi_len = strlen(breakdown_ansi);
        size_t buf_len = strlen(buffer);

        if (ansi_len + buf_len < MAX_LENGTH - 1)
        {
          memmove(buffer + ansi_len, buffer, buf_len + 1);
          memcpy(buffer, breakdown_ansi, ansi_len);

          buffer[ansi_len + buf_len] = '\0';
        } else {
          INFO_PRINT("Not enough space to prepend breakdown_ansi!\n");
        }

        breakdown_ansi[0] = '\0';
      }

      PrepareScreen();
      parse_ansi(buffer);
    }
    else if (bytesRead == 0)
    {
      INFO_PRINT("FIFO closed, reopening...\n");

      if (mutex_second_buffer)
      {
        _return_to_stand_buffer();
      }
    }
    else if (bytesRead == -1)
    {
      INFO_PRINT("FIFO error, reopening...\n");
      sleep(1);
    }
  }
}
