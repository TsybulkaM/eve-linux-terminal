#!/bin/bash

UDEV_RULE="/etc/udev/rules.d/99-spi2usb.rules"
sudo bash -c "cat > $UDEV_RULE" <<EOF
ACTION=="add", SUBSYSTEM=="usb", ATTR{idVendor}=="1b3d", ATTR{idProduct}=="0200",  MODE="0666"
EOF

sudo udevadm control --reload-rules
sudo udevadm trigger