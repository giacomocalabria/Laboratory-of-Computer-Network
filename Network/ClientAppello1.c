#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h> // gesthostbyname()

#define ENTITY_SIZE 1000000
struct headers{
char * n;
char * v;
};

struct hostent *he;

struct headers h[100];
char sl[1001];
char hbuf[5000];

//       int socket(int domain, int type, int protocol);

struct sockaddr_in remote_addr, local_addr;
unsigned char * ip;
char request[200] = "GET / HTTP/1.1\r\nHost:www.google.com\r\n\r\n";

char request2[100];
unsigned char entity[ENTITY_SIZE+1];
int main(int argc, char* argv[]) // argv[] holds argc C-style strings
{
int i,j,s,t;
int length = -1;

srand(time(NULL));

if (-1 ==(s = socket(AF_INET, SOCK_STREAM, 0))) {
        perror("Socket fallita");
        printf("%d\n",errno);
        return 1;
}

remote_addr.sin_family = AF_INET;
remote_addr.sin_port = htons(80);

local_addr.sin_family = AF_INET;
local_addr.sin_port = htons(20030); // port contenuto nel file port.txt 20025

if(bind(s, (struct sockaddr *)&local_addr, sizeof(struct sockaddr_in)) == -1){
    perror("bind fallita");
    return -1;
}

if(argc == 0){ // NON CI SONO ARGOMENTI NELLA LINEA DI COMANDO
        ip = (unsigned char*)&remote_addr.sin_addr.s_addr;
        ip[0]=142; ip[1]=250;ip[2]=200;ip[3]=36;
}
else{ // CI SONO ARGOMENTI NELLA LINEA DI COMANDO
        // EFFETTUO IL PARSING
        char *line = argv[1];

        char *sito = line + 7; // Tolgo la parte iniziale http://
        for(i = 0; *sito && *sito!='/';i++, sito++);
        line[i++]=0;
        char *pagina = line + 7 + i;

        printf("sito =%s\n",sito);
        printf("pagina =%s\n",pagina);

        sito = "tinypic.com";

        // ESTRAGGO L' IP DAL SITO
        he = gethostbyname(sito);
        int n = rand()%he->h_length;
        remote_addr.sin_addr.s_addr = *(unsigned int*) (he->h_addr_list[n]);

        // RISCRIVO LA REQUEST LINE
        sprintf(request,"GET /%s HTTP/1.1\r\nHost:%s\r\n\r\n",pagina,sito);
}


// Da questo punto non cambia nulla nel funzionamento del client

t = connect(s,(struct sockaddr *) &remote_addr,sizeof(struct sockaddr_in));
if(t ==-1) {
        perror("Connect Fallita\n");
  return 1;
}
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
        }
        else if(hbuf[i]==':' && ! h[j].v ){
                hbuf[i]=0;
                h[j].v=hbuf+i+1;
        }
}
for(i=0;h[i].n[0];i++){
        printf("h[%d].n ---> %s , h[%d].v ---> %s\n",i,h[i].n,i,h[i].v);
        if(!strcmp(h[i].n,"Content-Length"))
                length=atoi(h[i].v);
}

if(length == -1) length = 5000;

if(length == -1)
        for(i=0;i<ENTITY_SIZE && (t=read(s,entity+i,ENTITY_SIZE-i));i+=t);
else
        for(i=0;i<length && (t=read(s,entity+i,ENTITY_SIZE-i));i+=t);
        entity[i]=0;
        printf("%s",entity);
}