#!/bin/bash

## Ping ##

server="lon.speedtest.clouvider.net" # server di test
TTL=4 # TTL iniziale

for ((n=TTL; n<=255; n++)) # ciclo per trovare il TTL
do
    command="sudo ping -c 2 -t $n $server" # comando ping con TTL specificato
    echo "$command"
    cmdout=$(eval "$command")

    # Verifica se il TTL è corretto (non si è verificato un timeout)
    if [[ "$cmdout" != *"Time to live exceeded"* ]] && [[ "$cmdout" != *"Request timeout"* ]] && [[ "$cmdout" != *"100% packet loss"* ]]; then
        TTL=$n
        break
    fi
done
echo 
echo "E' necessario attraversare: $TTL Link"
echo

## Traceroute ##

command="sudo traceroute $server" # comando traceroute da eseguire
eval "$command"