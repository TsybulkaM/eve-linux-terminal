# EVE Controller Command Guide

This guide provides commands for interacting with the EVE controller to display text and manage the screen.

---

## Precondition

sudo 

## Start the Pipe

To start the application, run:

```bash
./eveld_43_480x272_EVE3_TPC
```

## Commands

### Text

```bash
echo "AC/DC" | cat -v > /tmp/eve_pipe
```

```bash
echo {A..Z} | tr -d ' ' > /tmp/eve_pipe
```

```bash
echo {a..z} | tr -d ' ' > /tmp/eve_pipe
```

### Formated Text

```bash
echo -e "\e[31mRed\e[mStandart." | cat -v > /tmp/eve_pipe
```

```bash
echo "Hello, Eve!" | pv -qL 10 | cat -v > /tmp/eve_pipe
```
This will print "Hello, World!" with delay.

```bash
cowsay "Hello, Eve!" | lolcat | cat -v > /tmp/eve_pipe
```
Cowsay ASCII-art

### System commands 

```bash
TERM=linux stty cols 57 rows 15; df -h | cat -v > /tmp/eve_pipe
```
Disks usage

### Clear Screen 
To clear the screen:
```bash
clear | cat -v > /tmp/eve_pipe
```

---

### Realised 

```bash
TERM=linux stty cols 54 rows 17; top | cat -v > /tmp/eve_pipe
```

### TODO 

```bash
TERM=linux stty cols 54 rows 16; htop | cat -v > /tmp/eve_pipe
TERM=linux stty cols 54 rows 16; cbonsai | cat -v > /tmp/eve_pipe
```

```bash
TERM=linux stty cols 54 rows 15; sl | cat -v > /tmp/eve_pipe
```

```bash
TERM=linux stty cols 54 rows 15; cmatrix | cat -v > /tmp/eve_pipe
```
