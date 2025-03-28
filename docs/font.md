# Font convertation

.glyph.h - chars' data
.xfont.h - size of font

To convert binary font's information to bit array:

```bash
xxd -i *.glyph > Font_16.glyph.h
xxd -i *.xfont > Font_16.xfont.h
```
