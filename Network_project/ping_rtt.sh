#!/bin/bash

## Ping ##

server="www.google.it"
nomefile="ris_rtt.txt"
K=100

if [ -f $nomefile ]; then
    rm $nomefile
fi

for ((L=16; L<=1472; L++))
do
    command="sudo ping -c $K -s $L -A $server"

    # Esecuzione del comando ping
    ping_output=$(eval "$command")

    # Estrazione dell'ultima linea
    last_line=$(echo "$ping_output" | tail -n 1)

    # Estrazione dei valori di RTT
    rtt_line=$(echo "$last_line" | grep -oE 'rtt min/avg/max/mdev = [0-9.]+/[0-9.]+/[0-9.]+/[0-9.]+ ')

    # Rimozione del prefisso "rtt min/avg/max/mdev = "
    rtt_values=${rtt_line#"rtt min/avg/max/mdev = "}

    # Separazione dei valori di RTT
    IFS='/' read -ra rtt_array <<< "$rtt_values"
    rtt_min=${rtt_array[0]}
    rtt_avg=${rtt_array[1]}
    rtt_max=${rtt_array[2]}
    rtt_mdev=${rtt_array[3]}

    # Stampa dei valori
    echo "$L;$rtt_min;$rtt_avg;$rtt_max;$rtt_mdev" >> $nomefile
    
    # Percentuale completamento
    echo "$(((L * 100)/1472)) %  siamo a L = $L, scritto su file $nomefile i valori $rtt_min, $rtt_avg, $rtt_max, $rtt_mdev "
done