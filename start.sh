#!/bin/bash

#script for creating server and clients

(build/server 3333 & sleep 0.2)

name=""

for((j = 1; j <= 20; j++)) do
for((i = 1; i <= 4; i++))
do
    name=$(printf "Client%d" $i )
    build/client $name 3333 $i &
done
done

#for close own tcp socket
#netstat -tulpn | grep -E LISTEN.*/build/server | grep -Eo [0-9]* | tail -1 | xargs kill