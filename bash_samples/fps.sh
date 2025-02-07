#!/bin/bash

FIFO="/tmp/eve_pipe"
FONT_SIZE=30
TEXT_X=10
TEXT_Y=10
COLOR_R=255
COLOR_G=255
COLOR_B=255

echo "clear" > "$FIFO"
sleep 0.1

frame_count=0
start_time=$(date +%s)

while true; do
    current_time=$(date +%s)
    elapsed=$((current_time - start_time))

    if [ "$elapsed" -ge 1 ]; then
        echo "FPS: $frame_count"
        
        frame_count=0
        start_time=$current_time
    fi

    echo "text $TEXT_X $TEXT_Y FPS: $frame_count" > "$FIFO"
    ((frame_count++))
done
