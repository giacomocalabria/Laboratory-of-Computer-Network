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
char command[100];
char request[100000];
char response[100000];

int authorized(const char *credentials){
    FILE *file = fopen("token.txt", "r");
    char *token = NULL;
    size_t len = 0;
    ssize_t nread = 0;

    while((nread = getline(&token, &len, file)) != -1){
        token[strcspn(token, "\n")] = '\0'; // Rimuovi il carattere '\n' dalla stringa 
        if (!strcmp(token, credentials)) {
            fclose(file); // Chiudi il file
            return 1; // Credenziale trovata, restituisci 1
        }
    }
    fclose(file); // Chiudi il file
    return 0; // Credenziale non trovata, restituisci 0
}

int main(){
    FILE * fin;
    int s,s2,t,len,i,j,yes=1,length,err;
    char ch;

    int authorization = 0;
    char *credentials;

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

    char *reqline, *method, *filename, *ver;
    
    // Il programma si mette in attesa di connessioni in entrata all'infinito (while(1))

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

        // Visualizza l'header della richiesta del client

        for(i=0;h[i].n[0];i++){
            printf("h[%d].n ---> %s , h[%d].v ---> %s\n",i,h[i].n,i,h[i].v);
            if(!strcmp(h[i].n, "Authorization")){
                authorization = 1;
                credentials = h[i].v + 1;
                for(;*credentials != ' '; credentials++);
                credentials++;
            }
        }

        /* 
            La struttura della richiesta del client è la seguente:
        
            Request line:  <method> <SP> <URL><SP> <HTTP-ver><CRLF> 
        
        */

        err = 0;
        
        method = reqline;  
        for(;*reqline && *reqline!=' ';reqline++);
        
        if(*reqline==' '){ 
            *reqline = 0; 
            reqline++;
            filename=reqline;
            for(;*reqline && *reqline!=' ';reqline++);
            if(*reqline==' '){ 
                *reqline = 0; 
                reqline++;
                ver=reqline;
                for(;*reqline ;reqline++);
                if(*reqline){
                    printf("Error in version\n"); 
                    err=1;
                }
            }
            else{
                printf("Error in filename\n"); 
                err=1;
            }
        }
        else{
            printf("Error in method\n"); 
            err=1;
        }

        if(err){ // Se c'è stato un errore nella richiesta del client, allora il server invia la status line indicando che c'è un errore 400 Bad Request
	        sprintf(response,"HTTP/1.1 400 Bad Request\r\n\r\n");
        }
        else{
            printf("Method = %s, filename = %s, version = %s",method, filename, ver);
            
            // Se il metodo è GET, allora il server invia la risorsa richiesto dal client
            if(!strcmp(method,"GET")){
                /*if(!strncmp(filename,"/cgi/",5)){
                    sprintf(command,"%s > tmp",filename+5);
                    fork();
                    execve("/bin/sh",command,NULL);
                    dup2(s2,1);
                    sprintf(filename,"/tmp");
                }*/

                /*
                    fopen() apre il file specificato dal parametro filename e restituisce un puntatore ad un oggetto di tipo FILE che viene utilizzato dalle altre funzioni per identificare il file. Il parametro mode specifica la modalità di apertura del file. La modalità "rt" apre il file in modalità lettura testuale. 
                */

                if((fin = fopen(filename+1,"rt"))==NULL) // Tenta di aprire il file e verifica se esso esiste
                    sprintf(response,"HTTP/1.1 404 Not Found\r\n\r\n"); // Se il file non esiste, allora il server invia la status line indicando che non è presente il file (codice 404 Not Found)
                else{
                    if(authorization){
                        if(authorized(credentials)){
                            sprintf(response,"HTTP/1.1 200 OK\r\n\r\n"); // Se il file esiste, allora il server invia la status line indicando che è presente il file (codice 200 OK)
                            write(s2,response,strlen(response)); // Scrive sul socket s2 la status line

                            while( EOF != (ch=fgetc(fin))){ // EOF è una costante definita in stdio.h che indica la fine del file
                                write(s2,&ch,1); // Scrive sul socket s2 il carattere ch letto dal file con fgetc()
                            }

                            /*
                                fget() legge il carattere successivo dal file puntato da fin e lo restituisce come valore di ritorno. Se il carattere letto è EOF, allora la funzione restituisce EOF.
                            */

                            fclose(fin); // Chiude il file
                            close(s2); // Chiude il socket s2
                            continue; // Se il metodo è GET, allora il server invia il contenuto del file richiesto dal client e poi si rimette in attesa di connessioni in entrata. Riprende il while iniziale del main()
                        } else {
                            sprintf(response,"HTTP/1.1 401 Unauthorized\r\nWWW-Authenticate: Basic realm=\"Secure Area\"\r\n\r\n");
                        }
                    } else {
                        sprintf(response,"HTTP/1.1 401 Unauthorized\r\nWWW-Authenticate: Basic realm=\"Secure Area\"\r\n\r\n");
                    }
                }
            }
            else // Altrimenti, il server invia un messaggio di errore al client (metodo non implementato)
                sprintf(response,"HTTP/1.1  501 Not Implemented\r\n\r\n");
        }
        
        // Invia la risposta al client scrivendo sul socket s2 nel caso in cui non ci siano stati errori nella richiesta del client o comunque una richiesta non del tipo "200 OK" dove è necessario inviare anche il contenuto del file.

        for(len=0; len<1000 && response[len]; len++); //Calcola la lunghezza della risposta
        
        write(s2,response,len); // Invia la risposta al client 
        
        /*
            write() scrive sul socket s2 la risposta al client. Il primo parametro è il socket, il secondo è il buffer contenente la risposta, il terzo è la lunghezza della risposta in byte da inviare.        
        */

        // Chiude il socket s2 e si rimette in attesa di connessioni in entrata o va a vedere se nella coda delle connessioni pendenti c'è una nuova connessione in entrata. 
        close(s2);
    }

    return 0;
}