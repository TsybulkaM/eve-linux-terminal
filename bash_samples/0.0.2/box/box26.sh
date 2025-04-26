#!/bin/bash

clear | cat -v > /tmp/eve_pipe

echo -ne "\e]50;IBM_Plex_Mono-26\a" | cat -v > /tmp/eve_pipe

cat 26text | cat -v > /tmp/eve_pipe