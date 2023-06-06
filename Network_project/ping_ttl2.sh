#!/bin/bash

## Ping ##

server="la.speedtest.clouvider.net "
TTL=4

for ((n=20; n >= 2; n--))
do
    command="sudo ping -c 2 -t $n $server"
    echo "$command"
    cmdout=$(eval "$command")

    if [[ "$cmdout" == *"Time to live exceeded"* ]] || [[ "$cmdout" == *"Request timeout"* ]] ; then
        TTL=$n
        TTL=$((TTL + 1))
        break
    fi
done

echo 
echo "E' necessario attraversare: $TTL Link"
echo

## Traceroute ##

command="sudo traceroute $server"
eval "$command"