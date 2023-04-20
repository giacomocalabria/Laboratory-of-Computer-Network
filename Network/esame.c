#include <arpa/inet.h>
#include<errno.h>
#include<stdio.h>
#include<signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <net/if.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <time.h>

#define MIN(x,y) (((x) > (y) ) ? (y) : (x) )
#define MAX(x,y) (((x) < (y) ) ? (y) : (x) )

#define MAXFRAME 10000
#define TIMER_USECS 1000  //1000 microsecs: the tick is 1ms
#define RXBUFSIZE 32768
#define TXBUFSIZE 120000

struct sigaction action_io, action_timer;
sigset_t mymask;
unsigned char l2buffer[MAXFRAME];
struct sockaddr_ll;
struct pollfd fds[1];
int fdfl;
long long int tick=0; 
int unique_s;
int fl;

struct sockaddr_ll sll;

int printbuf(void * b, int size){
	int i;
	unsigned char * c = (unsigned char *) b;
	for(i=0;i<size;i++)
		;//printf("%.2x(%.3d) ", c[i],c[i]);
	;//printf("\n");
}

unsigned char myip[4] = { 88,80,187,84 };
unsigned char mymac[6] = {0xf2,0x3c,0x91,0xdb,0xc2,0x98};
unsigned char mask[4] = { 255,255,255,0 };
unsigned char gateway[4] = { 88,80,187,1 };


struct icmp_packet{
unsigned char type;
unsigned char code;
unsigned short checksum;
unsigned short id;
unsigned short seq;
unsigned char data[20];
};

struct ip_datagram {
unsigned char ver_ihl;
unsigned char tos;
unsigned short totlen;
unsigned short id;
unsigned short fl_offs;
unsigned char ttl;
unsigned char proto;
unsigned short checksum;
unsigned int srcaddr;
unsigned int dstaddr;
unsigned char payload[20];
};




int resolve_mac(unsigned int destip, unsigned char * destmac);


struct arp_packet {
unsigned short int htype;
unsigned short int ptype;
unsigned char hlen;
unsigned char plen;
unsigned short op;
unsigned char srcmac[6];
unsigned char srcip[4];
unsigned char dstmac[6];
unsigned char dstip[4];
};

struct ethernet_frame {
unsigned char dstmac[6];
unsigned char srcmac[6];
unsigned short int type;
unsigned char payload[10];
};
int s;


int are_equal(void *a1, void *a2, int l)
{
char *b1 = (char *)a1;
char *b2 = (char *)a2;
for(int i=0; i<l; i++) if(b1[i]!=b2[i]) return 0;
return 1;
}

unsigned short int compl1( char * b, int len)
{
unsigned short total = 0;
unsigned short prev = 0;
unsigned short *p = (unsigned short * ) b;
int i;
for(i=0; i < len/2 ; i++){
	total += ntohs(p[i]);
	if (total < prev ) total++;
	prev = total;
	} 
if ( i*2 != len){
	//total += htons(b[len-1]<<8); 
	total += htons(p[len/2])&0xFF00;
	if (total < prev ) total++;
	prev = total;
	} 
return (total);
}

unsigned short int checksum2 ( char * b1, int len1, char* b2, int len2)
{
unsigned short prev, total;
prev = compl1(b1,len1); 
total = (prev + compl1(b2,len2));
if (total < prev ) total++;
return (0xFFFF - total);
}

unsigned short int checksum ( char * b, int len)
{
unsigned short total = 0;
unsigned short prev = 0;
unsigned short *p = (unsigned short * ) b;
int i;
for(i=0; i < len/2 ; i++){
	total += ntohs(p[i]);
	if (total < prev ) total++;
	prev = total;
	} 
if ( i*2 != len){
	//total += htons(b[len-1]<<8); 
	total += htons(p[len/2])&0xFF00;
	if (total < prev ) total++;
	prev = total;
	} 
return (0xFFFF-total);
}

void forge_icmp_echo(struct icmp_packet * icmp, int payloadsize)
{
int i;
icmp->type=8;
icmp->code=0;
icmp->checksum=htons(0);
icmp->id=htons(0x1234);
icmp->seq=htons(1);
for(i=0;i<payloadsize;i++)
	 icmp->data[i]=i&0xFF;
icmp->checksum=htons(checksum((unsigned char *)icmp , 8 + payloadsize));
}

void forge_ip(struct ip_datagram * ip, int payloadsize, char proto,unsigned int target )
{
ip->ver_ihl=0x45;
ip->tos=0;
ip->totlen=htons(20+payloadsize);
ip->id = htons(0xABCD); //to change to random number
ip->fl_offs=htons(0);
ip->ttl=128;
ip->proto = proto;
ip->checksum=htons(0);
ip->srcaddr= *(unsigned int*)myip;
ip->dstaddr= target;
ip->checksum = htons(checksum((unsigned char *)ip,20)); 
}

void forge_ethernet(struct ethernet_frame * eth, unsigned char * dest, unsigned short type)
{
memcpy(eth->dstmac,dest,6);
memcpy(eth->srcmac,mymac,6);
eth->type=htons(type);
 
};


int send_ip(unsigned char * payload, unsigned char * targetip, int payloadlen, unsigned char proto)
{
int i,t,len ;
unsigned char destmac[6];
unsigned char packet[2000];
struct ethernet_frame * eth = (struct ethernet_frame *) packet;
struct ip_datagram * ip = (struct ip_datagram *) eth->payload; 

/**** HOST ROUTING */
if( ((*(unsigned int*)targetip) & (*(unsigned int*) mask)) == ((*(unsigned int*)myip) & (*(unsigned int*) mask)))  //The
	t = resolve_mac(*(unsigned int *)targetip, destmac); // if yes
else
	t = resolve_mac(*(unsigned int *)gateway, destmac); // if not

if(t==-1) return -1;

;//printf("destmac: ");printbuf(destmac,6);

forge_ethernet(eth,destmac,0x0800);
forge_ip(ip,payloadlen,proto,*(unsigned int *)targetip); 
memcpy(ip->payload,payload,payloadlen);
/*
;//printf("\nIP: ");printbuf(ip,20);
;//printf("\nTCP: ");printbuf(ip->payload,payloadlen);
;//printf("\n");
*/
//printbuf(packet+14,20+payloadlen);
len=sizeof(sll);
bzero(&sll,len);
sll.sll_family=AF_PACKET;
sll.sll_ifindex = if_nametoindex("eth0");
t=sendto(unique_s,packet,14+20+payloadlen, 0,(struct sockaddr *)&sll,len);
if (t == -1) {perror("sendto failed"); return -1;}
}
		
