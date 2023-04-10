#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>

int main(){
  int s,t;

  // Viene effettuata la chiamata a sistema per creare il socket

  s = socket(AF_INET, SOCK_STREAM,0);

  printf("%d\n",s);

  s = socket(AF_INET, SOCK_STREAM,0);

  printf("%d\n",s);

  s = socket(AF_INET, SOCK_STREAM,0);

  printf("%d\n",s);

  /* Questo if verifica se il socket è stato creato correttamente. Infatti se il numero
     intero di ritorno vale -1 allora la creazione del socket non è andata a buon fine 
  */
  
  if (-1 == s){
    perror("Socket fallita"); // Manda un messaggio di errore
    printf("%d\n",errno);
    return 1; // Termina il programma in errore
  }
  
  return 0; // Termina il programma senza errori
}