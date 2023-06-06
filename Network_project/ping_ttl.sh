#!/bin/bash

## Ping ##

server="lon.speedtest.clouvider.net"
TTL=4

for ((n=TTL; n<=255; n++)) 
do
    command="sudo ping -c 2 -t $n $server"
    echo "$command"
    cmdout=$(eval "$command")

    if [[ "$cmdout" != *"Time to live exceeded"* ]] && [[ "$cmdout" != *"Request timeout"* ]] && [[ "$cmdout" != *"100% packet loss"* ]]; then
        TTL=$n
        break
    fi
done
echo 
echo "E' necessario attraversare: $TTL Link"
echo

## Traceroute ##

command="sudo traceroute $server"
eval "$command"