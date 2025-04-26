#!/bin/bash

clear | cat -v > /tmp/eve_pipe

echo -ne "\e]50;IBM_Plex_Mono-20\a" | cat -v > /tmp/eve_pipe

cat 20text | cat -v > /tmp/eve_pipe
