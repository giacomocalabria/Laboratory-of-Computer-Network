#include <sys/types.h>          /* See NOTES */ 
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

struct headers{
  char *n;
  char *v;
};

struct headers h[100];
char hbuf[5000]; 

//L'header non può esser lungo quanto si vuole

//       int socket(int domain, int type, int protocol);

struct sockaddr_in remote_addr;
unsigned char * ip;
char * request = "GET / HTTP/1.0\r\n\r\n";
char request2[100];
unsigned char response[1000001];
int main()
{
int s,t;

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

h[0].n = hbuf[0];
while(t=read(s,response,1000000)){
	for(int i=0; i<t;i++) printf("%c",response[i]);
}
}

// 142.250.200.36
// struct sockaddr_in {
               //sa_family_t    sin_family; /* address family: AF_INET */
               //in_port_t      sin_port;   /* port in network byte order */
               //struct in_addr sin_addr;   /* internet address */
           //};
//
           ///* Internet address. */
           //struct in_addr {
               //uint32_t       s_addr;     /* address in network byte order */
           //};

