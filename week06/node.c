#include "node.h"

#define SERVER_PORT     1337 // Port for the server
#define CONNECT_N       10 //Number of connections at the same time

int initialise_server(void) {

    //Create socket and server addresses for binding
    int server_socket;
    struct sockaddr_in server_addr;
    //Create server socket that is datagram for udp transmissions
    if ((server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        fprintf(stderr, "failed to create a socket errno: %d", errno);
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr)); //Clean up server address
    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //Bind server socket to server address we set up earlier
    if ((bind(server_socket, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1)) {
        fprintf(stderr, "failed to bind errno: %d", errno);
        return -1;
    }

    //Begin listening
    if (listen(server_socket, CONNECT_N) < 0) {
        fprintf(stderr, "failed to listen errno: %d", errno);
        return -1;
    }

    return -1;
}

int initialise_client(void) {
    //Set up variables for client socket its address and destination address
    int client_socket;
    ssize_t bytes_received, bytes_sent;
    struct sockaddr_in client_addr, destination_addr;
    socklen_t addr_len = 0;

    //Create client socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        fprintf(stderr, "failed to create a socket errno: %d", errno);
        return -1;
    }

    //Set up client address so it can receive udp transmission data
    memset(&client_addr, 0, sizeof(client_addr)); //Clean up server address
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(SERVER_PORT + 1);
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //Bind client to the address
    addr_len = sizeof(struct sockaddr);
    if ((bind(client_socket, (struct sockaddr *) &client_addr, sizeof(struct sockaddr)) == -1)) {
        fprintf(stderr, "failed to bind errno: %d", errno);
        return -1;
    }

    // Set up destination address (address of the server)
    destination_addr.sin_family = AF_INET;
    destination_addr.sin_port = htons(SERVER_PORT);
    struct hostent *host = gethostbyname(SERVER_IP_ADDRESS);
    destination_addr.sin_addr = *((struct in_addr *) host->h_addr);


    return -1;
}

int main(void) {
    initialise_server();
}