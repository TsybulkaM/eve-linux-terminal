#ifdef _MSC_VER
#include <conio.h>
#endif
#include "Ubuntu_16_bold.glyph.h"
#include "Ubuntu_16_bold.xfont.h"
#include "eve.h"
#include "hw_api.h"
#include <stdint.h>
#include <stdlib.h>


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

  Cmd_SetFont2(1, RAM_G, 0);

  Send_CMD(COLOR_RGB(255, 255, 255));

  Cmd_Text_Codepoints(100, 100, 1, OPT_CENTER, u8"функция");

  // End the display list
  Send_CMD(DISPLAY());
  // Swap commands into RAM
  Send_CMD(CMD_SWAP);
  // Trigger the CoProcessor to start processing the FIFO
  UpdateFIFO();
}

void MakeScreen_ABC()
{
  // Start a new display list
  Send_CMD(CMD_DLSTART);
  // Setup VERTEX2F to take pixel coordinates
  Send_CMD(VERTEXFORMAT(0));
  // Set the clear screen color
  Send_CMD(CLEAR_COLOR_RGB(0, 0, 0));
  // Clear the screen
  Send_CMD(CLEAR(1, 1, 1));

  Cmd_SetFont2(1, RAM_G, 0);

  Send_CMD(COLOR_RGB(255, 255, 255));

  Cmd_Text_Codepoints(100, 100, 1, OPT_CENTER, u8"ABC");

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
  HAL_SPI_WriteBuffer((uint8_t *)&Ubuntu_Bold_16_ASTC_xfont, Ubuntu_Bold_16_ASTC_xfont_len);
  HAL_SPI_Disable();

  StartCoProTransfer(RAM_G + 4096, 0);
  HAL_SPI_WriteBuffer((uint8_t *)&Ubuntu_Bold_16_ASTC_glyph, Ubuntu_Bold_16_ASTC_glyph_len);
  HAL_SPI_Disable();

  MakeScreen_HelloWorld();
  HAL_Delay(2000);
  MakeScreen_ABC();

  HAL_Close();
}