#define MAX_ARP 200

struct arpcacheline {
unsigned int key; //IP address
unsigned char mac[6]; //Mac address
}arpcache[MAX_ARP];

/* Congestion Control Parameters*/
#define ALPHA 1
#define BETA 4
#define KAPPA 4
#define SLOW_START 0
#define CONG_AVOID 1
#define FAST_RECOV 2
#define INIT_CGWIN 1//in MSS
#define INIT_THRESH 16 //in MSS
#define INIT_TIMEOUT 100000/TIMER_USECS


#define TCP_PROTO 6
#define MAX_FD 10
#define TCP_MSS 1400 
// Socket states (file descriptor)
#define FREE 0 
#define TCP_UNBOUND 1
#define TCP_BOUND 2
#define TCB_CREATED 3
// TCP Finite State Machine INPUTS
#define APP_ACTIVE_OPEN 1
#define APP_PASSIVE_OPEN 2
#define PKT_RCV 3
#define APP_CLOSE 4
#define TIMEOUT 5
// TCB states (TCP)
#define TCP_CLOSED 10 // initial state
#define LISTEN 11  // represents waiting for a connection request from any remote TCP and port.

#define SYN_SENT 12 // represents waiting for a matching connection request after having sent a connection request.

#define SYN_RECEIVED 13 //  represents waiting for a confirming connection request acknowledgment after having both received and sent a connection request.

#define ESTABLISHED 14 // - represents an open connection, data received can be delivered to the user.  The normal state for the data transfer phase of the connection.
#define FIN_WAIT_1 15 // waiting for a connection termination request from the remote TCP, or an acknowledgment of the conne
#define FIN_WAIT_2 16 // waiting for a connection termination request from the remote TCP.
#define CLOSE_WAIT 17 // waiting for a connection termination request from the local user.
#define CLOSING 18  // waiting for a connection termination request acknowledgment from the remote TCP.
#define LAST_ACK 19 // waiting for an acknowledgment of the connection termination request previously sent to the remote TCP
#define TIME_WAIT 20 //waiting for enough time to pass to be sure the remote TCP received the acknowledgment of its connecti

#define FIN 0x01
#define SYN 0x02
#define RST	0x04
#define PSH 0x08
#define ACK 0x10
#define URG 0x20

int myerrno;

void myperror(char *message) {;//printf("%s: %s\n",message,strerror(myerrno));
}


struct tcp_segment {
unsigned short s_port;
unsigned short d_port;
unsigned int seq;
unsigned int ack;
unsigned char d_offs_res; 
unsigned char flags; 
unsigned short window;
unsigned short checksum;
unsigned short urgp;
unsigned char payload[TCP_MSS];
};
/*
                     +--------+--------+--------+--------+
                     |           Source Address          |
                     +--------+--------+--------+--------+
                     |         Destination Address       |
                     +--------+--------+--------+--------+
                     |  zero  |  PTCL  |    TCP Length   |
                     +--------+--------+--------+--------+

*/

struct pseudoheader {
unsigned int s_addr, d_addr;
unsigned char zero;
unsigned char prot;
unsigned short len;
};

struct rxcontrol{
unsigned int stream_offs;
unsigned int streamsegmentsize; 
struct rxcontrol * next;
};

struct txcontrolbuf{
struct tcp_segment * segment;
int totlen;
int payloadlen;
long long int expire;
struct txcontrolbuf * next;
long long int txtime;
int retry;
};

struct tcpctrlblk{
struct txcontrolbuf *txfirst, * txlast;
int st;
unsigned short r_port;
unsigned int r_addr;
unsigned char * rxbuffer; 
unsigned int rx_win_start;
struct rxcontrol * unack;
unsigned int cumulativeack;
unsigned int ack_offs, seq_offs;
unsigned long long timeout;
unsigned int sequence;
unsigned int txfree;
unsigned int mss;
unsigned int cgwin;
unsigned short adwin;
unsigned int threshold;
unsigned int rtt_e;
unsigned int Drtt_e;
unsigned int cong_st;
unsigned int stream_end;
unsigned int fsm_timer;
unsigned int last_ack;
unsigned int repeated_acks;
};

struct socket_info{
struct tcpctrlblk * tcb;
int st; 
unsigned short l_port;
unsigned int l_addr;
struct tcpctrlblk * tcblist; //Backlog listen queue
int bl; //backlog length;
}fdinfo[MAX_FD];

int prepare_tcp(int s, unsigned char flags, unsigned char * payload, int payloadlen){
struct tcpctrlblk *t = fdinfo[s].tcb;
struct tcp_segment * tcp;
struct txcontrolbuf * txcb = (struct txcontrolbuf*) malloc(sizeof( struct txcontrolbuf));

//txcb->expire = tick -1 ; 
txcb->payloadlen = payloadlen;
txcb->totlen = payloadlen + 20;
txcb->retry = 0;
tcp = txcb->segment = (struct tcp_segment *) malloc(sizeof(struct tcp_segment));

tcp->s_port = fdinfo[s].l_port ;
tcp->d_port = t->r_port;
if( t->r_port == 0 )
	;//printf("Illegal Packet...\n");
tcp->seq = htonl(t->seq_offs+t->sequence);
tcp->d_offs_res=0x50;
tcp->flags=flags&0x3F;
tcp->urgp=0;
if(payload != NULL)
	memcpy(tcp->payload,payload,payloadlen);
txcb->next=NULL;
if(t->txfirst == NULL) { t->txlast = t->txfirst = txcb;}
else {t->txlast->next = txcb; t->txlast = t->txlast->next; }
printf("Packet seq inserted %d:%d\n",t->sequence, t->sequence+payloadlen);
t->sequence += payloadlen; 
//DEFERRED FIELDS
//tcp->ack;
//tcp->window;
//tcp->checksum=0;
}

