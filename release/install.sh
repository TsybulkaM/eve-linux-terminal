#!/bin/bash

set -e

EVE_BINARY="eveld_43_480x272_EVE3_TPC"
VENDOR_ID="1b3d"
PRODUCT_ID="0200"
GITHUB_RELEASE_URL="https://github.com/TsybukloN/EVE-project/releases/latest/download/eveld_43_480x272_EVE3_TPC"

echo "Installing EVE device support..."

# --- 0. Binary ---
if [ ! -f "$EVE_BINARY" ]; then
  if command -v wget &>/dev/null; then
    sudo wget -O "$EVE_BINARY" "$GITHUB_RELEASE_URL"
  elif command -v curl &>/dev/null; then
    sudo curl -L -o "$EVE_BINARY" "$GITHUB_RELEASE_URL"
  else
    echo "Error: wget and curl are not installed. Please install one of them."
    exit 1
  fi
else
  echo "Given binary already exists."
fi

cp $EVE_BINARY /usr/local/bin
chmod +x /usr/local/bin/$EVE_BINARY

# --- 1. Create systemd-service ---
SYSTEMD_SERVICE="/etc/systemd/system/eveld.service"
sudo bash -c "cat > $SYSTEMD_SERVICE" <<EOF
[Unit]
Description=Eve service handler (%i)
After=network.target

[Service]
ExecStart=/usr/local/bin/$EVE_BINARY
Restart=always
User=root
Group=root

[Install]
WantedBy=multi-user.target
EOF

# --- 2. Create udev-rule ---
UDEV_RULE="/etc/udev/rules.d/99-eve.rules"
sudo bash -c "cat > $UDEV_RULE" <<EOF
ACTION=="add", SUBSYSTEM=="usb", ATTR{idVendor}=="$VENDOR_ID", ATTR{idProduct}=="$PRODUCT_ID", \
  TAG+="systemd", RUN+="/bin/systemctl restart eveld"
EOF

# --- 3. Restarting services ---
sudo systemctl daemon-reload
sudo udevadm control --reload-rules

echo "The installation is complete! Now when you connect EVE the process will start automatically."
