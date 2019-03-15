#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "hashmap.h"

#define CONNECT_N       10 //Number of connections at the same time
#define PROT_PING 55
#define PROT_ACK 56
#define PROT_ADD_PEER 50
#define SERVER_PORT     1337 // Port for the server
#define MY_IP_ADDRESS "192.168.1.67"
#define TRUE 1
#define FALSE 0
#define KEY_SIZE 30
#define PING_INTERVAL 3

typedef struct Peer {
    char name[25];
    char ip_address[20];
    uint16_t port;
} Peer;

struct greet_client_data {
    int client_socket, number;
    struct sockaddr_in client_addr;
};

struct PeerNode {
    Peer self;
    struct HashMap *PeerList;
    struct HashMap *FileList;
};

struct Protocol {
    short type;
};

void *initialise_client(void *);

void *initialise_server(void *);

void *handle_client(void *);

void *ping_clients(void *);

struct PeerNode this_node;
