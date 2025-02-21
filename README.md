# EVE Controller Command Guide

This guide provides commands for interacting with the EVE controller to display text and manage the screen.

---

## Precondiution

Run all comand from root user or run following commands:

```bash
sudo nano /etc/udev/rules.d/99-spi2usb.rules
SUBSYSTEM=="usb", ATTRS{idVendor}=="1b3d", ATTRS{idProduct}=="0200", MODE="0666"
sudo udevadm control --reload-rules
sudo udevadm trigger
```

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
TERM=linux stty cols 55 rows 15; top | cat -v > /tmp/eve_pipe
```

```bash
htop | head -n 1 | cat -v > /tmp/eve_pipe
```

```bash
TERM=linux stty cols 55 rows 15; htop | cat -v > /tmp/eve_pipe
```
