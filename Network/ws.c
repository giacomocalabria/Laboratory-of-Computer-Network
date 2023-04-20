#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <unistd.h>

 #include <arpa/inet.h>
 #include <stdio.h>
char * reqline;
 struct header {
 char * n;
 char * v;
 } h[100];
 
char hbuffer[10000];

struct sockaddr_in local_addr, remote_addr;
char request[100000];
char response[100000];
int main(){
int s,s2,t,len,i,j,yes=1,length;
s = socket(AF_INET, SOCK_STREAM,0);
if ( s == -1 ) { perror("Socket Fallita\n"); return 1;}
 t= setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));
 if (t==-1){perror("setsockopt fallita"); return 1;}
local_addr.sin_family = AF_INET;
local_addr.sin_port = htons(8082);

if ( bind( s,(struct sockaddr *) &local_addr, sizeof(struct sockaddr_in)) == -1){
	perror("bind fallita");
	return -1;
}
if (listen(s,5)==-1) { perror("Listen Fallita"); return -1;}
while( 1 ){
	len = sizeof(struct sockaddr_in);
	s2=accept(s,(struct sockaddr *)&remote_addr, &len);
	if (s2 == -1) { perror ("Accept Fallita"); return -1; }
 bzero(hbuffer,10000);
 bzero(h,sizeof(struct header)*100);
 reqline = h[0].n = hbuffer;
 	for (i=0,j=0; read(s2,hbuffer+i,1); i++) {
	 if(hbuffer[i]=='\n' && hbuffer[(i)?i-1:i]=='\r'){
 	 		hbuffer[i-1]=0; // Termino il token attuale
 			if (!h[j].n[0]) break;
 			h[++j].n=hbuffer+i+1;
 		}	
 if (j!=0 && hbuffer[i]==':' && !h[j].v){
 	hbuffer[i]=0;
 	h[j].v = hbuffer + i + 1;
 }
 }
for(i=0;h[i].n[0];i++){
  printf("h[%d].n ---> %s , h[%d].v ---> %s\n",i,h[i].n,i,h[i].v);
  if(!strcmp(h[i].n,"Content-Length"))
    length=atoi(h[i].v);
}


	t= read(s2,request,1000);
	sprintf(response,"HTTP/1.1 404 Not Found\r\n\r\n");
	for(len=0;len<1000 && response[len] ; len++);
	write(s2,response,len);
	request[t]=0;
	printf("%s\n",request);
	close(s2);
	}
}


 