int resolve_mac(unsigned int destip, unsigned char * destmac) 
{
int len,n,i;
clock_t start;
unsigned char pkt[1500];
struct ethernet_frame *eth;
struct arp_packet *arp;
for(i=0;i<MAX_ARP && (arpcache[i].key!=0);i++)
		if(!memcmp(&arpcache[i].key,&destip,4)) break;
if(arpcache[i].key){ //If found return 
	memcpy(destmac,arpcache[i].mac,6); 
	return 0; }
eth = (struct ethernet_frame *) pkt;
arp = (struct arp_packet *) eth->payload; 
for(i=0;i<6;i++) eth->dstmac[i]=0xff;
for(i=0;i<6;i++) eth->srcmac[i]=mymac[i];
eth->type=htons(0x0806);
arp->htype=htons(1);
arp->ptype=htons(0x0800);
arp->hlen=6;
arp->plen=4;
arp->op=htons(1);
for(i=0;i<6;i++) arp->srcmac[i]=mymac[i];
for(i=0;i<4;i++) arp->srcip[i]=myip[i];
for(i=0;i<6;i++) arp->dstmac[i]=0;
for(i=0;i<4;i++) arp->dstip[i]=((unsigned char*) &destip)[i];
//printbuf(pkt,14+sizeof(struct arp_packet));
bzero(&sll,sizeof(struct sockaddr_ll));
sll.sll_family = AF_PACKET;
sll.sll_ifindex = if_nametoindex("eth0");
len = sizeof(sll);
n=sendto(unique_s,pkt,14+sizeof(struct arp_packet), 0,(struct sockaddr *)&sll,len);
fl--;

sigprocmask(SIG_UNBLOCK,&mymask,NULL);
start=clock();
printf("Waiting ARP...\n");
while(pause()){ //wake up only upon signals
for(i=0;(i<MAX_ARP) && (arpcache[i].key!=0);i++)
		if(!memcmp(&arpcache[i].key,&destip,4)) break;
if(arpcache[i].key){ 
		memcpy(destmac,arpcache[i].mac,6);
		sigprocmask(SIG_BLOCK,&mymask,NULL);
		fl++;
		return 0;
	}
if ((clock()-start) > CLOCKS_PER_SEC/100) break;  
	}
sigprocmask(SIG_BLOCK,&mymask,NULL);
fl++;
//sigaddset(&mymask,SIGALRM);   
return -1 ; //Not resolved
}

void update_tcp_header(int s, struct txcontrolbuf *txctrl){
struct tcpctrlblk * tcb  = fdinfo[s].tcb;
struct pseudoheader pseudo;
pseudo.s_addr = fdinfo[s].l_addr;
pseudo.d_addr = tcb->r_addr;
pseudo.zero = 0;
pseudo.prot = 6;
pseudo.len = htons(txctrl->totlen);
txctrl->segment->checksum = htons(0); 
txctrl->segment->ack = htonl(tcb->ack_offs + tcb->cumulativeack);
txctrl->segment->window = htons(tcb->adwin);
txctrl->segment->checksum = htons(checksum2((unsigned char*)&pseudo, 12, (unsigned char*) txctrl->segment, txctrl->totlen));  
}


int mysocket(int family, int type, int proto)
{
int i;
if (( family == AF_INET ) && (type == SOCK_STREAM) && (proto ==0)){
	for(i=3; i<MAX_FD && fdinfo[i].st!=FREE;i++);
	if(i==MAX_FD) {myerrno = ENFILE; return -1;}  
	else {
		bzero(fdinfo+i, sizeof(struct socket_info));
		fdinfo[i].st = TCP_UNBOUND;
		myerrno = 0;
		return i;
		}
}else {myerrno = EINVAL; return -1; }
}

#define MIN_PORT 19000
#define MAX_PORT 19999
int last_port=MIN_PORT;
int port_in_use( unsigned short port ){
int s;
for ( s=3; s<MAX_FD; s++)
	if (fdinfo[s].st != FREE && fdinfo[s].st!=TCP_UNBOUND)
		if(fdinfo[s].l_port == port)
			return 1;
return 0;
}

unsigned short int get_free_port()
{
unsigned short p;
for ( p = last_port; p<MAX_PORT && port_in_use(p); p++);
if (p<MAX_PORT) return last_port=p;
for ( p = MIN_PORT; p<last_port && port_in_use(p); p++);
if (p<last_port) return last_port=p;
return 0;
}

int mybind(int s, struct sockaddr * addr, int addrlen){
if((addr->sa_family == AF_INET)){
	struct sockaddr_in * a = (struct sockaddr_in*) addr;
	if ( s >= 3 && s<MAX_FD){
		if(fdinfo[s].st != TCP_UNBOUND){myerrno = EINVAL; return -1;}
		if(a->sin_port && port_in_use(a->sin_port)) {myerrno = EADDRINUSE; return -1;} 
		fdinfo[s].l_port = (a->sin_port)?a->sin_port:get_free_port();   
		if(fdinfo[s].l_port == 0 ) {myerrno = ENOMEM; return -1;}
		fdinfo[s].l_addr = (a->sin_addr.s_addr)?a->sin_addr.s_addr:*(unsigned int*)myip;
		fdinfo[s].st = TCP_BOUND;
		myerrno = 0;
		return 0;
	}
	else { myerrno = EBADF ; return -1;}
}
else { myerrno = EINVAL; return -1; }
}

