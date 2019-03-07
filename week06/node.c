#include "node.h"

#define SERVER_PORT     1337 // Port for the server
#define SERVER_IP_ADDRESS "192.168.1.58"
#define TRUE 1

Node this_node;
int current_connect;
struct greet_client_data {
    int client_socket;
    struct sockaddr_in client_addr;

};

void *initialise_server(void *data) {

    //Create socket and server addresses for binding
    int server_socket, client_fd[CONNECT_N];
    pthread_t clients[CONNECT_N];
    socklen_t addrlen;
    struct sockaddr_in server_addr;

    addrlen = sizeof(struct sockaddr_in);
    current_connect = 0;
    //Create server socket that is datagram for tcp transmissions
    if ((server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        fprintf(stderr, "failed to create a socket errno: %d", errno);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr)); //Clean up server address
    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //Bind server socket to server
    if ((bind(server_socket, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1)) {
        fprintf(stderr, "failed to bind errno: %d", errno);
        exit(EXIT_FAILURE);
    }
    this_node.self.addr = server_addr;

    //Begin listening
    if (listen(server_socket, CONNECT_N) < 0) {
        fprintf(stderr, "failed to listen errno: %d", errno);
        exit(EXIT_FAILURE);
    }
    while (TRUE) {
        printf("Server \n");
        sleep(1);
        if (current_connect < CONNECT_N) {
            struct greet_client_data c_data;
            Peer cur_node = this_node.peer_list[current_connect];
            client_fd[current_connect] = accept(server_socket, (struct sockaddr *) &cur_node.addr, &addrlen);
            c_data.client_socket = client_fd[current_connect];
            c_data.client_addr = cur_node.addr;
            pthread_create(&clients[current_connect], NULL, greet_client, (void *) &c_data);
            current_connect++;
        }
    }
}

void *greet_client(void *data) {
    struct greet_client_data *client_data = (struct greet_client_data *) data;
    socklen_t addr_len;
    ssize_t recieved_bytes;
    Node new_node;
    recieved_bytes = recvfrom(client_data->client_socket, (void *) &new_node, sizeof(new_node), 0,
                              (struct sockaddr *) &client_data->client_addr, &addr_len);
    printf("%s\n", new_node.self.name);
}

void *initialise_client(void *data) {
    //Set up variables for client socket its address and destination address
    int client_socket;
    ssize_t bytes_received, bytes_sent;
    struct sockaddr_in destination_addr;
    socklen_t addr_len = sizeof(struct sockaddr);

    //Create client socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        fprintf(stderr, "failed to create a socket errno: %d", errno);
        exit(EXIT_FAILURE);
    }

    // Set up destination address (address of the server)
    destination_addr.sin_family = AF_INET;
    destination_addr.sin_port = htons(SERVER_PORT);
    struct hostent *host = gethostbyname(SERVER_IP_ADDRESS);
    destination_addr.sin_addr = *((struct in_addr *) host->h_addr);

    //Connect to the server
    if (connect(client_socket, (struct sockaddr *) &destination_addr, addr_len) == -1) {
        fprintf(stderr, "failed to connect errno:%d", errno);
        exit(EXIT_FAILURE);
    }

    //Send data

    bytes_received = sendto(client_socket, (void *) &this_node, sizeof(this_node), 0,
                            (struct sockaddr *) &destination_addr,
                            sizeof(struct sockaddr));
    if (bytes_received == -1) {
        fprintf(stderr, "error on send errno: %d", errno);
        exit(EXIT_FAILURE);
    }
}


int main(void) {
    pthread_t client, server;
    memset(&this_node, 0, sizeof(this_node));
    printf("How should i call you?\n");
    read(0, this_node.self.name, sizeof(this_node.self.name));
    printf("Ok! Your name is:%s\n", this_node.self.name);
    pthread_create(&server, NULL, initialise_server, NULL);
    while (TRUE) {
        printf("What do you want to do?\n");
        printf("To connect - 1, To wait - 2\n");
        char buf[2];
        read(0, buf, sizeof(char));
        buf[1] = '\0';
        if (strcmp(buf, "1") == 0) {
            pthread_create(&client, NULL, initialise_client, NULL);
        }
        sleep(1);
    }
}