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

#define CONNECT_N       10 //Number of connections at the same time

typedef struct Peer {
    char name[25];
    char ip_address[20];
    unsigned int port;
} Peer;

typedef struct PeerNode {
    Peer self;
    Peer peer_list[CONNECT_N];
} Node;

void *initialise_client(void *);

void *initialise_server(void *);

void *greet_client(void *);