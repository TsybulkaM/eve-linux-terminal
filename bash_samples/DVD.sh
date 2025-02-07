#!/bin/bash

PIPE="/tmp/eve_pipe"
WIDTH=480
HEIGHT=272

TEXT="DEMO"
TEXT_WIDTH=$(( ${#TEXT}*20 ))
TEXT_HEIGHT=35
FONT_SIZE=30

X=10
Y=10
DX=2
DY=1

generate_random_color() {
    R=$(( RANDOM % 256 ))
    G=$(( RANDOM % 256 ))
    B=$(( RANDOM % 256 ))
}

generate_random_color

echo "clear" > "$PIPE"

while true; do
    echo "custText $X $Y $FONT_SIZE $R $G $B $TEXT" > "$PIPE"
    sleep 0.01

    ((X+=DX))
    ((Y+=DY))

    if (( X + TEXT_WIDTH >= WIDTH || X <= 0 )); then
        DX=$(( -DX ))
        generate_random_color
    fi
    if (( Y + TEXT_HEIGHT >= HEIGHT || Y <= 0 )); then
        DY=$(( -DY ))
        generate_random_color
    fi
done