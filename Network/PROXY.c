#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <unistd.h>
#include <netdb.h> // Lib per gethostbyname()
#include <netinet/in.h>
#include <sys/types.h>
#include <signal.h>

struct header {
  char * n;
  char * v;
};


char request[10000];
char request2[10000];
char response[1000];
char response2[10000];
char hbuffer[10000];
char buffer[2000];

char *reqline, *method, *url, *ver, *scheme, *hostname, *port, *filename;

struct header h[100];
struct hostent * he;
struct sockaddr_in local, remote, server;

int pid;

int main(){
    FILE * fin;
    int c,n,i,j,t,s,s2,s3,len,yes=1;

    // Viene creato un socket TCP
    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == -1){ // Verifichiamo che il socket sia stato creato correttamente
        perror("Socket Fallita\n"); 
        return 1;
    }

    t = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if(t == -1){
        perror("setsockopt fallita"); 
        return 1;
    }

    local.sin_family = AF_INET;
    local.sin_port = htons(20161);
    local.sin_addr.s_addr = 0;

    // Viene effettuato il binding del socket
    /* Associa il socket s alla porta 8082 e all'indirizzo IP
    
        Legare il socket ad un port e anche ad un indirizzp IP se la macchina ha più interfacce IP (più indirizzi), tramite la chiamata a sistema bind()
    
    */
    if(bind(s, (struct sockaddr *)&local, sizeof(struct sockaddr_in)) == -1){
        perror("bind fallita");
        return -1;
    }

    /* 
        La chiamata a sistema listen() mette il socket in ascolto sulla porta specificata, in attesa di connessioni in entrata. Il primo parametro è il socket, il secondo è il numero massimo di connessioni pendenti che il sistema può accettare per quel socket.
    */

    if(listen(s, 10) == -1){
        perror("Listen Fallita"); 
        return -1;
    }

    remote.sin_family = AF_INET;
    remote.sin_port = htons(0);
    remote.sin_addr.s_addr = 0;

    len = sizeof(struct sockaddr_in);

    while(1){
        // Da questo momento il server è in ascolto

        /*
            La chiamata a sistema accept() estrae la prima connessione in coda di connessioni pendenti per il socket s, creando un nuovo socket per la comunicazione con il client. Il socket s rimane in ascolto per ulteriori connessioni in entrata.
        */
        s2 = accept(s, (struct sockaddr *)&remote, &len);
        printf("Remote address: %.8X\n",remote.sin_addr.s_addr);

        if(fork())  // Processo padre 
            continue;

        if(s2 == -1){
            perror("Accept Fallita"); 
            return -1;
        }

        /*
            La funzione bzero() elimina i dati nella memoria puntata da hbuffer, per una lunghezza di 10000 byte. In questo modo, se il client invia una richiesta più corta di 10000 byte, i byte rimanenti saranno comunque a 0.
        */
        bzero(hbuffer, 10000);
        bzero(h, sizeof(struct header)*100);

        reqline = h[0].n = hbuffer;

        for (i=0,j=0; read(s2,hbuffer+i,1); i++) {
            printf("%c",hbuffer[i]);
            if(hbuffer[i]=='\n' && hbuffer[i-1]=='\r'){
                hbuffer[i-1]=0; // Termino il token attuale
                if (!h[j].n[0]) break;
                h[++j].n=hbuffer+i+1;
            }
            if(hbuffer[i]==':' && !h[j].v && j>0){
                hbuffer[i]=0;
                h[j].v = hbuffer + i + 1;
            }
        }

        printf("Request line: %s\n",reqline);

        method = reqline;
        for(i=0;i<100 && reqline[i]!=' ';i++); 
        reqline[i++]=0;

	    url=reqline+i;
	    for(;i<100 && reqline[i]!=' ';i++);
        reqline[i++]=0;

        ver=reqline+i;
        for(;i<100 && reqline[i]!='\r';i++);
        reqline[i++]=0;

        // Se il metodo richiesto al proxy è GET
        if(!strcmp(method,"GET")){
            scheme=url;
            printf("url=%s\n",url);

            for(i=0;url[i]!=':' && url[i] ;i++);
            if(url[i]==':')
                url[i++]=0;
            else{
                printf("Parse error, expected ':'"); 
                exit(1);
            }

            if(url[i]!='/' || url[i+1] !='/'){
                printf("Parse error, expected '//'");
                exit(1);
            }
            i=i+2; 
            hostname=url+i;
            for(;url[i]!='/'&& url[i];i++);	
            if(url[i]=='/') 
                url[i++]=0;
            else{
                printf("Parse error, expected '/'"); 
                exit(1);
            }
            filename = url+i;

            printf("Schema: %s, hostname: %s, filename: %s\n",scheme,hostname,filename); 

            he = gethostbyname(hostname);
            printf("%d.%d.%d.%d\n",(unsigned char) he->h_addr[0],(unsigned char) he->h_addr[1],(unsigned char) he->h_addr[2],(unsigned char) he->h_addr[3]);

            s3 = socket(AF_INET, SOCK_STREAM, 0 );
            if(s3 == -1){
                printf("errno = %d\n",errno); 
                perror("Socket Fallita"); 
                exit(-1); 
            }

            server.sin_family = AF_INET;
            server.sin_port =htons(80);
            server.sin_addr.s_addr = *(unsigned int *)(he->h_addr);

            if(-1 == connect(s3,(struct sockaddr *) &server, sizeof(struct sockaddr_in))){
                perror("Connect Fallita");
                exit(1);
            }	

            sprintf(request,"GET /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n\r\n",filename,hostname);
            printf("%s\n",request);
            write(s3,request,strlen(request));
            while(t=read(s3,buffer,2000))
                write(s2,buffer,t);

            close(s3);
		}
        // Altrimenti se il metodo richiestp è CONNECT
        else if(!strcmp("CONNECT",method)) { // it is a connect  host:port 
            hostname=url;
            for(i=0;url[i]!=':';i++); url[i]=0;
            port=url+i+1;
            printf("hostname:%s, port:%s\n",hostname,port);
            
            he = gethostbyname(hostname);
            if(he == NULL){
                printf("Gethostbyname Fallita\n");
                return 1;
            }
            
            printf("Connecting to address = %u.%u.%u.%u\n", (unsigned char ) he->h_addr[0],(unsigned char ) he->h_addr[1],(unsigned char ) he->h_addr[2],(unsigned char ) he->h_addr[3]); 			
            
            
            s3=socket(AF_INET,SOCK_STREAM,0);
            if(s3==-1){
                perror("Socket to server fallita");
                return 1;
            }

            server.sin_family=AF_INET;
            server.sin_port=htons((unsigned short)atoi(port));
            server.sin_addr.s_addr=*(unsigned int*) he->h_addr;	

            t=connect(s3,(struct sockaddr *)&server,sizeof(struct sockaddr_in));		
            if(t==-1){
                perror("Connect to server fallita");
                exit(0);
            }

            sprintf(response,"HTTP/1.1 200 Established\r\n\r\n");
            write(s2,response,strlen(response));
            if(!(pid=fork())){ //Child
                while(t=read(s2,request2,2000)){	
                    write(s3,request2,t);
                //printf("CL >>>(%d)%s \n",t,hostname); //SOLO PER CHECK
                }	
                exit(0);
            }
            else{ //Parent	
                while(t=read(s3,response2,2000)){	
                    write(s2,response2,t);
                //printf("CL <<<(%d)%s \n",t,hostname);
                }	
                kill(pid,SIGTERM);
                close(s3);
            }	
		}
        else{
			sprintf(response,"HTTP/1.1 501 Not Implemented\r\n\r\n");
    	    write(s2,response,strlen(response));
	    }
        close(s2);
        exit(1);
    }
    close(s);

    return 0;
}

