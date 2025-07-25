#!/bin/bash

set -x

clear | cat -v > /tmp/eve_pipe

echo -ne "\e]50;IBM_Plex_Mono-16\a" | cat -v > /tmp/eve_pipe

cat 16text | cat -v > /tmp/eve_pipe
