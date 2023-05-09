#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(){
    int s,t; // s è il socket, t è una variabile di appoggio

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
    if(t==0){
        printf("TUTTO OK\n");
    }

    // Invia la richiesta al server

    char * request = "GET / \r\n"; // Richiesta da inviare al server
    char request2[100]; // che cosa serve questo? 
    
    unsigned char response[1000001]; // Buffer per la risposta del server
  
    for(t=0; request[t]; t++); // Conta il numero di caratteri della richiesta da inviare al server (non considera il carattere di fine stringa '\0')
    printf("Invio %d caratteri\n",t);
    // Invia la richiesta al server
    write(s, request, t);
    /*  write() è una chiamata a sistema che invia dati su un socket 
        s è il socket, request è il buffer da cui leggere i dati da inviare, t è il numero di byte da inviare
        write() writes up to count bytes from the buffer starting at buf
        to the file referred to by the file descriptor fd.
    */

	sleep(2);

    // Legge la risposta del server
    while(t = read(s, response, 1000000)){ // read legge i dati dal socket e li mette nel buffer response
        // t è il numero di byte letti dal socket
        if(t == -1){ // Verifica se la lettura è andata a buon fine
            perror("Read fallita\n"); // Manda un messaggio di errore
            return 1; // Termina il programma con codice di uscita 1 (errore)
        }
	    for(int i=0; i<t;i++)
            printf("%c",response[i]); // Stampa a video la risposta del server
    }

    return 0; // Termina il programma senza errori
}