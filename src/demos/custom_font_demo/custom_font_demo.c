#ifdef _MSC_VER
#include <conio.h>
#endif

#include "Ubuntu_16.glyph.h"
#include "Ubuntu_16.xfont.h"
#include "Ubuntu_20.glyph.h"
#include "Ubuntu_20.xfont.h"

#include "eve.h"
#include "hw_api.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK_SIZE 4096

uint32_t font2_xfont_addr = 0;

void MakeScreen_text()
{
    Send_CMD(CMD_DLSTART);
    Send_CMD(VERTEXFORMAT(0));
    Send_CMD(CLEAR_COLOR_RGB(0, 0, 0));
    Send_CMD(CLEAR(1, 1, 1));

    // Устанавливаем шрифты
    Cmd_SetFont2(1, RAM_G, 0);
    Cmd_SetFont2(2, font2_xfont_addr, 0);

    Send_CMD(COLOR_RGB(255, 255, 255));

    Cmd_Text_Codepoints(100, 100, 1, OPT_CENTER, "Less");
    Cmd_Text_Codepoints(100, 150, 2, OPT_CENTER, "More");

    Send_CMD(DISPLAY());
    Send_CMD(CMD_SWAP);
    UpdateFIFO();
}

int main()
{
    if (EVE_Init(DEMO_DISPLAY, DEMO_BOARD, DEMO_TOUCH) <= 1)
    {
        printf("ERROR: Eve not detected.\n");
        return -1;
    }

    uint32_t offset = RAM_G;

    // Загружаем первый шрифт (16px)
    StartCoProTransfer(offset, 0);
    HAL_SPI_WriteBuffer((uint8_t *)UbuntuMono_Regular_16_ASTC_xfont, UbuntuMono_Regular_16_ASTC_xfont_len);
    HAL_SPI_Disable();
    offset += CHUNK_SIZE;

    StartCoProTransfer(offset, 0);
    HAL_SPI_WriteBuffer((uint8_t *)UbuntuMono_Regular_16_ASTC_glyph, UbuntuMono_Regular_16_ASTC_glyph_len);
    HAL_SPI_Disable();
    offset += UbuntuMono_Regular_16_ASTC_glyph_len;

    // Выравниваем offset до границы CHUNK_SIZE
    if (offset % CHUNK_SIZE != 0) {
        offset += CHUNK_SIZE - (offset % CHUNK_SIZE);
    }

    // Загружаем данные глифов для второго шрифта (20px)
    uint32_t glyph_offset = offset + CHUNK_SIZE; // Место для xfont структуры
    StartCoProTransfer(glyph_offset, 0);
    HAL_SPI_WriteBuffer((uint8_t *)Ubuntu_Italic_20_ASTC_glyph, Ubuntu_Italic_20_ASTC_glyph_len);
    HAL_SPI_Disable();

    // Создаем копию xfont структуры и модифицируем указатель
    uint8_t xfont_copy[Ubuntu_Italic_20_ASTC_xfont_len];
    memcpy(xfont_copy, Ubuntu_Italic_20_ASTC_xfont, Ubuntu_Italic_20_ASTC_xfont_len);
    
    // Устанавливаем указатель на данные глифов (смещение 144 в структуре)
    xfont_copy[144] = glyph_offset & 0xFF;
    xfont_copy[145] = (glyph_offset >> 8) & 0xFF;
    xfont_copy[146] = (glyph_offset >> 16) & 0xFF;
    xfont_copy[147] = (glyph_offset >> 24) & 0xFF;

    // Загружаем модифицированную xfont структуру
    font2_xfont_addr = offset;
    StartCoProTransfer(offset, 0);
    HAL_SPI_WriteBuffer(xfont_copy, Ubuntu_Italic_20_ASTC_xfont_len);
    HAL_SPI_Disable();

    MakeScreen_text();

    HAL_Close();
    return 0;
}