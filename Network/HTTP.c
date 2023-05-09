#include <sys/types.h>          /* See NOTES */ 
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

#define ENTITY_SIZE 1000000 
 
struct headers{	// Struttura per gli header
	char * n;  	// Nome dell'header
	char * v;  	// Valore dell'header
};

struct headers h[100];   // L'header non può esser lungo quanto si vuole 
char sl[1001]; 
char hbuf[5000];  // Buffer per la risposta del server

unsigned char entity[ENTITY_SIZE+1];

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

	char *request = "GET / HTTP/1.0\r\n\r\n"; // Richiesta da inviare al server (GET / HTTP/1.0\r\n\r\n)
	char request2[100];

	for(t=0; request[t]; t++); // Conta il numero di caratteri della richiesta da inviare al server (non considera il carattere di fine stringa '\0')
    printf("Invio una richiesta di %d caratteri\n",t);
    // Invia la richiesta al server
 	write(s, request, t);
    /*  write() è una chiamata a sistema che invia dati su un socket 
        s è il socket, request è il buffer da cui leggere i dati da inviare, t è il numero di byte da inviare
        write() writes up to count bytes from the buffer starting at buf
        to the file referred to by the file descriptor fd.
    */

    // Legge la risposta del server
	for(i=0; i<1000 && read(s,sl+i,1) && (sl[i]!='\n' || sl[(i)?i-1:i]!='\r'); i++){ }  // Legge la status line della risposta del server

	sl[i]=0;  // Termina la stringa con il carattere di fine stringa '\0'
	printf("Status Line ----> %s\n ", sl);  // Stampa la status line della risposta del server

	h[0].n = & hbuf[0];  // h[0].n punta al primo carattere di hbuf

	for(j=0,i=0;i<5000 && read(s,hbuf+i,1);i++){
		if(hbuf[i]=='\n' && hbuf[(i)?i-1:i]=='\r'){
			hbuf[i-1]=0;
			if(h[j].n[0] == 0) break;
			h[++j].n=hbuf+i+1;
		}
		else if(hbuf[i]==':' && ! h[j].v ){  
			hbuf[i]=0;
			h[j].v=hbuf+i+1;
		}		
	}

	for(i=0;h[i].n[0];i++)  // Stampa gli header ricevuti dal server
		printf("h[%d].n ---> %s , h[%d].v ---> %s\n",i,h[i].n,i,h[i].v);

	for(i=0;i<ENTITY_SIZE && (t=read(s,entity+i,ENTITY_SIZE-i));i+=t); // Legge l'entity body della risposta del server

	entity[i]=0;
	printf("%s",entity);
}