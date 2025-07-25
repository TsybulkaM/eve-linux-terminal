#include "eveld.h"

void PrepareScreen()
{
  Send_CMD(CMD_DLSTART);
  Send_CMD(VERTEXFORMAT(0));
  Send_CMD(CLEAR_COLOR_RGB(0, 0, 0));
  Send_CMD(CLEAR(1, 1, 1));
  uint32_t offset = RAM_G;
  for (int i = 0; i < fonts_len; i++)
  {
    DEBUG_PRINT("Font %d setting to %d\n", fonts[i].id, offset);
    Cmd_SetFont2(fonts[i].id, offset, 0);
    offset += CHANK_SIZE + fonts[i].glyph_size;

    UpdateFIFO();
    Wait4CoProFIFOEmpty();
  }
}

void DisplayFrame()
{
  Send_CMD(DISPLAY());
  Send_CMD(CMD_SWAP);
  UpdateFIFO();
  Wait4CoProFIFOEmpty();
}

void ClearScreen(void)
{
  DisplayFrame();
  staticTextCount = 0;
}

void ResetScreen(void)
{
  ClearScreen();
  PrepareScreen();
}

int GetTextWidth(const char *str, uint8_t font, int max_chars)
{
  if (!str)
    return 0;

  int width = 0;
  int chars_processed = 0;

  while (*str && (max_chars < 0 || chars_processed < max_chars))
  {
    uint8_t c = (uint8_t)*str;
    uint32_t codepoint = 0;
    int char_len = 1;

    if (c < 0x80)
    {
      codepoint = c;
    }
    else if ((c & 0xE0) == 0xC0)
    {
      codepoint = ((str[0] & 0x1F) << 6) | (str[1] & 0x3F);
      char_len = 2;
    }
    else if ((c & 0xF0) == 0xE0)
    {
      codepoint = ((str[0] & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
      char_len = 3;
    }
    else if ((c & 0xF8) == 0xF0)
    {
      codepoint = ((str[0] & 0x07) << 18) | ((str[1] & 0x3F) << 12) | ((str[2] & 0x3F) << 6) |
                  (str[3] & 0x3F);
      char_len = 4;
    }

    str += char_len;
    width += get_font_by_id(font)->width;
    chars_processed++;
  }

  return width;
}

size_t utf8_char_length(uint8_t first_byte)
{
  if ((first_byte & 0x80) == 0)
    return 1; // 0xxxxxxx (ASCII)
  if ((first_byte & 0xE0) == 0xC0)
    return 2; // 110xxxxx
  if ((first_byte & 0xF0) == 0xE0)
    return 3; // 1110xxxx
  if ((first_byte & 0xF8) == 0xF0)
    return 4; // 11110xxx
  return 1;   // Invalid byte, treat as 1-byte character
}

const char *utf8_nth_char(const char *str, int index)
{
  int count = 0;
  while (*str && count < index)
  {
    uint8_t c = (uint8_t)*str;
    str += utf8_char_length(c);
    count++;
  }

  return *str ? str : NULL;
}

bool colors_are_equal(Color a, Color b)
{
  return a.r == b.r && a.g == b.g && a.b == b.b;
}

void SetActualNewLine(uint16_t line)
{
  DEBUG_PRINT("New line, y = %d x = %d\n", actual_word.y, actual_word.x);
  actual_word.line = line;
}

void AppendCharToActualWord(const char *bytes_to_append, size_t num_bytes)
{
  uint8_t actual_word_width = GetTextWidth(bytes_to_append, actual_word.font, 1);
  if (actual_word_bytes + num_bytes < MAX_LENGTH &&
      actual_word.width + actual_word_width <= Display_Width())
  {
    memcpy(&actual_word.text[actual_word_bytes], bytes_to_append, num_bytes);

    actual_word_bytes += num_bytes;
    actual_word.text[actual_word_bytes] = '\0';

    actual_word.symbol_len++;
    actual_word.width += actual_word_width;
  }
  else
  {
    ERROR_PRINT("Error during append char: maximum length reached\n");
  }
}

void DeleteCharH(void)
{
  if (actual_word_bytes > 0)
  {
    uint8_t last_char_width = GetTextWidth(
        utf8_nth_char(actual_word.text, actual_word.symbol_len - 1), actual_word.font, 1);
    actual_word.width -= last_char_width;
    actual_word.x -= last_char_width;
    actual_word_bytes--;
  }
  else
  {
    // TODO: handle case when there are no characters to delete
    /*
    StaticText* last = &staticTexts[staticTextCount - 1];
    uint8_t last_char_width = GetTextWidth(utf8_nth_char(last->text, last->symbol_len - 1), 1);
    staticTexts[staticTextCount - 1].width = last_char_width;
    staticTexts[staticTextCount - 1].text[strlen(last->text)] = '\0';*/
  }
}

void DrawStaticTexts(void)
{
  for (int i = 0; i < staticTextCount; i++)
  {

    if (!colors_are_equal(staticTexts[i].bg_color,
                          (Color){DEFAULT_COLOR_BG_R, DEFAULT_COLOR_BG_G, DEFAULT_COLOR_BG_B}))
    {

      Send_CMD(COLOR_RGB(
          staticTexts[i].bg_color.r, staticTexts[i].bg_color.g, staticTexts[i].bg_color.b));

      Send_CMD(BEGIN(RECTS));
      Send_CMD(VERTEX2F(staticTexts[i].x,
                        staticTexts[i].y + get_font_by_id(staticTexts[i].font)->width - 9));

      Send_CMD(
          VERTEX2F(staticTexts[i].x + GetTextWidth(staticTexts[i].text, staticTexts[i].font, -1),
                   staticTexts[i].y + get_font_by_id(staticTexts[i].font)->height));
      Send_CMD(END());
    }

    Send_CMD(COLOR_RGB(
        staticTexts[i].text_color.r, staticTexts[i].text_color.g, staticTexts[i].text_color.b));

    Cmd_Text(staticTexts[i].x,
                  staticTexts[i].y,
                  staticTexts[i].font,
                  DEFAULT_OPTION,
                  staticTexts[i].text);
  }

  DEBUG_PRINT("staticTextCount: %d\n", staticTextCount);
  DisplayFrame();
}

void DeleteChatH(uint16_t count)
{
  for (int i = 0; i < staticTextCount; i++)
  {
    if (staticTexts[i].line != actual_word.line || staticTexts[i].x <= actual_word.x)
    {
      staticTexts[i].x -= count * get_font_by_id(staticTexts[i].font)->width;
    }
  }
}

void ClearLine(void)
{
  int j = 0;
  for (int i = 0; i < staticTextCount; i++)
  {
    if (staticTexts[i].line != actual_word.line)
    {
      staticTexts[j++] = staticTexts[i];
    }
    else
    {
      DEBUG_PRINT("Cleared Line %d: %s\n", staticTexts[i].line, staticTexts[i].text);
    }
  }
  staticTextCount = j;
}

void ClearLineAfterX(void)
{
  int j = 0;
  for (int i = 0; i < staticTextCount; i++)
  {
    if (staticTexts[i].line != actual_word.line || staticTexts[i].x <= actual_word.x)
    {
      staticTexts[j++] = staticTexts[i];
    }
    else
    {
      DEBUG_PRINT("Cleared After X=%d: %s\n", actual_word.x, staticTexts[i].text);
    }
  }
  staticTextCount = j;
}

void ClearAfterX(void)
{
  int j = 0;
  for (int i = 0; i < staticTextCount; i++)
  {
    if (staticTexts[i].line < actual_word.line ||
        (staticTexts[i].line == actual_word.line && staticTexts[i].x < actual_word.x))
    {
      staticTexts[j++] = staticTexts[i];
    }
    else
    {
      DEBUG_PRINT("Cleared After X=%d: %s\n", actual_word.x, staticTexts[i].text);
    }
  }
  staticTextCount = j;
}

void ClearLineBeforeX(void)
{
  int j = 0;
  for (int i = 0; i < staticTextCount; i++)
  {
    if (staticTexts[i].line != actual_word.line || staticTexts[i].x > actual_word.x)
    {
      staticTexts[j++] = staticTexts[i];
    }
    else
    {
      DEBUG_PRINT("Cleared Before X=%d: %s\n", actual_word.x, staticTexts[i].text);
    }
  }
  staticTextCount = j;
}

void ClearBeforeX(void)
{
  int j = 0;
  for (int i = 0; i < staticTextCount; i++)
  {
    if (staticTexts[i].line > actual_word.line ||
        (staticTexts[i].line == actual_word.line && staticTexts[i].x > actual_word.x))
    {
      staticTexts[j++] = staticTexts[i];
    }
    else
    {
      DEBUG_PRINT("Cleared Before X=%d: %s\n", actual_word.x, staticTexts[i].text);
    }
  }
  staticTextCount = j;
}

int GetTextOffset(StaticText *word, int xPos)
{
  int offset = 0, px = word->x;
  while (px < xPos && word->text[offset] != '\0')
  {
    int charWidth = GetTextWidth(utf8_nth_char(word->text, offset), word->font, 1);
    if (px + charWidth > xPos)
    {
      break;
    }
    px += charWidth;
    offset++;
  }
  return offset;
}

StaticText CreateSubText(StaticText src, int newX, int newWidth)
{
  StaticText subText = src;
  subText.x = newX;
  subText.width = newWidth;

  int charOffset = GetTextOffset(&src, newX);
  if (charOffset >= strlen(src.text))
  {
    subText.text[0] = '\0';
    return subText;
  }

  int px = newX;
  int textLen = 0;
  for (int i = charOffset; src.text[i] != '\0' && px < newX + newWidth; i++)
  {
    int charWidth = GetTextWidth(utf8_nth_char(src.text, i), src.font, -1);
    if (px + charWidth > newX + newWidth)
    {
      break;
    }
    subText.text[textLen++] = src.text[i];
    px += charWidth;
  }
  subText.text[textLen] = '\0';

  if (textLen == 0)
  {
    subText.text[0] = '\0';
  }
  return subText;
}

bool IsOnlySpaces(const char *text)
{
  for (int i = 0; text[i] != '\0'; i++)
  {
    if (text[i] != ' ')
    {
      return false;
    }
  }
  return true;
}

void AddOrMergeActualTextStatic(void)
{
  if (actual_word.symbol_len == 0)
  {
    DEBUG_PRINT("Skipping empty text\n");
    return;
  }

  actual_word.text[actual_word_bytes] = '\0';

  if (IsOnlySpaces(actual_word.text))
  {
    actual_word.x += actual_word.width;
    DEBUG_PRINT("Text contains only spaces, moving x to %d\n", actual_word.x);
    actual_word.symbol_len = 0;
    actual_word_bytes = 0;
    actual_word.width = 0;
    return;
  }

  actual_word.y = min(actual_word.y, Display_Height() - get_font_by_id(actual_word.font)->height);
  actual_word.width = min(actual_word.width, Display_Width() - actual_word.x);

  DEBUG_PRINT("Actual word: '%s' at [%d, %d] width: %d, font: %d\n",
              actual_word.text,
              actual_word.x,
              actual_word.y,
              actual_word.width,
              actual_word.font);

  StaticText newStaticTexts[MAX_STATIC_TEXTS];
  int newCount = 0;
  StaticText *intersectingWords[MAX_STATIC_TEXTS];
  int intersectCount = 0;

  for (int i = 0; i < staticTextCount; i++)
  {
    StaticText *word = &staticTexts[i];

    if (word->line != actual_word.line || word->x + word->width <= actual_word.x ||
        word->x >= actual_word.x + actual_word.width)
    {
      newStaticTexts[newCount++] = *word;
    }
    else
    {
      DEBUG_PRINT("Intersecting word: '%s' at [%d, %d] width: %d\n",
                  word->text,
                  word->x,
                  word->y,
                  word->width);
      intersectingWords[intersectCount++] = word;
    }
  }

  if (intersectCount == 0)
  {
    DEBUG_PRINT("No intersections, adding new word: '%s'\n", actual_word.text);
    newStaticTexts[newCount++] = actual_word;
  }
  else
  {
    StaticText *word;
    bool merged = false;
    for (int i = 0; i < intersectCount; i++)
    {
      word = intersectingWords[i];

      if (word->font == actual_word.font &&
          colors_are_equal(word->text_color, actual_word.text_color) &&
          colors_are_equal(word->bg_color, actual_word.bg_color))
      {

        DEBUG_PRINT("Merging text '%s' with '%s'\n", word->text, actual_word.text);

        uint8_t actual_index_start = 0;
        uint16_t current_x = word->x;
        uint8_t word_length = strlen(word->text);

        if (word->x + word->width == actual_word.x)
        {
          actual_index_start = word_length;
          current_x = word->x + word->width;
        }
        else
        {
          for (int j = 0; j < word_length; j++)
          {
            if (current_x >= actual_word.x)
            {
              break;
            }
            current_x += GetTextWidth(utf8_nth_char(word->text, j), word->font, 1);
            actual_index_start++;
          }
        }

        if (actual_index_start + actual_word_bytes > MAX_LENGTH)
        {
          ERROR_PRINT("Text merge out of bounds\n");
          return;
        }

        if (actual_index_start < MAX_LENGTH)
        {
          DEBUG_PRINT("actual_index_start: %d\n", actual_index_start);

          int max_copy_len = min(actual_word_bytes, MAX_LENGTH - actual_index_start);

          for (int e = 0; e < max_copy_len; e++)
          {
            word->text[actual_index_start + e] = actual_word.text[e];
          }

          int new_length = max(word_length, actual_index_start + max_copy_len);

          if (new_length < MAX_LENGTH)
          {
            word->text[new_length] = '\0';
          }
          else
          {
            word->text[MAX_LENGTH - 1] = '\0';
          }
        }

        word->x = min(word->x, actual_word.x);
        word->width = min(GetTextWidth(word->text, word->font, -1), Display_Width() - word->x);
        // actual_word.x = word->x + word->width;

        DEBUG_PRINT("Merged result: '%s' at [%d, %d] width: %d\n",
                    word->text,
                    word->x,
                    word->y,
                    word->width);

        merged = true;
      }
      else
      {
        DEBUG_PRINT("Splitting word: '%s' at [%d, %d]\n", word->text, word->x, word->y);

        StaticText leftPart = CreateSubText(*word, word->x, actual_word.x - word->x);
        StaticText rightPart =
            CreateSubText(*word,
                          actual_word.x + actual_word.width,
                          (word->x + word->width) - (actual_word.x + actual_word.width));

        if (leftPart.text[0] != '\0')
        {
          DEBUG_PRINT("Adding left part: '%s' at [%d, %d] width: %d\n",
                      leftPart.text,
                      leftPart.x,
                      leftPart.y,
                      leftPart.width);
          newStaticTexts[newCount++] = leftPart;
        }

        if (rightPart.text[0] != '\0')
        {
          DEBUG_PRINT("Adding right part: '%s' at [%d, %d] width: %d\n",
                      rightPart.text,
                      rightPart.x,
                      rightPart.y,
                      rightPart.width);
          newStaticTexts[newCount++] = rightPart;
        }
      }
    }

    if (merged)
    {
      newStaticTexts[newCount++] = *word;
    }
    else
    {
      DEBUG_PRINT("Adding new word: '%s' at [%d, %d] width: %d\n",
                  actual_word.text,
                  actual_word.x,
                  actual_word.y,
                  actual_word.width);
      newStaticTexts[newCount++] = actual_word;
    }
  }

  if (newCount > MAX_STATIC_TEXTS)
  {
    ERROR_PRINT("Too many static texts! Possible memory corruption, clearing line\n");
    ClearLine();
    return;
  }

  memcpy(staticTexts, newStaticTexts, newCount * sizeof(StaticText));
  staticTextCount = newCount;

  actual_word.x += actual_word.width;
  actual_word_bytes = 0;
  actual_word.width = 0;
}