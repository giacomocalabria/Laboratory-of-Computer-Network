#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>

struct header {
    char * n;
    char * v;
};

struct header h[100];

char hbuffer[10000];
char request[100000];
char response[100000];

int main(){
    int s,s2,t,len,i,j,yes=1,length;

    // Viene creato un socket TCP
    s = socket(AF_INET, SOCK_STREAM, 0);

    // Verifichiamo che il socket sia stato creato correttamente

    if(s == -1){
        perror("Socket Fallita\n"); 
        return 1;
    }

    t = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if(t == -1){
        perror("setsockopt fallita"); 
        return 1;
    }

    /* struct sockaddr_in {
               sa_family_t    sin_family;
               in_port_t      sin_port;
               struct in_addr sin_addr;
           };
    */
    struct sockaddr_in local_addr, remote_addr;

    local_addr.sin_family = AF_INET; // IPv4
    local_addr.sin_port = htons(8082); // Porta 8082 

    /* Associa il socket s alla porta 8082 e all'indirizzo IP
    
        Legare il socket ad un port e anche ad un indirizzp IP se la macchina ha più interfacce IP (più indirizzi), tramite la chiamata a sistema bind()
    
    */

    if(bind(s, (struct sockaddr *)&local_addr, sizeof(struct sockaddr_in)) == -1){
        perror("bind fallita");
        return -1;
    }

    /* 
        La chiamata a sistema listen() mette il socket in ascolto sulla porta specificata, in attesa di connessioni in entrata. Il primo parametro è il socket, il secondo è il numero massimo di connessioni pendenti che il sistema può accettare per quel socket.
    */

    if(listen(s, 5) == -1){
        perror("Listen Fallita"); 
        return -1;
    }

    char *reqline;

    while(1){
        len = sizeof(struct sockaddr_in);

        // Da questo momento il server è in ascolto sulla porta 8082

        /*
            La chiamata a sistema accept() estrae la prima connessione in coda di connessioni pendenti per il socket s, creando un nuovo socket per la comunicazione con il client. Il socket s rimane in ascolto per ulteriori connessioni in entrata.
        */
        s2 = accept(s, (struct sockaddr *)&remote_addr, &len);
        if(s2 == -1){
            perror("Accept Fallita"); 
            return -1;
        }

        // NOTA BENE: da questo momento in poi il socket s2 è connesso al client
        
        /*
            La funzione bzero() elimina i dati nella memoria puntata da hbuffer, per una lunghezza di 10000 byte. In questo modo, se il client invia una richiesta più corta di 10000 byte, i byte rimanenti saranno comunque a 0.
        */
        bzero(hbuffer, 10000);
        bzero(h, sizeof(struct header)*100);

        reqline = h[0].n = hbuffer;

        // Legge la richiesta del client e la memorizza in hbuffer
        for(i=0,j=0; read(s2,hbuffer+i,1); i++){  // Legge il contenuto della richiesta del client
            if(hbuffer[i]=='\n' && hbuffer[(i)?i-1:i]=='\r'){
                    hbuffer[i-1] = 0; // Termino il token attuale
                    if(!h[j].n[0]) break;
                    h[++j].n = hbuffer+i+1;
                }	
            if(j!=0 && hbuffer[i]==':' && !h[j].v){ // Se il carattere letto è un ':' e il puntatore "v" è NULL, allora vado a leggere il valore del token
                hbuffer[i] = 0;
                h[j].v = hbuffer + i + 1;
            }
        }

        // NOTA BENE: la parte sopra è uguale nel clinet e nel server

        // Visualizza l'header della richiesta del client e il campo "Content-Length" se presente

        for(i=0;h[i].n[0];i++){
            printf("h[%d].n ---> %s , h[%d].v ---> %s\n",i,h[i].n,i,h[i].v);
        }

        // NOTA BENE: da questo momento in poi il server risponde al client

        sprintf(response,"HTTP/1.1 404 Not Found\r\n\r\n"); // Inviamo solo la status line della risposta.
        for(len=0;len<1000 && response[len] ; len++);
        write(s2,response,len);

        close(s2);
    }
}