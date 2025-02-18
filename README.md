# EVE Controller Command Guide

This guide provides commands for interacting with the EVE controller to display text and manage the screen.

---

## Precondiution

Run all comand from root user!

## Start the Pipe

To start the application, run:

```bash
./cli_controller_43_480x272_EVE3_TPC
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
echo -e "\e[31mRed\e[m\e[31mStandart." | cat -v > /tmp/eve_pipe
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
clear | cat -v > /tmp/eve_pipe && df -h | cat -v > /tmp/eve_pipe
```
Disks usage


### Clear Screen
To clear the screen:
```bash
clear | cat -v > /tmp/eve_pipe
```

---

### TODO 

```bash
top | head -n 1 | cat -v > /tmp/eve_pipe
```

```bash
TERM=linux stty cols 57 rows 10; top | cat -v > /tmp/eve_pipe
```

```bash
htop | head -n 1 | cat -v > /tmp/eve_pipe
```

```bash
TERM=linux stty cols 49 rows 10; htop | cat -v > /tmp/eve_pipe
```

TERM=linux stty cols 55 rows 10; while true; do clear | cat -v > /tmp/eve_pipe; top -n 3 | cat -v > /tmp/eve_pipe; sleep 1; done
