#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

int main(){
    // setup a socket
    int network_socket;
    // create the socket endpoint
    network_socket = socket(AF_INET, SOCK_STREAM, 0); // AF_INET: IPv4, SOCK_STREAM: TCP 0: default protocol

    // specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // AF_INET: IPv4
    server_address.sin_port = htons(80); // htons: host to network short 
    unsigned char *ip = (unsigned char*)&server_address.sin_addr.s_addr; 

    // Per questo esempio prendiamo l'indirizzo di www.google.com (vedi sotto come)
    ip[0]=142; ip[1]=250;ip[2]=200;ip[3]=36;

    int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address)); // connect to the server

    // check for error with the connection
    if(connection_status == -1){  // Verifica se il collegamento è ancora valido
        perror("Connect Fallita\n"); // Manda un messaggio di errore
        return 1; // Termina il programma con codice di uscita 1 (errore)
    } 

    // receive data from the server
    char server_response[256];
    recv(network_socket, &server_response, sizeof(server_response), 0); // receive data from the socket

    // print out the server's response
    printf("The server sent the data: %s\n", server_response);

    // close the socket
    close(network_socket);

    return 0;
}