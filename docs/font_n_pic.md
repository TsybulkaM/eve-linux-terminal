# Font and Pic convertation

.glyph.h - chars' data
.xfont.h - size of font

To convert binary font's information to bit array:

```bash
for file in *.glyph; do xxd -i "$file" > "$file.h"; done
for file in *.xfont; do xxd -i "$file" > "$file.h"; done
```

Also it works with `.png`: `xxd -i logo.png > logo.h`

