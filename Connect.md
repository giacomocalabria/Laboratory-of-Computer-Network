## `connect()`

La chiamata di sistema `connect()` inizializza una connessione ad un socket. La sua sintassi è la seguente

```c
#include <sys/types.h>
#include <sys/socket.h>

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

La chiamata di sistema `connect()` connette il socket a cui fa riferimento il descrittore di file *file descriptor* `sockfd` all' indirizzo specificato da `addr`.

L'argomento `addrlen` specifica la dimensione di `addr`. Il formato dell'indirizzo in `addr` è determinato dallo spazio degli indirizzi del socket `sockfd`

<div style="column-count: 2">
    <div>
        Se il socket <code>sockfd</code> è del tipo <code>SOCK_DGRAM</code>, allora <code>addr</code> è l'indirizzo a cui vengono inviati i datagrammi per impostazione predefinita e l'unico indirizzo da cui vengono ricevuti i datagrammi.
    </div>
    <div>
        Se il socket è di tipo <code>SOCK_STREAM</code> o <code>SOCK_SEQPACKET</code>, questa chiamata tenta di stabilire una connessione al socket associato all'indirizzo specificato da <code>addr</code>.
    </div>
</div>

Generalmente i socket di protocollo basati sulla connessione possono connettersi `connect()` solo una volta. I socket di protocollo senza connessione possono utilizzare `connect()` più volte per modificare la loro associazione. 

I socket senza connessione possono dissolvere l'associazione collegandosi a un indirizzo con il membro `sa_family` di `sockaddr` impostato su `AF_UNSPEC`

#### Valore di ritorno

Le la connessione o l'associazione ha esito positivo, viene restituito 0. Altrimenti, in caso di errore, viene restituito -1 e `errno` viene impostato in modo appropriato.
