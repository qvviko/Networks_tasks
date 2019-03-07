#include "node.h"

#define SERVER_PORT     1337 // Port for the server
#define SERVER_IP_ADDRESS "localhost"
#define TRUE 1
#define FALSE 0

Node this_node;
int current_connect;
struct greet_client_data {
    int client_socket, number;
    struct sockaddr_in client_addr;
};

int member(Peer element) {
    for (int i = 0; i < CONNECT_N; i++) {
        if (strcmp((char *) &this_node.peer_list[i], (char *) &element) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

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
            client_fd[current_connect] = accept(server_socket,
                                                (struct sockaddr *) &this_node.peer_list[current_connect].addr,
                                                &addrlen);
            c_data.client_socket = client_fd[current_connect];
            c_data.client_addr = this_node.peer_list[current_connect].addr;
            c_data.number = current_connect;
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
    //Receive data about self from new client
    recieved_bytes = recvfrom(client_data->client_socket, (void *) &new_node, sizeof(new_node), 0,
                              (struct sockaddr *) &client_data->client_addr, &addr_len);
    if (recieved_bytes == -1) {
        fprintf(stderr, "Error on recv errno: %d", errno);
        exit(EXIT_FAILURE);
    }

    printf("Got new node! Name:%s:%s:%u\n", new_node.self.name, inet_ntoa(new_node.self.addr.sin_addr),
           htons(new_node.self.addr.sin_port));
    //Copy name and address of the client
    memcpy(this_node.peer_list[client_data->number].name, new_node.self.name, sizeof(new_node.self.name));
    this_node.peer_list[client_data->number].addr = new_node.self.addr;

    //Send known nodes to the new client
    sendto(client_data->client_socket, this_node.peer_list, sizeof(this_node.peer_list), 0,
           (struct sockaddr *) &client_data->client_addr, sizeof(struct sockaddr));
    return 0;
}

void *initialise_client(void *data) {
    //Set up variables for client socket its address and destination address
    int client_socket;
    Peer node_list[CONNECT_N];
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
        fprintf(stderr, "failed to number errno:%d", errno);
        exit(EXIT_FAILURE);
    }

    //Send data about self
    bytes_sent = sendto(client_socket, (void *) &this_node, sizeof(this_node), 0,
                        (struct sockaddr *) &destination_addr,
                        sizeof(struct sockaddr));
    if (bytes_sent == -1) {
        fprintf(stderr, "error on send errno: %d", errno);
        exit(EXIT_FAILURE);
    }

    //Receive data about other nodes
    bytes_received = recvfrom(client_socket, (void *) &node_list, sizeof(this_node), 0,
                              (struct sockaddr *) &destination_addr,
                              &addr_len);

    //Connect to all new nodes
    for (int i = 0; i < CONNECT_N; ++i) {
        Peer new_node = node_list[i];
        if (!member(new_node)) {
            if (connect(client_socket, (struct sockaddr *) &new_node.addr, addr_len) == -1) {
                fprintf(stderr, "failed to number errno:%d", errno);
                exit(EXIT_FAILURE);
            }
        }
    }
    if (bytes_received == -1) {
        fprintf(stderr, "error on receive errno: %d", errno);
        exit(EXIT_FAILURE);
    }
}


int main(void) {
    pthread_t client, server;
    ssize_t bytes_read;
    memset(&this_node, 0, sizeof(this_node));
    printf("How should i call you?\n");
    bytes_read = read(0, this_node.self.name, sizeof(this_node.self.name) - 1);
    this_node.self.name[bytes_read - 1] = '\0';
    printf("Ok! Your name is:%s\n", this_node.self.name);
    pthread_create(&server, NULL, initialise_server, NULL);
    while (TRUE) {
        printf("What do you want to do?\n");
        printf("To number - 1, To wait - 2\n");
        char buf[2];
        read(0, buf, sizeof(buf));
        buf[1] = '\0';
        if (strcmp(buf, "1") == 0) {
            pthread_create(&client, NULL, initialise_client, NULL);
        }
        sleep(1);
    }
}