#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define ENTITY_SIZE 1000000
#define CHUNKED -2
#define UNDEFINED -1

struct headers{	// Struttura per gli header
	char * n;  	// Nome dell'header
	char * v;  	// Valore dell'header
};

// STRUTTURA DEI CHUNKS

/* 
	Chunked-Body   = *chunk
					last-chunk
					trailer
					CRLF

	chunk          = chunk-size [ chunk-extension ] CRLF
					chunk-data CRLF
	chunk-size     = 1*HEX
	last-chunk     = 1*("0") [ chunk-extension ] CRLF

	chunk-extension= *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
	chunk-ext-name = token
	chunk-ext-val  = token | quoted-string
	chunk-data     = chunk-size(OCTET)
	trailer        = *(entity-header CRLF)
*/

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

    /*  struct sockaddr_in {
            sa_family_t    sin_family;
            in_port_t      sin_port;
            struct in_addr sin_addr;
        };
    */
    struct sockaddr_in remote_addr;
    unsigned char * ip;

    remote_addr.sin_family = AF_INET; // Tipo di collegamento (Protocollo internet IPv4)	
    remote_addr.sin_port = htons(8082); // Porta di rete 80 (HTTP) in network byte order

    ip = (unsigned char*)&remote_addr.sin_addr.s_addr; 

    // Per questo esempio prendiamo l'indirizzo di www.google.com (vedi sotto come)
    //147.162.235.155
    //ip[0]=142; ip[1]=250;ip[2]=200;ip[3]=36;
    
    //ip[0]=88; ip[1]=80;ip[2]=187;ip[3]=84;

    ip[0]=127; ip[1]=0;ip[2]=0;ip[3]=0;
    
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

	//char * request = "GET / HTTP/1.1\r\nHost:www.google.com\r\n\r\n";
    char * request = "GET /prova.html HTTP/1.1\r\n\r\n";
    
    /* Richiesta HTTP/1.1 da inviare al server
        GET / HTTP/1.1\r\nHost:www.google.com\r\n\r\n
        GET è il metodo della richiesta
        / è il path della risorsa richiesta
        HTTP/1.1 è la versione del protocollo HTTP
        \r\n indica la fine della riga
        Host:www.google.com è l'header Host con valore www.google.com
        \r\n indica la fine della riga
        \r\n indica la fine della riga

        Essa è una Request del tipo
    */

    for(t=0; request[t]; t++); 
    /* Conta il numero di caratteri della richiesta da inviare al server (non considera il carattere di fine stringa '\0') Viene eseguito un ciclo "for" per contare il numero di caratteri nella stringa di richiesta da inviare al server. Il ciclo continua fino a quando non viene raggiunto il carattere di fine stringa '\0', e ad ogni iterazione viene incrementato l'indice "t". Alla fine del ciclo, la variabile "t" contiene il numero di caratteri della stringa di richiesta da inviare al server (non considera il carattere di fine stringa '\0').*/
    printf("Invio una richiesta di %d caratteri. Contenuto richiesta: %s\n",t,request);

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

    int length = UNDEFINED; // Lunghezza dell'entity body
    int chunk_size = UNDEFINED; // Dimensione del chunk

    unsigned char entity[ENTITY_SIZE+1];

    unsigned char chunk[10];
    unsigned char check[3];

    // Leggo la status line della risposta del server
    for(i=0; i<1000 && read(s,sl+i,1) && (sl[i]!='\n' || sl[(i)?i-1:i]!='\r'); i++);

    /* Viene eseguito un ciclo "for" per leggere la status line della risposta del server. Il ciclo legge un carattere alla volta dal socket "s" e lo memorizza nel buffer "sl" fino a quando una delle seguenti condizioni viene soddisfatta: l'indice "i" raggiunge il valore 1000, la lettura dal socket fallisce o il carattere letto è un '\n' ma il carattere precedente (sl[i-1]) non è un '\r'. In pratica, il ciclo legge la status line fino a quando non viene raggiunto il carattere di fine riga '\n' seguito da un possibile '\r'. */

    sl[i]=0;  // Termina la stringa con il carattere di fine stringa '\0'. Il carattere di fine riga '\n' (o l'ultimo carattere letto se non c'è un '\n') viene sostituito con il carattere di fine stringa '\0', terminando così la stringa contenuta nel buffer "sl".
	
    printf("Status Line ----> %s\n", sl);  // Stampa la status line della risposta del server

    // Leggo gli header della risposta del server

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

    /* La parte precedente è essenzialmente la stessa, durante l'acquisizione degli header vviene letto il nome e il valore.
    
    Vogliamo ora ricavare dagli header della risposta del server la "Content-Length" dell'entity body della risposta del server.

    La "Content-Length" è un header che indica la lunghezza dell'entity body della risposta del server. La "Content-Length" è un header opzionale, quindi non è detto che sia presente nella risposta del server. Se la "Content-Length" non è presente, allora la lunghezza dell'entity body della risposta del server è sconosciuta. In questo caso, la lunghezza dell'entity body della risposta del server viene impostata a 5000.

    Vogliamo ora sapere se la risposta del server è in formato "chunked". Nel campo dell' header "Transfer-Encoding" se viene usata la cofigica "chuncked" dobbiamo utilizzare tale per leggere la risposta del server.

    Allo stesso tempo stampiamo gli header della risposta del server.
        
    */

    for(i=0;h[i].n[0];i++){
        printf("h[%d].n ---> %s , h[%d].v ---> %s\n",i,h[i].n,i,h[i].v); // Stampa gli header ricevuti dal server

        // Ricerca dell'header "Content-Length" nella risposta del server
	    if(!strcmp(h[i].n,"Content-Length")) // Se l'header corrente è "Content-Length", allora viene memorizzata la lunghezza dell'entity body della risposta del server nella variabile "length".
		    length = atoi(h[i].v);  // atoi() converte una stringa in un intero

        if(!strcmp(h[i].n,"Transfer-Encoding") && !strcmp(h[i].v," chunked"))
		    length = CHUNKED; // Se l'header corrente è "Transfer-Encoding" e il suo valore è "chunked", allora la risposta del server è in formato "chunked"
    }

    printf("LENGTH: %d\n", length);

    // Se length è stata contrassegnata come CHUNKED, allora la risposta del server è in formato "chunked" e viene eseguito il seguente codice
    if(length == CHUNKED){
        for (i=0, j=0, chunk_size=-1; chunk_size != 0; ) {
            for(i=0; i<10 && (read(s,chunk+i,1)) && chunk[i-1]!='\r' && chunk[i]!='\n';i++);
            chunk[i]=0;

            printf("Estraggo chunksize: %s\n",chunk);
            chunk_size=(int) strtol(chunk,NULL,16);
            printf("Chunk size = %d\n", chunk_size);
            printf("Consumo Chunk data\n");
            for(i=0; i<ENTITY_SIZE && (t=read(s,entity+j,chunk_size-i)); i+=t, j+=t);
            i = read(s,check,2); // Legge i due caratteri di fine chunk-data "\r\n"
            //printf("Il valore di check è %d %d\n", check[0], check[1]);
            printf("Check del CRLF dopo chunk data\n");
            if(i!=2 || check[0]!='\r' || check[1]!='\n'){ // Se il check del CRLF fallisce, allora c'è un errore nella lettura del chunk-data
                printf("Errore nella lettura del chunk-data\n"); 
                return -1;
            }
        }
 
	    entity[j]=0;
    }
    else if(length != UNDEFINED) {
	    for(i=0;i<length && (t=read(s,entity+i,ENTITY_SIZE-i));i+=t);
        entity[i]=0;
    }
    else{ 
        for(i=0;(t=read(s,entity+i,ENTITY_SIZE-i));i+=t);
        entity[i]=0;
    }

    printf("%s\n",entity);

    close(s);

    return 0;
}       