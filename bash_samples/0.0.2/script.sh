#!/bin/bash
#set -x
# restart service
systemctl restart eveld.service
fontsize=$1
# font size

if [ "$fontsize" == "16" ]; then
echo -e "\e]50;IBM_Plex_Mono-16\a" | cat -v > /tmp/eve_pipe

elif [ "$fontsize" == "20" ]; then

echo -e "\e]50;IBM_Plex_Mono-20\a" | cat -v > /tmp/eve_pipe

elif [ "$fontsize" == "26" ]; then
echo -e "\e]50;IBM_Plex_Mono-26\a" | cat -v > /tmp/eve_pipe
fi

echo "check display"

now=$(date +"%d-%m-%Y")

for i in {0..1..1}
do
#	cat test > /tmp/eve_pipe
#        cat 26text | cat -v > /tmp/eve_pipe
#        cat 20text | cat -v > /tmp/eve_pipe
#        cat 16text | cat -v > /tmp/eve_pipe

#------------------------------------------------
# 16 font size   14 строк вмещается в экран
# printf "%b" "-----------------------------------------------
# | date: $now                            |
# | frame number: $i                             |
# |                                             |
# |          РУССКИЙ АЛФАВИТ                    |
# |          1234560!!!(){}@                    |
# |          Support!!!ABCD#                    |
# |                                             |
# |                                             |
# |                                             |
# |                                             |
# |                                             |
# |                                             |
# -----------------------------------------------
# " | cat -v > /tmp/eve_pipe
#------------------------------------------------
if [ "$fontsize" == "16" ]; then

printf "%b" "
  date: $now 
  frame number: $i
 У лукоморья дуб зелёный;
 Златая цепь на дубе том:
 И днём и ночью кот учёный
 Всё ходит по цепи кругом;
 Идёт направо - песнь заводит,
 Налево - сказку говорит.
 Там чудеса: там леший бродит,
 Русалка на ветвях сидит;
 Там на неведомых дорожках
 Следы невиданных зверей;
======end=====
" | cat -v > /tmp/eve_pipe

sleep 1s

elif [ "$fontsize" == "20" ]; then

printf "%b" "
  date: $now 
  frame number: $i
 
           РУССКИЙ АЛФАВИТ
           1234560!!!(){}@
           Support!!!ABCD#






" | cat -v > /tmp/eve_pipe

sleep 1s

elif [ "$fontsize" == "26" ]; then

printf "%b" "
  date: $now 
  frame number: $i
 
           РУССКИЙ АЛФАВИТ
           1234560!!!(){}@
           Support!!!ABCD#



" | cat -v > /tmp/eve_pipe

	sleep 1s

	echo $i
fi

done;
