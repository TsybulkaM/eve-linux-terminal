#ifdef _MSC_VER
#include <conio.h>
#endif
#include "JetBrainsMono-Italic-VariableFont_wght_16_ASTC.glyph.h"
#include "JetBrainsMono-Italic-VariableFont_wght_16_ASTC.xfont.h"
#include "eve.h"
#include "hw_api.h"
#include <stdint.h>
#include <stdlib.h>


size_t utf8_to_unicode_array(const char* utf8_str, uint16_t** unicode_array) {
  size_t len = 0;
  size_t arr_size = 16;
  *unicode_array = malloc(arr_size * sizeof(uint16_t));
  
  while (*utf8_str) {
      uint32_t code_point = 0;
      uint8_t first_byte = (uint8_t)*utf8_str++;
      
      if ((first_byte & 0x80) == 0) {
          // 1-байтовый символ (ASCII)
          code_point = first_byte;
      } else if ((first_byte & 0xE0) == 0xC0) {
          // 2-байтовый символ
          code_point = (first_byte & 0x1F) << 6;
          code_point |= (*utf8_str++ & 0x3F);
      } else if ((first_byte & 0xF0) == 0xE0) {
          // 3-байтовый символ
          code_point = (first_byte & 0x0F) << 12;
          code_point |= (*utf8_str++ & 0x3F) << 6;
          code_point |= (*utf8_str++ & 0x3F);
      }

      if (len >= arr_size) {
          arr_size *= 2;
          *unicode_array = realloc(*unicode_array, arr_size * sizeof(uint16_t));
      }
      
      (*unicode_array)[len++] = (uint16_t)code_point;
  }
  
  return len;
}

void MakeScreen_HelloWorld()
{
  // Start a new display list
  Send_CMD(CMD_DLSTART);
  // Setup VERTEX2F to take pixel coordinates
  Send_CMD(VERTEXFORMAT(0));
  // Set the clear screen color
  Send_CMD(CLEAR_COLOR_RGB(0, 0, 0));
  // Clear the screen
  Send_CMD(CLEAR(1, 1, 1));
  Send_CMD(COLOR_RGB(255, 255, 255));
  // Select the custom font for font 1, the xfont data is written at RAM_G
  Cmd_SetFont2(1, RAM_G, 0);

  const char* russian_utf8 = u8"1234567890абвгдеёжзийклмнопрстуфхцчшщъыьэюя";
  uint16_t* unicode_chars;
  size_t char_count = utf8_to_unicode_array(russian_utf8, &unicode_chars);

  Cmd_Text_Codepoints(100, 100, 1, OPT_CENTER, unicode_chars, char_count);

  free(unicode_chars);
  // End the display list
  Send_CMD(DISPLAY());
  // Swap commands into RAM
  Send_CMD(CMD_SWAP);
  // Trigger the CoProcessor to start processing the FIFO
  UpdateFIFO();
}

int main()
{
  if (EVE_Init(DEMO_DISPLAY, DEMO_BOARD, DEMO_TOUCH) <= 1)
  {
    printf("ERROR: Eve not detected.\n");
    return -1;
  }

  StartCoProTransfer(RAM_G, 0);
  HAL_SPI_WriteBuffer((uint8_t *)&JetBrainsMono_Italic_VariableFont_wght_16_ASTC_xfont,
                      JetBrainsMono_Italic_VariableFont_wght_16_ASTC_xfont_len);
  HAL_SPI_Disable();

  StartCoProTransfer(RAM_G + 4096, 0);
  HAL_SPI_WriteBuffer((uint8_t *)&JetBrainsMono_Italic_VariableFont_wght_16_ASTC_glyph,
                      JetBrainsMono_Italic_VariableFont_wght_16_ASTC_glyph_len);
  HAL_SPI_Disable();

  MakeScreen_HelloWorld();

  HAL_Close();
}
