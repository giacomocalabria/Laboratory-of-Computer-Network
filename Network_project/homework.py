import os
import re
import matplotlib.pyplot as plt
from pathlib import Path

servers = {"Atlanta" : "atl.speedtest.clouvider.net", "New York" : "nyc.speedtest.clouvider.net", "London" : "lon.speedtest.clouvider.net",
           "Los Angeles" : "la.speedtest.clouvider.net", "Paris" : "paris.testdebit.info", "Lille" : "lille.testdebit.info ",
           "Lyon" : "lyon.testdebit.info", "Aix-Marseille" : "aix-marseille.testdebit.info", "Bordeaux" : "bordeaux.testdebit.info "}

server = input("Inserire il server al quale si vuole efettuare una sessione di ping: ").strip()
while server not in servers:
    print("Server " + server + " non disponibile\n")
    print("I server disponibili sono:")
    for key in servers.keys():
        print(key)
    print()
    server = input("Inserire il server al quale si vuole efettuare una sessione di ping: ").strip()

indirizzo = servers[server]

if Path("risultati_ping.txt").is_file():
    os.system("rm risultati_ping.txt")

Ls = range(20, 20, 10)
c = 100
mins = []
avgs = []
maxs = []
mdevs = []
for L in Ls:    #Quando passo i byte con -s a questi devo anche aggiungere gli 8 byte di header
    print(f"\nSto eseguendo il ping con -c {c} -s {L} e -t infinito al server {indirizzo} per calcolare i vari RTT\n")
    command = f"sudo ping -c {c} -s {L} -A {indirizzo}"
    #command = f"sudo ping -c {c} -s {L} -A google.com"
    with open ("risultati_ping.txt", "a") as file:
        file.write(command + "\n")

    os.system(command + " | tee risultati_temp.txt >> risultati_ping.txt ")

    with open ("risultati_ping.txt", "a") as file:
        file.write("\n\n")
    
    with open ("risultati_temp.txt") as file:
        for line in file:
            match = re.search(r'(\d+[.]\d+)/(\d+[.]\d+)/(\d+[.]\d+)/(\d+[.]\d+)', line)
            if match:
                mins.append(float(match.group(1)))
                avgs.append(float(match.group(2)))
                maxs.append(float(match.group(3)))
                mdevs.append(float(match.group(4)))

Ls = [(L + 28) * 8 for L in Ls]
plt.plot(Ls, mins, "ro")
plt.title("Andamento del valore minimo del RTT")
plt.xlabel("L (pkt size) - bits")
plt.ylabel("$RTT_{min(L)}$")
plt.grid()
plt.show()

plt.plot(Ls, avgs, "bo")
plt.title("Andamento del valore medio del RTT")
plt.xlabel("L (pkt size) - bits")
plt.ylabel("$RTT_{avg(L)}$")
plt.grid()
plt.show()

#plt.plot(Ls, maxs, "po")
plt.plot(Ls, maxs)
plt.title("Andamento del valore massimo del RTT")
plt.xlabel("L (pkt size) - bits")
plt.ylabel("$RTT_{max(L)}$")
plt.grid()
plt.show()

plt.plot(Ls, mdevs, "go")
plt.title("Andamento del valore della deviazione standard del RTT")
plt.xlabel("L (pkt size) - bits")
plt.ylabel("$RTT_{std(L)}$")
plt.grid()
plt.show()