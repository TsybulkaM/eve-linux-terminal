
## Dependents

### pkg-config

For Debian/Ubuntu (or Raspberry Pi OS):
```bash
sudo apt update
sudo apt install pkg-config
```

For Arch Linux (or Manjaro):
```bash
sudo pacman -S pkgconf
```

For Fedora:
```bash
sudo dnf install pkgconf
```

### libftdi1

For Debian/Ubuntu:
```bash
sudo apt install libftdi1-dev
```
For Arch Linux (or Manjaro):
```bash
sudo pacman -S libftdi
```

For Fedora:
```bash
sudo dnf install libftdi-devel
```

## Run binary for usual user

```bash
sudo nano /etc/udev/rules.d/99-spi2usb.rules
SUBSYSTEM=="usb", ATTRS{idVendor}=="1b3d", ATTRS{idProduct}=="0200", MODE="0666"
sudo udevadm control --reload-rules
sudo udevadm trigger
```