int fsm(int s, int event, struct ip_datagram * ip)
{
struct tcpctrlblk * tcb = fdinfo[s].tcb;
;//printf("Socket: %d FSM State =%d, Input=%d \n",s,tcb->st,event);	
struct tcp_segment * tcp;
int i;
if(ip != NULL)
 tcp = (struct tcp_segment * )((char*)ip+((ip->ver_ihl&0xF)*4));
switch(tcb->st){
	case TCP_CLOSED:
		if(event == APP_ACTIVE_OPEN) {
			tcb->rxbuffer = (unsigned char*) malloc(RXBUFSIZE);
			tcb->txfree = TXBUFSIZE;
			tcb->seq_offs=rand();
			tcb->ack_offs=0;
			tcb->stream_end=0xFFFFFFFF; //Max file
			tcb->mss = TCP_MSS;
			tcb->sequence=0;
			tcb->rx_win_start=0;
			tcb->cumulativeack =0;
			tcb->threshold = INIT_THRESH * TCP_MSS;
			tcb->cgwin = INIT_CGWIN* TCP_MSS;
			tcb->timeout = INIT_TIMEOUT;
			tcb->rtt_e = 0;
			tcb->Drtt_e = 0;
			tcb->cong_st = SLOW_START;
			tcb->adwin =32768;
			prepare_tcp(s,SYN,NULL,0);
			tcb->st = SYN_SENT;	
		}
		break;
	
	case SYN_SENT:	
		if(event == PKT_RCV){
			if((tcp->flags&SYN) && (tcp->flags&ACK) && (htonl(tcp->ack)==tcb->seq_offs + 1)){
				tcb->seq_offs ++;
				tcb->last_ack=tcp->ack;
				tcb->ack_offs = htonl(tcp->seq) + 1;	
				free(tcb->txfirst->segment);
				free(tcb->txfirst);
				tcb->txfirst = tcb->txlast = NULL;	
				prepare_tcp(s,ACK,NULL,0);	
				tcb->st = ESTABLISHED;
				}
			}
		break;

	 case ESTABLISHED: 
			if(event ==PKT_RCV && (tcp->flags&FIN))
				tcb->st = CLOSE_WAIT;
			else if(event == APP_CLOSE ){
    			prepare_tcp(s,FIN|ACK,NULL,0); //we announce no more data are sent...
    			tcb->st = FIN_WAIT_1;
    			}
  break;

	
	 case  CLOSE_WAIT:
			if(event == APP_CLOSE ){
					prepare_tcp(s,FIN|ACK,NULL,0);
					tcb->st = LAST_ACK;
			}		
	  break;

		case LAST_ACK:
			if((event == PKT_RCV) && (tcp->flags&ACK)	){
					if(htonl(tcp->ack) == (tcb->seq_offs + tcb->sequence + 1)){
						tcb->st = TCP_CLOSED;
					 	tcb->txfirst = tcb->txlast = NULL;	
				}
			}
		break;
case LISTEN:
  if((event == PKT_RCV) && ((tcp->flags)&SYN)){
    tcb->rxbuffer=(unsigned char*)malloc(RXBUFSIZE);
    tcb->seq_offs=rand();
    tcb->txfree = TXBUFSIZE; //Dynamic buffer
    tcb->ack_offs=htonl(tcp->seq)+1;
    tcb->r_port = tcp->s_port;
    tcb->r_addr = ip->srcaddr;
		tcb->rx_win_start=0;
    tcb->cumulativeack=0;
    tcb->adwin=RXBUFSIZE;
    tcb->mss=TCP_MSS;
		tcb->threshold = INIT_THRESH * TCP_MSS;
		tcb->cgwin = INIT_CGWIN* TCP_MSS;
    tcb->timeout = INIT_TIMEOUT; 
		tcb->rtt_e = 0;
		tcb->Drtt_e = 0;
		tcb->cong_st = SLOW_START;
    prepare_tcp(s,SYN|ACK,NULL,0);
    tcb->st = SYN_RECEIVED;
    }
    break;
case SYN_RECEIVED:
  if(((event == PKT_RCV) && ((tcp->flags)&ACK)) &&!((tcp->flags)&SYN)){
    if(htonl(tcp->ack) == tcb->seq_offs + 1){
				free(tcb->txfirst->segment);
      free(tcb->txfirst);
      tcb->seq_offs++;
			tcb->last_ack = tcp->ack;
      tcb->txfirst = tcb->txlast = NULL; //Remove SYN+ACK from TXbuffer
      tcb->ack_offs=htonl(tcp->seq);
      for(i=0;i<fdinfo[s].bl && fdinfo[s].tcblist[i].st!=FREE;i++);
      if (fdinfo[s].tcblist[i].st!=FREE)
            prepare_tcp(s,RST,NULL,0);
        else {
              fdinfo[s].tcblist[i]=*tcb;
              fdinfo[s].tcblist[i].st = ESTABLISHED;
            }
      tcb->r_port = 0; // Detach the remote binding from the listening socket
      tcb->r_addr = 0;// Detach the remote binding from the listening socket
      tcb->st = LISTEN;
      }
    }
  break;

case FIN_WAIT_1:
  if((event == PKT_RCV) && ((tcp->flags)&FIN)){
    tcb->st = CLOSING;
    // Acknoweldgment will be sent as cumulative.
    }
  else if((event == PKT_RCV)&&((tcp->flags)&ACK))
        if(htonl(tcp->ack) == tcb->seq_offs + tcb->sequence + 1)
          tcb->st = FIN_WAIT_2;
  break;

case FIN_WAIT_2:
  if((event == PKT_RCV) && ((tcp->flags)&FIN)){
		tcb->fsm_timer = tick + tcb->timeout *4;
    tcb->st = TIME_WAIT;
    }
  break;


case CLOSING:
  if((event == PKT_RCV)&&((tcp->flags)&ACK)) //Receiving FIN's ACK+1
        if(htonl(tcp->ack) == tcb->seq_offs + tcb->sequence + 1){
					tcb->fsm_timer = tick + tcb->timeout *4;
          tcb->st = TIME_WAIT;
					}
				
  break;


case TIME_WAIT: 
		if(event == TIMEOUT){
				while(tcb->txfirst!=NULL){
					struct txcontrolbuf * tmp = tcb->txfirst;
					tcb->txfirst = tcb->txfirst->next;
					free(tmp);
				}
				while(tcb->unack=NULL){
					struct rxcontrol * tmp = tcb->unack;
					tcb->unack = tcb->unack->next;
					free(tmp);
				}
				free(tcb->rxbuffer);
				free(fdinfo[s].tcb);
				bzero(fdinfo+s,sizeof(struct socket_info));

				fdinfo[s].st=FREE;
			}	
break;



	}
}

