#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

#define ENTITY_SIZE 10000 
 
struct headers{	// Struttura per gli header
	char * n;  	// Nome dell'header
	char * v;  	// Valore dell'header
};

int main(){
	int i,j,s,t;

	// Viene effettuata la chiamata a sistema per creare il socket
    s = socket(AF_INET, SOCK_STREAM, 0);

    /* Questo if verifica se il socket è stato creato correttamente. Infatti se il numero
        intero di ritorno vale -1 allora la creazione del socket non è andata a buon fine 
    */
    if (-1 == s){
        perror("Socket fallita"); // Manda un messaggio di errore
        printf("%d\n",errno);
        return 1; // Termina il programma in errore
    }
 
 	/* struct sockaddr_in {
               sa_family_t    sin_family;
               in_port_t      sin_port;
               struct in_addr sin_addr;
           };
    */
    struct sockaddr_in remote_addr;
    unsigned char * ip;

    remote_addr.sin_family = AF_INET; // Tipo di collegamento (Protocollo internet IPv4)	
    remote_addr.sin_port = htons(80); // Porta di rete 80 (HTTP) in network byte order

    ip = (unsigned char*)&remote_addr.sin_addr.s_addr; 

    // Per questo esempio prendiamo l'indirizzo di www.google.com (vedi sotto come)
    ip[0]=142; ip[1]=250;ip[2]=200;ip[3]=36;
    
    // Istanzia il collegamento con connect()

    t = connect(s,(struct sockaddr *) &remote_addr,sizeof(struct sockaddr_in));
    /*t è una variabile intera che ci dice se il collegamento è andato a buon fine, se t = 0 allora non vi sono stati problemi, altrimenti se vale -1 vi è un errore. E' necessario verificare sia subito se ci sono stati errori ma anche dopo un lasso di tempo per vedere se la connessione con il socket ha "retto".*/ 
    
    if(t ==-1){  // Verifica se il collegamento è ancora valido
        perror("Connect Fallita\n"); // Manda un messaggio di errore
        return 1; // Termina il programma con codice di uscita 1 (errore)
    }
    /*if(t==0){
        printf("TUTTO OK\n");
    }*/


	// Invia la richiesta al server

	char *request = "GET / HTTP/1.0\r\n\r\n"; // Stringa di richiesta da inviare al server (GET / HTTP/1.0\r\n\r\n) che rappresenta una richiesta di tipo GET all'endpoint "/" utilizzando la versione di protocollo HTTP 1.0. I caratteri "\r\n\r\n" rappresentano i caratteri di fine riga per segnalare la fine dell'header della richiesta.
	char request2[100];
	for(t=0; request[t]; t++); 
    /* Conta il numero di caratteri della richiesta da inviare al server (non considera il carattere di fine stringa '\0') Viene eseguito un ciclo "for" per contare il numero di caratteri nella stringa di richiesta da inviare al server. Il ciclo continua fino a quando non viene raggiunto il carattere di fine stringa '\0', e ad ogni iterazione viene incrementato l'indice "t". Alla fine del ciclo, la variabile "t" contiene il numero di caratteri della stringa di richiesta da inviare al server (non considera il carattere di fine stringa '\0').*/
    printf("Invio una richiesta di %d caratteri\n",t);
    // Invia la richiesta al server
 	write(s, request, t);
    /*  write() è una chiamata a sistema che invia dati su un socket 
        s è il socket, request è il buffer da cui leggere i dati da inviare, t è il numero di byte da inviare
        write() writes up to count bytes from the buffer starting at buf
        to the file referred to by the file descriptor fd. Viene utilizzata la funzione write() per inviare la richiesta al server tramite il socket "s". Viene passato il puntatore alla stringa di richiesta "request" e il numero di caratteri "t" calcolato nel passaggio precedente.
    */

    // Legge la risposta del server: in sintesi, il codice legge la status line e gli header della risposta del server e li memorizza in strutture dati appropriate per ulteriori elaborazioni o analisi.

    struct headers h[100];   // Struttura per gli header della risposta del server
    char sl[1001];      // Buffer per la status line della risposta del server
    char hbuf[5000];    // Buffer per la risposta del server (header + entity body) 

    unsigned char entity[ENTITY_SIZE+1]; // Buffer per l'entity body della risposta del server

    for(i=0; i<1000 && read(s,sl+i,1) && (sl[i]!='\n' || sl[(i)?i-1:i]!='\r'); i++){ }  // Legge la status line della risposta del server

    /* Viene eseguito un ciclo "for" per leggere la status line della risposta del server. Il ciclo legge un carattere alla volta dal socket "s" e lo memorizza nel buffer "sl" fino a quando una delle seguenti condizioni viene soddisfatta: l'indice "i" raggiunge il valore 1000, la lettura dal socket fallisce o il carattere letto è un '\n' ma il carattere precedente (sl[i-1]) non è un '\r'. In pratica, il ciclo legge la status line fino a quando non viene raggiunto il carattere di fine riga '\n' seguito da un possibile '\r'. */

	sl[i]=0;  // Termina la stringa con il carattere di fine stringa '\0'. Il carattere di fine riga '\n' (o l'ultimo carattere letto se non c'è un '\n') viene sostituito con il carattere di fine stringa '\0', terminando così la stringa contenuta nel buffer "sl".
	
    printf("Status Line ----> %s\n ", sl);  // Stampa la status line della risposta del server

	h[0].n = & hbuf[0];  // h[0].n punta al primo carattere di hbuf. Viene inizializzato il primo header (h[0]) in modo che il suo puntatore "n" punti al primo carattere del buffer "hbuf".

    /*
        Viene eseguito un secondo ciclo "for" per leggere gli header della risposta del server. Il ciclo legge un carattere alla volta dal socket "s" e lo memorizza nel buffer "hbuf" fino a quando una delle seguenti condizioni viene soddisfatta: l'indice "i" raggiunge il valore 5000 o la lettura dal socket fallisce.
    */

	for(j=0, i=0; i < 5000 && read(s, hbuf+i, 1); i++){  // Legge gli header della risposta del server
		if(hbuf[i]=='\n' && hbuf[(i)?i-1:i]=='\r'){ // Se il carattere letto è un '\n' e il carattere precedente (hbuf[i-1]) è un '\r', allora è stato letto un carattere di fine riga '\n' seguito da un '\r' che indica la fine degli header della risposta del server.
			hbuf[i-1] = 0;
			if(h[j].n[0] == 0) break; // Se l'header è vuoto, termina il ciclo
			h[++j].n = hbuf+i+1; 
		}
		else if(hbuf[i]==':' && !h[j].v){ // Se il carattere letto è un ':' e il puntatore "v" dell'header corrente (h[j]) è NULL, allora è stato letto il carattere ':' che separa il nome del valore dell'header.
			hbuf[i] = 0;
			h[j].v = hbuf+i+1;
		}		
	}

	for(i=0;h[i].n[0];i++)  // Stampa gli header ricevuti dal server
		printf("h[%d].n ---> %s , h[%d].v ---> %s\n",i,h[i].n,i,h[i].v); // Stampa gli header ricevuti dal server

    // Legge l'entity body della risposta del server

    printf("Leggo l'entity body della risposta del server\n");
    printf("ENTITY_SIZE = %d\n",ENTITY_SIZE);

	for(i=0;i < ENTITY_SIZE && (t = read(s, entity+i, ENTITY_SIZE-i)); i+=t); // Legge l'entity body della risposta del server

	entity[i]=0;
	printf("%s",entity);
}