# EVE Controller Command Guide

## Overview

This guide provides commands for interacting with the EVE controller to display text and manage the screen.

---

## Precondition

In folder need to be: 

``` bash
> eveld_43_480x272_EVE3_TPC
> install.sh
> uninstall.sh
```

### Installation

Download installer:

``` bash
curl -L -o install.sh https://github.com/TsybukloN/EVE-project/releases/latest/download/install.sh
```

Run the installation script:

``` bash
sudo bash install.sh
```

### Update

Delete previous binary file:

```bash
rm -rf eveld_43_480x272_EVE3_TPC
```

Run the installation script:

``` bash
sudo bash install.sh
```

New binary pull up from latest GitHub Release.

### Uninstallation

Download installer:

``` bash
curl -L -o install.sh https://github.com/TsybukloN/EVE-project/releases/latest/download/uninstall.sh
```

Run the installation script:

``` bash
sudo bash uninstall.sh
```


## Commands

### Realised 

```bash
TERM=linux stty cols 54 rows 17; top | cat -v > /tmp/eve_pipe
```

### Known Issues

The following commands execute but may not work as expected:

```bash
TERM=linux stty cols 54 rows 16; htop | cat -v > /tmp/eve_pipe
```

### Clear Screen 
To clear the screen:
```bash
clear | cat -v > /tmp/eve_pipe
```

### Test commands

#### Text

```bash
echo "AC/DC" | cat -v > /tmp/eve_pipe
```

```bash
echo {A..Z} | tr -d ' ' > /tmp/eve_pipe
```

```bash
echo {a..z} | tr -d ' ' > /tmp/eve_pipe
```

#### Formated Text

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

#### System commands 

```bash
TERM=linux stty cols 57 rows 15; df -h | cat -v > /tmp/eve_pipe
```
Disks usage

## Planned Features

```bash
TERM=linux stty cols 54 rows 15; sl | cat -v > /tmp/eve_pipe
```

```bash
TERM=linux stty cols 54 rows 15; cmatrix | cat -v > /tmp/eve_pipe
```