int myconnect(int s, struct sockaddr * addr, int addrlen){
if((addr->sa_family == AF_INET)){
	struct sockaddr_in * a = (struct sockaddr_in*) addr;
	struct sockaddr_in local;
	if ( s >= 3 && s<MAX_FD){
			if(fdinfo[s].st == TCP_UNBOUND){
					local.sin_port=htons(0);
					local.sin_addr.s_addr = htonl(0);
					local.sin_family = AF_INET;
					if(-1 == mybind(s,(struct sockaddr *) &local, sizeof(struct sockaddr_in)))	{myperror("implicit binding failed\n"); return -1; }
			}
			if(fdinfo[s].st == TCP_BOUND){
					fdinfo[s].tcb = (struct tcpctrlblk *) malloc(sizeof(struct tcpctrlblk));
					bzero(fdinfo[s].tcb, sizeof(struct tcpctrlblk));
					fdinfo[s].st = TCB_CREATED;
					fdinfo[s].tcb->st = TCP_CLOSED;
					fdinfo[s].tcb->r_port = a->sin_port;
					fdinfo[s].tcb->r_addr = a->sin_addr.s_addr;
					fsm(s,APP_ACTIVE_OPEN,NULL); 				
			} else {myerrno = EBADF; return -1; } 	
			while(sleep(10)){
					if(fdinfo[s].tcb->st == ESTABLISHED ) return 0;
					if(fdinfo[s].tcb->st == TCP_CLOSED ){ myerrno = ECONNREFUSED; return -1;} 
			}	
			myerrno=ETIMEDOUT; return -1;
}
else { myerrno = EBADF; return -1; }
}
else { myerrno = EINVAL; return -1; }
}

int mywrite(int s, unsigned char * buffer, int maxlen){
int len,totlen=0,j,actual_len;
if(fdinfo[s].st != TCB_CREATED || fdinfo[s].tcb->st != ESTABLISHED ){ myerrno = EINVAL; return -1; }
if(maxlen == 0) return 0;

do{
actual_len = MIN(maxlen,fdinfo[s].tcb->txfree);
if ((actual_len !=0) || (fdinfo[s].tcb->st == TCP_CLOSED)) break;
}while(pause());

for(j=0;j<actual_len; j+=fdinfo[s].tcb->mss){
		len = MIN(fdinfo[s].tcb->mss, actual_len-j);
if(-1 == sigprocmask(SIG_BLOCK, &mymask, NULL)){perror("sigprocmask"); return -1 ;}
		prepare_tcp(s,ACK,buffer+j,len);
if(-1 == sigprocmask(SIG_UNBLOCK, &mymask, NULL)){perror("sigprocmask"); return -1 ;}
		fdinfo[s].tcb->txfree -= len;
		totlen += len;
	}
return totlen;	
} 


int myread(int s, unsigned char *buffer, int maxlen)
{
int j,actual_len;
if((fdinfo[s].st != TCB_CREATED) || (fdinfo[s].tcb->st < ESTABLISHED )){ myerrno = EINVAL; return -1; }
actual_len = MIN(maxlen,fdinfo[s].tcb->cumulativeack - fdinfo[s].tcb->rx_win_start);
if(fdinfo[s].tcb->cumulativeack > fdinfo[s].tcb->stream_end) actual_len --;
if(actual_len==0){
		while(pause()){
			actual_len = MIN(maxlen,fdinfo[s].tcb->cumulativeack - fdinfo[s].tcb->rx_win_start);
			if(fdinfo[s].tcb->cumulativeack > fdinfo[s].tcb->stream_end) actual_len --;
			if(actual_len!=0) break;		
			if(fdinfo[s].tcb->rx_win_start) 
				if(fdinfo[s].tcb->rx_win_start==fdinfo[s].tcb->stream_end) {return 0;}
    	if ((fdinfo[s].tcb->st == CLOSE_WAIT) && (fdinfo[s].tcb->unack == NULL ) ) {return 0;} // FIN received and acknowledged
		}
	}
for(j=0; j<actual_len; j++){
	buffer[j]=fdinfo[s].tcb->rxbuffer[(fdinfo[s].tcb->rx_win_start + j)%RXBUFSIZE];
}
fdinfo[s].tcb->rx_win_start += j;
return j;
}

int myclose(int s){
if((fdinfo[s].st == TCP_CLOSED) || (fdinfo[s].st == TCP_UNBOUND)) { myerrno = EBADF; return -1;}
fsm(s,APP_CLOSE,NULL);
return 0;
}

void mytimer(int number){
int i,tot;
struct txcontrolbuf * txcb;
if(-1 == sigprocmask(SIG_BLOCK, &mymask, NULL)){perror("sigprocmask"); return ;}
fl++;
tick++;
if(tick%(1000000/TIMER_USECS)){
	//;//printf("Mytimer Called\n");
}
if (fl > 1) printf("Overlap Timer\n");
for(i=0;i<MAX_FD;i++){
	if(fdinfo[i].st == TCB_CREATED){
		struct tcpctrlblk * tcb = fdinfo[i].tcb;
		if((tcb->fsm_timer!=0 ) && (tcb->fsm_timer < tick)){
			fsm(i,TIMEOUT,NULL);	
			continue;
			}
		
		for(tot=0,txcb=tcb->txfirst;  txcb!=NULL && (tot<MIN(tcb->cgwin,tcb->adwin)); tot+=txcb->totlen, txcb = txcb->next){
			if(txcb->retry==0){ //first send
				txcb->txtime=tick;
				txcb->retry ++;
				printf("TX Seq:%d,(%d times), Sock = %d, Timeout = %lld, Thresh: %d TxWin:%d (%.1f), ST: %d, RTT_E: %d\n",htonl(txcb->segment->seq) - fdinfo[i].tcb->seq_offs,txcb->retry,i,tcb->timeout,tcb->threshold, tcb->cgwin,tcb->cgwin/(float)tcb->mss,tcb->cong_st,tcb->rtt_e);
				update_tcp_header(i, txcb);
				send_ip((unsigned char*) txcb->segment, (unsigned char*) &fdinfo[i].tcb->r_addr, txcb->totlen, TCP_PROTO);
			}
			else if(txcb->txtime+tcb->timeout < tick ){
				//txcb->expire = tick + tcb->timeout;
				txcb->txtime=tick;
				txcb->retry ++;
				if(tcb->cong_st == CONG_AVOID) tcb->threshold= MAX(tcb->cgwin/2,2*tcb->mss);
				if(tcb->cong_st == FAST_RECOV) tcb->threshold/=2;
				tcb->cgwin = INIT_CGWIN* tcb->mss;
				tcb->timeout *= 2;
				tcb->cong_st = SLOW_START;
				update_tcp_header(i, txcb);
				send_ip((unsigned char*) txcb->segment, (unsigned char*) &fdinfo[i].tcb->r_addr, txcb->totlen, TCP_PROTO);
				printf("TX Seq:%d,(%d times), Sock = %d, Timeout = %lld, Thresh: %d TxWin/MSS: %f, ST: %d\n",htonl(txcb->segment->seq) - fdinfo[i].tcb->seq_offs,txcb->retry,i,tcb->timeout,tcb->threshold, tcb->cgwin/(float)tcb->mss,tcb->cong_st);
				}
		}
	}
}
	fl--;
	if(-1 == sigprocmask(SIG_UNBLOCK, &mymask, NULL)){perror("sigprocmask"); return ;}
}

