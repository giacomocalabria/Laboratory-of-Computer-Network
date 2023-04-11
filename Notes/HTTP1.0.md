# Connessione HTTP

```c
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
     intero di ritorno vale -1 allora la connessione al socket fallisce 
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
  
  /*t è una variabile intera che ci dice se il collegamento è andato a buon fine, 
se t = 0 allora non vi sono stati problemi, altrimenti se vale -1 vi è un errore. 
E' necessario verificare sia subito se ci sono stati errori ma anche dopo un 
lasso di tempo per vedere se la connessione con il socket ha "retto".*/ 
  
  if(t ==-1){  // Verifica se il collegamento è ancora valido
    perror("Connect Fallita\n"); // Manda un messaggio di errore
    return 1; // Termina il programma con codice di uscita 1 (errore)
  }
  else if(t==0){
    printf("TUTTO OK");
  }
  
  sleep(5);  // Aspetta per 100 secondi
  
  if(t ==-1){  // Verifica se il collegamento è ancora valido
    perror("Connect Fallita\n"); // Manda un messaggio di errore
    return 1; // Termina il programma con codice di uscita 1 (errore)
  }
  
  return 0; // Termina il programma senza errori
}
```

Definiamo le strutture necessarie

```c
#define ENTITY_SIZE 1000000
struct headers{
  char * n;
  char * v;
};

struct headers h[100];
char sl[1001];
char hbuf[5000];

char * request = "GET / HTTP/1.0\r\n\r\n";
char request2[100];
unsigned char entity[ENTITY_SIZE+1];
```

Proseguiamo con il codice

```c
  for(t=0;request[t];t++);
  write(s,request,t);
  for(i=0;i<1000 && read(s,sl+i,1) && (sl[i]!='\n' || sl[(i)?i-1:i]!='\r');i++){ }
  sl[i]=0;
  printf("Status Line ----> %s\n ", sl);

  h[0].n = & hbuf[0];
  for(j=0,i=0;i<5000 && read(s,hbuf+i,1);i++){
    if(hbuf[i]=='\n' && hbuf[(i)?i-1:i]=='\r'){
      hbuf[i-1]=0;
      if(h[j].n[0] == 0) break;
      h[++j].n=hbuf+i+1;
													// j=j+1 ; h[j].n =....   <---- h[++j];
													// h[j].n =....;  j=j+1;  <---- h[j++];
	}
    else if(hbuf[i]==':' && ! h[j].v ){
      hbuf[i]=0;
      h[j].v=hbuf+i+1;
    }
  }
  for(i=0;h[i].n[0];i++)
    printf("h[%d].n ---> %s , h[%d].v ---> %s\n",i,h[i].n,i,h[i].v);
  
  for(i=0;i<ENTITY_SIZE && (t=read(s,entity+i,ENTITY_SIZE-i));i+=t);
  entity[i]=0;
  printf("%s",entity);
}

/*
while(t=read(s,response,1000000)){
	for(int i=0; i<t;i++) printf("%c",response[i]);
}
*/
```

Ok ora bisogna commentare
