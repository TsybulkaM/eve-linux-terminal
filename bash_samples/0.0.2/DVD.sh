#!/bin/bash

trap "" SIGPIPE

rows=480
cols=272

fifo=/tmp/eve_pipe

x=1
y=1

dx=1
dy=1

colors=(31 32 33 34 35 36 91 92 93 94 95 96)
color_index=0

while true; do
    echo -ne "\e[${y};${x}H"

    echo -ne "\e[1;${colors[$color_index]}mDVD\e[0m"

    sleep 0.05

    echo -ne "\e[${y};${x}H   "

    ((x += dx))
    ((y += dy))

    if ((x <= 1 || x >= cols - 3)); then
        dx=$(( -dx ))
        color_index=$(( (color_index + 1) % ${#colors[@]} ))
    fi

    if ((y <= 1 || y >= rows - 1)); then
        dy=$(( -dy ))
        color_index=$(( (color_index + 1) % ${#colors[@]} ))
    fi
done