void myio(int number)
{
int i,len,size,shifter;
//;//printf("Myio Called\n");
struct ethernet_frame * eth=(struct ethernet_frame *)l2buffer;

if(-1 == sigprocmask(SIG_BLOCK, &mymask, NULL)){perror("sigprocmask"); return ;}
fl++;
if (fl > 1) ;//printf("Overlap (%d) in myio\n",fl);
if( poll(fds,1,0) == -1) { perror("Poll failed"); return; }
if (fds[0].revents & POLLIN){
	len = sizeof(struct sockaddr_ll);
	while ( 0 <= (size = recvfrom(unique_s,eth,MAXFRAME,0, (struct sockaddr *) &sll,&len))){
		if(size >1000) ;//printf("Packet %d-bytes received\n",size);  		
		if (eth->type == htons (0x0806)) {
			struct arp_packet * arp = (struct arp_packet *) eth->payload;
			if(htons(arp->op) == 2){ //It is ARP response
				for(i=0;(i<MAX_ARP) && (arpcache[i].key!=0);i++)
				if(!memcmp(&arpcache[i].key,arp->srcip,4)){
					memcpy(arpcache[i].mac,arp->srcmac,6); // Update
					break;
					}
					if(arpcache[i].key==0){
						;//printf("New ARP cache entry inserted\n");
						memcpy(arpcache[i].mac,arp->srcmac,6); //new insert
						memcpy(&arpcache[i].key,arp->srcip,4); // Update
					}
			}// It is ARP response
		} //it is ARP
		else if(eth->type == htons(0x0800)){
			struct ip_datagram * ip = (struct ip_datagram *) eth->payload;
			if (ip->proto == TCP_PROTO){
				struct tcp_segment * tcp = (struct tcp_segment *) ((char*)ip + (ip->ver_ihl&0x0F)*4);
				for(i=0;i<MAX_FD;i++)
					if((fdinfo[i].st == TCB_CREATED) && (fdinfo[i].l_port == tcp->d_port) 
							&& (tcp->s_port == fdinfo[i].tcb->r_port) && (ip->srcaddr == fdinfo[i].tcb->r_addr))
								break;
				if(i==MAX_FD)// if  not found connected TCB : second choice: listening socket
      			for(i=0;i<MAX_FD;i++) // Foreach TCB find listening sockets
        			if((fdinfo[i].st == TCB_CREATED) &&(fdinfo[i].tcb->st==LISTEN) && (tcp->d_port == fdinfo[i].l_port))
            		break;
			  if(i<MAX_FD)
						{
						struct tcpctrlblk * tcb = fdinfo[i].tcb;
						//printbuf((unsigned char*)ip,htons(ip->totlen));
						;//printf("(remote:%d) ---> (locahost:%d) socket=%d\n",htons(tcp->s_port),htons(tcp->d_port),i);

						;//printf("Received ack %d\n", htonl(tcp->ack)-tcb->seq_offs);
						fsm(i,PKT_RCV,ip);
						;//printf("status = %d\n",fdinfo[i].tcb->st);
						if(tcb->st < ESTABLISHED)break; 
						if(tcb->txfirst !=NULL){
							shifter = htonl(tcb->txfirst->segment->seq);
							;//printf("Processing ack  %d\n", htonl(tcp->ack)-tcb->seq_offs);
							if((htonl(tcp->ack)-shifter >= 0) && (htonl(tcp->ack)-shifter-1 <= htonl(tcb->txlast->segment->seq) + tcb->txlast->payloadlen-shifter)){ // -1 is to compensate the FIN	
								 //while((tcb->txfirst!=NULL) && ((htonl(tcp->ack)-tcb->seq_offs) >= (htonl(tcb->txfirst->segment->seq)-tcb->seq_offs+tcb->txfirst->payloadlen))){ //Ack>=Seq+payloadlen
								 if ((tcb->cong_st==CONG_AVOID) && (3<=(tcb->repeated_acks=(tcp->ack == tcb->last_ack)?tcb->repeated_acks+1:0)))  
												if(tcb->txfirst!= NULL){
													shifter = htonl(tcb->txfirst->segment->seq);
													struct txcontrolbuf * txcb;
													for(txcb=tcb->txfirst;txcb!=NULL && ((txcb->segment->seq-shifter) <=(htonl(tcp->ack)-shifter));txcb=txcb->next)
															txcb->retry=0; //Force the resend; Fast Retransmit
													tcb->threshold = tcb->cgwin/2;
													tcb->cgwin = tcb->threshold + 2*tcb->mss;
													tcb->cong_st=FAST_RECOV;
													}
											  printf("ACK: %d last ACK: %d\n",htonl(tcp->ack)-tcb->seq_offs, htonl(tcb->last_ack)-tcb->seq_offs);
										 										 switch( tcb->cong_st ){
												case SLOW_START : tcb->cgwin += (htonl(tcp->ack)-htonl(tcb->last_ack)); 
																		 			if(tcb->cgwin > tcb->threshold) tcb->cong_st = CONG_AVOID; 
																		 			break;
												case CONG_AVOID : tcb->cgwin += (htonl(tcb->last_ack)-htonl(tcp->ack))*(htonl(tcb->last_ack)-htonl(tcp->ack))/tcb->cgwin; 
																					break;
												case FAST_RECOV : if(tcb->last_ack==tcp->ack) tcb->cgwin += tcb->mss;
																					else { 
																						tcb->cgwin = tcb->threshold;
																						tcb->cong_st=CONG_AVOID;
																					}
																					break;
										 }
										 tcb->last_ack = tcp->ack; //in network order
							while((tcb->txfirst!=NULL) && ((htonl(tcp->ack)-shifter) >= (htonl(tcb->txfirst->segment->seq)-shifter + tcb->txfirst->payloadlen))){ //Ack>=Seq+payloadlen
									struct txcontrolbuf * temp = tcb->txfirst;
										if(htonl(tcp->ack)-shifter ==(htonl(temp->segment->seq)-shifter + temp->payloadlen)){ // Exact ACK matching: calculate roundtrip
											if(tcb->txfirst->retry==1){ // If not retransmitted
												int rtt = tick - tcb->txfirst->txtime;
												if (tcb->rtt_e == 0) tcb->rtt_e = rtt;
												tcb->Drtt_e = ((8-BETA)*tcb->Drtt_e + BETA*abs(rtt-tcb->rtt_e))>>3;
												tcb->rtt_e = ((8-ALPHA)*tcb->rtt_e + ALPHA*rtt)>>3;
												tcb->timeout = tcb->rtt_e + KAPPA*tcb->Drtt_e;
												}
											}
										 tcb->txfirst = tcb->txfirst->next;
										 printf("Removing seq %d\n",htonl(temp->segment->seq)-tcb->seq_offs);
										 fdinfo[i].tcb->txfree+=temp->payloadlen;
										 										 free(temp->segment);
										 free(temp);
										 if(tcb->txfirst	== NULL) tcb->txlast = NULL;
								}//While
							} 
						}
						unsigned int streamsegmentsize = htons(ip->totlen) - (ip->ver_ihl&0xF)*4 - (tcp->d_offs_res>>4)*4; 
						unsigned int stream_offs = ntohl(tcp->seq)-tcb->ack_offs;
						unsigned char * streamsegment = ((unsigned char*)tcp)+((tcp->d_offs_res>>4)*4);
						if((stream_offs + streamsegmentsize - tcb->rx_win_start)<RXBUFSIZE){
							tcb->adwin = RXBUFSIZE- (stream_offs + streamsegmentsize - tcb->rx_win_start);
							struct rxcontrol * rxsegment, *newrxsegment;
							for(rxsegment = tcb->unack; rxsegment!=NULL; rxsegment = rxsegment->next)
								if(rxsegment->next == NULL || rxsegment->next->stream_offs >= stream_offs) break;

							if(rxsegment != NULL && rxsegment->next!=NULL && rxsegment->next->stream_offs == stream_offs) break;

							for(int k=0; k<streamsegmentsize;k++)
								tcb->rxbuffer[(stream_offs+k)%RXBUFSIZE] = streamsegment[k];  

							newrxsegment = (struct rxcontrol *) malloc(sizeof(struct rxcontrol));
							newrxsegment->stream_offs = stream_offs;
							newrxsegment->streamsegmentsize = streamsegmentsize;
							if(tcp->flags&FIN) {
										tcb->stream_end=stream_offs + streamsegmentsize;
										newrxsegment->streamsegmentsize++;
										}
							if(rxsegment ==NULL) {
								tcb->unack = newrxsegment;
								newrxsegment->next = NULL;
							}
							else if( (stream_offs >= tcb->cumulativeack) && (tcb->unack->stream_offs > stream_offs) ){
								newrxsegment->next = tcb->unack;
								tcb->unack = newrxsegment;
							}
							else	
							 {
								newrxsegment->next = rxsegment->next;
								rxsegment->next = newrxsegment;
							}	
							while((tcb->unack != NULL) && (tcb->unack->stream_offs == tcb->cumulativeack)){
								struct rxcontrol * tmp;
								tmp = tcb->unack;
								tcb->cumulativeack += tcb->unack->streamsegmentsize;
								tcb->unack = tcb->unack->next;	
								free(tmp);
								}	
							if(tcb->txfirst==NULL) 
								prepare_tcp(i,ACK,NULL,0);	
					} 
				break;
				}// End of segment processing
		}//If TCP protocol
	}//IF ethernet
}//While packet
if (( errno != EAGAIN) && (errno!= EINTR )) { perror("Packet recvfrom Error\n"); }
}
fds[0].events= POLLIN|POLLOUT;
fds[0].revents=0;
if (fl > 1) ;//printf("Overlap (%d) in myio\n",fl);
	//printbuf(eth,size); 
fl--;
if(-1 == sigprocmask(SIG_UNBLOCK, &mymask, NULL)){perror("sigprocmask"); return ;}
}

