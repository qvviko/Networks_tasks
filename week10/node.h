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
#include <ctype.h>

#define CONNECT_N       10
#define PROT_REQUEST 0
#define PROT_SYN 1
#define SERVER_PORT     1337
#define MY_IP_ADDRESS "10.91.46.102"
#define TRUE 1
#define FALSE 0
#define PING_INTERVAL 5
#define DEBUG FALSE
#define IS_UNIVERSAL TRUE

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define BUF_SIZE 1024
typedef struct Peer {
    char name[BUF_SIZE];
    char ip_address[20];
    uint16_t port;
} Peer;
struct LinkedPeerNode {
    struct LinkedPeerNode *next, *previous;
    struct Peer value;
};
struct LinkedPeerList {
    int length;
    struct LinkedPeerNode *self;
};


struct PeerFile {
    char name[BUF_SIZE];
    Peer owner;
    int is_loaded;
};
struct LinkedFileNode {
    struct LinkedFileNode *next, *previous;
    struct PeerFile value;
};
struct LinkedFileList {
    int length;
    struct LinkedFileNode *self;
};

struct greet_client_data {
    int client_socket;
    struct sockaddr_in client_addr;
};
struct PeerNode {
    Peer self;
    struct LinkedPeerList peers;
    struct LinkedFileList files;
};
struct PeerNode this_node;
struct LinkedPeerList black_list;
struct LinkedPeerList current_list;
pthread_mutex_t bldb_lock;
pthread_mutex_t cdb_lock;

void *initialise_server(void *);

void *handle_client(void *);

void *ping_clients(void *);

void add_peer_to_a_list(struct Peer peer);

