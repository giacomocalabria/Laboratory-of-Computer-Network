#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

struct sockaddr_in remote_addr;
unsigned char * ip;

int main(){
  int s,t;

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

  remote_addr.sin_family = AF_INET; // Tipo di collegamento (Protocollo internet IPv4)	
  remote_addr.sin_port = htons(80); // Porta di rete 80 

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
    printf("TUTTO OK");
  }
  
  sleep(10);  // Aspetta per 100 secondi
  
  if(t ==-1){  // Verifica se il collegamento è ancora valido
    perror("Connect Fallita\n"); // Manda un messaggio di errore
    return 1; // Termina il programma con codice di uscita 1 (errore)
  }
  
  return 0; // Termina il programma senza errori
}