int mylisten(int s, int bl){
if (fdinfo[s].st!=TCP_BOUND) {myerrno=EBADF; return -1;}
fdinfo[s].tcb = (struct tcpctrlblk *) malloc (sizeof(struct tcpctrlblk));
bzero(fdinfo[s].tcb,sizeof(struct tcpctrlblk));
fdinfo[s].st = TCB_CREATED;
fdinfo[s].tcb->st = LISTEN; /*Marking socket as passive opener*/
/*** Now we create the pending connection backlog ***********/
fdinfo[s].tcblist = (struct tcpctrlblk *) malloc (bl * sizeof(struct tcpctrlblk));
bzero(fdinfo[s].tcblist,bl* sizeof(struct tcpctrlblk));
fdinfo[s].bl = bl; //Backlog length size;
}

int myaccept(int s, struct sockaddr * addr, int * len)
{
int i,j;
if (addr->sa_family == AF_INET){
  struct sockaddr_in * a = (struct sockaddr_in *) addr;
  *len = sizeof(struct sockaddr_in);
  if (fdinfo[s].tcb->st!=LISTEN) {myerrno=EBADF; return -1;}
  if (fdinfo[s].tcblist == NULL) {myerrno=EBADF; return -1;}
  do{
      for(i=0;i<fdinfo[s].bl;i++){
        if(fdinfo[s].tcblist[i].st==ESTABLISHED){ //Not Fifo Queue
          for(j=3;j<MAX_FD && fdinfo[j].st!=FREE;j++); // Searching for free d
          if (j == MAX_FD) { myerrno=ENFILE; return -1;} //Not free descriptor
          else  { //Free File descriptor found
            fdinfo[j]=fdinfo[s];
						fdinfo[j].tcb=(struct tcpctrlblk *) malloc(sizeof(struct tcpctrlblk));
						memcpy(fdinfo[j].tcb,fdinfo[s].tcblist+i,sizeof(struct tcpctrlblk)); //Tcb of j is copied from this pending connection on backlog
            a->sin_port = fdinfo[j].tcb->r_port; //report on remote port
            a->sin_addr.s_addr = fdinfo[j].tcb->r_addr;//report on remote IP a
            fdinfo[j].bl=0; //twin socket has not backlog queue
            fdinfo[s].tcblist[i].st=FREE;
            prepare_tcp(j,ACK,NULL,0);
            return j; //New socket connect is returned
          }
        }//if pending connection
      }//for each fd
    } while(pause()); //Accept never ends
  }else { myerrno=EINVAL; return -1;}
}


