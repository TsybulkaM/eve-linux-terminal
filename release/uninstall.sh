#!/bin/bash

sudo systemctl stop eveld
sudo systemctl disable eveld

sudo rm -f /usr/local/bin/eveld_43_480x272_EVE3_TPC

sudo rm -f /etc/systemd/system/eveld.service
sudo systemctl daemon-reload

sudo rm -f /etc/udev/rules.d/99-eve.rules
sudo udevadm control --reload-rules
