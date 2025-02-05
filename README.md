# EVE Controller Command Guide

This guide provides commands for interacting with the EVE controller to display text and manage the screen.

---

## Start the Pipe

To start the application, run:

```bash
sudo ./cli_controller_43_480x272_EVE3_TPC
```

## Commands

### Text
By default, the text will be printed with a font size of 30 and in white color.
```bash
echo "text <start_x> <start_y> <text>" > /tmp/eve_pipe
```
Example:
```bash
echo "text 10 0 AC/DC" > /tmp/eve_pipe
```

### Formated Text
You can print text with a custom font size and color.
```bash
echo "custText <start_x> <start_y> <font_size> <COLOR_R> <COLOR_G> <COLOR_B> <text>" > /tmp/eve_pipe
```
Example:
```bash
echo "custText 10 0 21 159 31 39 AC/DC" > /tmp/eve_pipe
```
This will print "AC/DC" in red color with a font size of 30 at coordinates (10, 0).

### Clear Screen
To clear the screen:
```bash
echo "clear" > /tmp/eve_pipe
```

---

This version is more organized with headings and explanations for each command. It also includes an example with the formatted text and color, which should make it clearer for users.