int main(int argc, char **argv)
{
clock_t start;
fl = 0;
struct itimerval myt;
action_io.sa_handler = myio;
action_timer.sa_handler = mytimer;
sigaction(SIGIO, &action_io, NULL);
sigaction(SIGALRM, &action_timer, NULL);
unique_s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
if (unique_s == -1 ) { perror("Socket Failed"); return 1;}
if (-1 == fcntl(unique_s, F_SETOWN, getpid())){ perror("fcntl setown"); return 1;} 
fdfl = fcntl(unique_s, F_GETFL, NULL); if(fdfl == -1) { perror("fcntl f_getfl"); return 1;}
fdfl = fcntl(unique_s, F_SETFL,fdfl|O_ASYNC|O_NONBLOCK); if(fdfl == -1) { perror("fcntl f_setfl"); return 1;}
fds[0].fd = unique_s;
fds[0].events= POLLIN|POLLOUT;
fds[0].revents=0;
sll.sll_family = AF_PACKET;
sll.sll_ifindex = if_nametoindex("eth0");
myt.it_interval.tv_sec=0; /* Interval for periodic timer */
myt.it_interval.tv_usec=TIMER_USECS; /* Interval for periodic timer */
myt.it_value.tv_sec=0;    /* Time until next expiration */
myt.it_value.tv_usec=TIMER_USECS;    /* Time until next expiration */
if( -1 == sigemptyset(&mymask)) {perror("Sigemtpyset"); return 1;}
if( -1 == sigaddset(&mymask, SIGIO)){perror("Sigaddset");return 1;} 
if( -1 == sigaddset(&mymask, SIGALRM)){perror("Sigaddset");return 1;} 
if(-1 == sigprocmask(SIG_UNBLOCK, &mymask, NULL)){perror("sigprocmask"); return 1;}
if ( -1 == setitimer(ITIMER_REAL, &myt, NULL)){perror("Setitimer"); return 1;}
/*************USER'S MAIN CLIENT ****************
{
int w,t;
unsigned char * httpreq = "GET /\r\n";
unsigned char httpresp[500000];
int s;
struct sockaddr_in addr, loc_addr;
s=mysocket(AF_INET,SOCK_STREAM,0);
addr.sin_family = AF_INET;
addr.sin_port =htons(80);
//addr.sin_addr.s_addr = inet_addr("23.215.60.27");
addr.sin_addr.s_addr = inet_addr("172.217.169.4");
//addr.sin_addr.s_addr = inet_addr("127.0.0.1");
loc_addr.sin_family = AF_INET;
loc_addr.sin_port =((argc==2)?htons(atoi(argv[1])):htons(0));
;//printf("Local port = %d\n",htons(loc_addr.sin_port));
loc_addr.sin_addr.s_addr = htonl(0);
if( -1 == mybind(s,(struct sockaddr *) &loc_addr, sizeof(struct sockaddr_in))){myperror("mybind"); return 1;}
if (-1 == myconnect(s,(struct sockaddr * )&addr,sizeof(struct sockaddr_in))){myperror("myconnect"); return 1;}
if ( mywrite(s,httpreq,strlen(httpreq))==1) { myperror("Mywrite Failed\n"); return -1;}
for (w=0; t=myread(s,httpresp+w,500000-w);w+=t)
	if(t== -1){myperror("myread"); return 1;}
	
httpresp[w]=0;
;//printf("%s\n\n",httpresp);

if (-1 == myclose(s)){myperror("myclose"); return 1;}

;//printf("I closed =================\n");
}

********************END OF USER'S MAIN **************/
/********** USER'S WEB SERVER CODE****************/
{
struct sockaddr_in addr,remote_addr;
int i,j,k,s,t,s2,len;
int c;
FILE * fin;
char * method, *path, *ver;
char request[5000],response[10000];
s =  mysocket(AF_INET, SOCK_STREAM, 0);
if ( s == -1 ){ perror("Socket fallita"); return 1; }
addr.sin_family = AF_INET;
addr.sin_port =((argc==2)?htons(atoi(argv[1])):htons(0));
addr.sin_addr.s_addr = 0;
if ( mybind(s,(struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {perror("bind fallita"); return 1;}
if ( mylisten(s,5) == -1 ) { myperror("Listen Fallita"); return 1; }
len = sizeof(struct sockaddr_in);
while(1){
remote_addr.sin_family=AF_INET;
s2 =  myaccept(s, (struct sockaddr *)&remote_addr,&len);
if ( s2 == -1 ) { myperror("Accept Fallita"); return 1;}
t = myread(s2,request,4999);
if ( t == -1 ) { myperror("Read fallita"); return 1;}
request[t]=0;
;//printf("%s",request);
method = request;
for(i=0;request[i]!=' ';i++){} request[i]=0; path = request+i+1;
for(i++;request[i]!=' ';i++); request[i]=0; ver = request+i+1;
for(i++;request[i]!='\r';i++); request[i]=0;
;//printf("method=%s path=%s ver=%s\n",method,path,ver);
if ((fin = fopen(path+1,"rt"))==NULL){
  sprintf(response,"HTTP/1.1 404 Not Found\r\n\r\n");
  mywrite(s2,response,strlen(response));
  }
else {
      sprintf(response,"HTTP/1.1 200 OK\r\n\r\n");
      mywrite(s2,response,strlen(response));
			
      while (t=fread(response,1,TCP_MSS,fin))
        for(i=0;i<t;i=i+k)
					k=mywrite(s2,response+i,t-i);
      fclose(fin);
    }
      myclose(s2);
}
 } //end while
/******************* END OF SERVER CODE**************/
           
start= tick;
while (pause()){
if(((tick-start)*TIMER_USECS)>1000000) break;
	}
return 0;
}


