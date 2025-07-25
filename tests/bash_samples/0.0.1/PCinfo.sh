#!/bin/bash

FIFO_PATH="/tmp/eve_pipe"

send_command() {
    echo "$1" > "$FIFO_PATH"
}

send_command "clear"

sleep 0.1

send_command "staticText 20 20 30 255 255 0 CPU Load:&staticText 20 60 30 255 255 0 RAM Usage:&staticText 20 100 30 255 255 0 CPU Temp:&staticText 20 140 30 255 255 0 Uptime:&staticText 20 180 30 255 255 0 Time:" 


while true; do
    CPU_LOAD=$(top -bn1 | grep "Cpu(s)" | awk '{print 100 - $8"%"}')
    RAM_USAGE=$(free -m | awk '/Mem:/ {print $3 "MB / " $2 "MB"}')
    CPU_TEMP=$(sensors | grep 'Core 0' | awk '{print $3}' | tr -d '+°C')
    UPTIME=$(uptime -p)
    TIME=$(date "+%H:%M:%S")

    send_command "text 200 20 $CPU_LOAD&text 200 60 $RAM_USAGE&text 200 100 $CPU_TEMP&text 140 140 $UPTIME&text 200 180 $TIME"

    sleep 0.1
done
