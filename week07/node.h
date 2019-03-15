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
#define PROT_OK 100
#define PROT_NO_200
#define PROT_PING 55
#define PROT_ACK 56
#define PROT_ADD_PEER 50
#define PROT_SYNC_PEERS 60
#define PROT_SYNC_FILES 61
#define SERVER_PORT     1333 // Port for the server
#define MY_IP_ADDRESS "192.168.1.67"
#define TRUE 1
#define FALSE 0
#define PING_INTERVAL 3
#define PEER_BUF 10


typedef struct Peer {
    char name[25];
    char ip_address[20];
    uint16_t port;
} Peer;

struct LinkedNode {
    struct LinkedNode *next, *previous;
    struct Peer value;
};
struct LinkedList {
    int length;
    struct LinkedNode *self;
};

struct PeerNode {
    Peer self;
    struct LinkedList peers;
};

struct Protocol {
    short type;
};

struct greet_client_data {
    int client_socket;
    struct sockaddr_in client_addr;
};


void *initialise_client(void *);

void *initialise_server(void *);

void *handle_client(void *);

void *ping_clients(void *);

void connect_to_peer(struct Peer);

struct PeerNode this_node;
