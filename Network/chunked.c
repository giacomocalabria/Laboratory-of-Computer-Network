#include <sys/types.h>          /* See NOTES */ 
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
struct headers{
char * n;
char * v;
};

struct headers h[100];
char sl[1001];
char hbuf[5000];

//       int socket(int domain, int type, int protocol);

struct sockaddr_in remote_addr;
unsigned char * ip;
char * request = "GET / HTTP/1.1\r\nHost:www.google.com\r\n\r\n";

char request2[100];
unsigned char chunk[10];
unsigned char entity[ENTITY_SIZE+1];
unsigned char check[3];
int main()
{
int i,j,s,t,chunk_size=-1;
int length = UNDEFINED;
if (-1 ==(s = socket(AF_INET, SOCK_STREAM, 0))) {
	perror("Socket fallita");
	printf("%d\n",errno);
	return 1;
}
 remote_addr.sin_family = AF_INET;
 remote_addr.sin_port = htons(80);
 ip = (unsigned char*)&remote_addr.sin_addr.s_addr; 
 ip[0]=142; ip[1]=250;ip[2]=200;ip[3]=36;
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
	if(!strcmp(h[i].n,"Transfer-Encoding") && !strcmp(h[i].v," chunked"))
		length = CHUNKED;
}

printf("LENGTH: %d\n", length);
if (length == CHUNKED) {
	for (i=0, j=0, chunk_size=-1; chunk_size != 0; ) {
		for(i=0; i<10 && (read(s,chunk+i,1)) && chunk[i-1]!='\r' && chunk[i]!='\n';i++);
		chunk[i]=0;
		printf("Estraggo chunksize: %s\n",chunk);
		chunk_size=(int) strtol(chunk,NULL,16);
		printf("Chunk size = %d\n", chunk_size);
		printf("Consumo Chunk data\n");
		for(i=0; i<ENTITY_SIZE && (t=read(s,entity+j,chunk_size-i)); i+=t, j+=t);
		i = read(s,check,2);
		printf("Check del CRLF dopo chunk data\n");
		if (i!=2 || check[0]!='\r' || check[1]!='\n') {printf("Errore nella lettura del chunk-data\n"); return -1;}
	}
 
	entity[j]=0;
}	
else if(length !=UNDEFINED) {
	for(i=0;i<length && (t=read(s,entity+i,ENTITY_SIZE-i));i+=t);
	entity[i]=0;
}
else{ 
	for(i=0;(t=read(s,entity+i,ENTITY_SIZE-i));i+=t);
	entity[i]=0;
}
printf("%s\n",entity);
}


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

