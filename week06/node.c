#include "node.h"

#define SERVER_PORT     1337 // Port for the server
#define SERVER_IP_ADDRESS "192.168.1.52"
#define MY_IP_ADDRESS "192.168.1.67"
#define TRUE 1
#define FALSE 0
#define PING_INTERVAL 3
struct PeerNode this_node;
int current_connect;

struct greet_client_data {
    int client_socket, number;
    struct sockaddr_in client_addr;
};

int member(Peer element) {
    for (int i = 0; i < CONNECT_N; i++) {
        if ((strcmp(this_node.peer_list[i].ip_address, element.ip_address) == 0) &&
            element.port == this_node.peer_list[i].port) {
            return TRUE;
        }
    }
    return FALSE;
}

void *initialise_server(void *data) {

    //Create socket and server addresses for binding
    int server_socket, clients_fd;
    pthread_t clients[CONNECT_N], pinger;
    socklen_t addrlen;
    struct sockaddr_in server_addr;

    addrlen = sizeof(struct sockaddr_in);
    current_connect = 0;

    //Create server socket that is datagram for tcp transmissions
    if ((server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        fprintf(stderr, "failed to create a server socket errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr)); //Clean up server address
    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(MY_IP_ADDRESS);

    //Bind server socket to server
    if ((bind(server_socket, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1)) {
        fprintf(stderr, "failed to bind server socket errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    // Copy node information about us to the self part
    memset(this_node.self.ip_address, 0, sizeof(this_node.self.ip_address));
    strcpy(this_node.self.ip_address, MY_IP_ADDRESS);
    this_node.self.port = SERVER_PORT;

    //Begin listening
    if (listen(server_socket, CONNECT_N) < 0) {
        fprintf(stderr, "failed to listen server errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    pthread_create(&pinger, NULL, ping_clients, NULL);
    while (TRUE) {
        if (current_connect < CONNECT_N) {
            struct greet_client_data c_data;
            struct sockaddr_in client_addr;
            clients_fd = accept(server_socket,
                                (struct sockaddr *) &client_addr,
                                &addrlen);
            c_data.client_socket = clients_fd;
            c_data.client_addr = client_addr;
            c_data.number = current_connect;
            pthread_create(&clients[current_connect], NULL, handle_client, (void *) &c_data);
        }
    }
}

void *ping_clients(void *data) {
    Peer null;
    memset(&null, 0, sizeof(null));
    while (TRUE) {
        sleep(PING_INTERVAL);
        struct Protocol p;
        ssize_t bytes_received, bytes_sent;
        p.type = PING;
        int connect_fd;
        struct sockaddr_in server_addr;
        socklen_t addr_len;
        addr_len = sizeof(server_addr);

        for (int i = 0; i < CONNECT_N; ++i) {
            p.type = PING;
            if (memcmp(&this_node.peer_list[i], &null, sizeof(this_node.peer_list[i])) != 0) {
                printf("pinging.. \n");
                if ((connect_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
                    fprintf(stderr, "failed to create a socket to ping clients errno: %d\n", errno);
                    exit(EXIT_FAILURE);
                }
                server_addr.sin_family = AF_INET;
                server_addr.sin_port = htons(this_node.peer_list[i].port);
                server_addr.sin_addr.s_addr = inet_addr(this_node.peer_list[i].ip_address);

                if (connect(connect_fd, (struct sockaddr *) &server_addr, addr_len) == -1) {
                    if (errno == ECONNREFUSED) {
                        printf("Node Name:%s:%s:%u left\n", this_node.peer_list[i].name,
                               this_node.peer_list[i].ip_address,
                               this_node.peer_list[i].port);
                        memset(&this_node.peer_list[i], 0, sizeof(this_node.peer_list[i]));
                        continue;
                    } else {
                        fprintf(stderr, "failed to connect to ping errno:%d\n", errno);
                        exit(EXIT_FAILURE);
                    }
                }

                //Send protocol type
                bytes_sent = sendto(connect_fd, (void *) &p, sizeof(p), 0,
                                    (struct sockaddr *) &server_addr,
                                    sizeof(struct sockaddr));
                if (bytes_sent == -1) {
                    fprintf(stderr, "error on send ping errno: %d\n", errno);
                    exit(EXIT_FAILURE);
                }

                //Receive answer
                bytes_received = recvfrom(connect_fd, (void *) &p, sizeof(p), 0,
                                          (struct sockaddr *) &server_addr,
                                          &addr_len);
                if (bytes_received == -1) {
                    if (errno == ETIMEDOUT) {
                        printf("Node Name:%s:%s:%u left\n", this_node.peer_list[i].name,
                               this_node.peer_list[i].ip_address,
                               this_node.peer_list[i].port);
                        memset(&this_node.peer_list[i], 0, sizeof(this_node.peer_list[i]));
                        continue;
                    } else {
                        fprintf(stderr, "error on receive ping errno: %d\n", errno);
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
}

void *handle_client(void *data) {
    struct greet_client_data *client_data = (struct greet_client_data *) data;
    socklen_t addr_len;
    ssize_t received_bytes;
    struct PeerNode new_node;
    struct Protocol p;

    //Receive protocol data from client
    received_bytes = recvfrom(client_data->client_socket, (void *) &p, sizeof(p), 0,
                              (struct sockaddr *) &client_data->client_addr, &addr_len);
    if (received_bytes == -1) {
        fprintf(stderr, "Error on recv protocol errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    //Check protocol type, do appropriate things according to it
    if (p.type == PING) {
        p.type = ACK;
        sendto(client_data->client_socket, (void *) &p, sizeof(p), 0, (struct sockaddr *) &client_data->client_addr,
               sizeof(struct sockaddr));
        printf("Got pinged, answering\n");
    } else if (p.type == ADD) {
        //Receive data about self from new client
        received_bytes = recvfrom(client_data->client_socket, (void *) &new_node, sizeof(new_node), 0,
                                  (struct sockaddr *) &client_data->client_addr, &addr_len);

        if (received_bytes == -1) {
            fprintf(stderr, "Error on recv self info about client errno: %d\n", errno);
            exit(EXIT_FAILURE);
        }


        if (!member(new_node.self)) {
            printf("Got new node! Name:%s:%s:%u\n", new_node.self.name, new_node.self.ip_address,
                   new_node.self.port);
            //Copy name and address of the client
            strcpy(this_node.peer_list[client_data->number].name, new_node.self.name);
            strcpy(this_node.peer_list[client_data->number].ip_address, new_node.self.ip_address);
            this_node.peer_list[client_data->number].port = new_node.self.port;
            current_connect++;
        }
        //Send known nodes to the new client
        sendto(client_data->client_socket, this_node.peer_list, sizeof(this_node.peer_list), 0,
               (struct sockaddr *) &client_data->client_addr, sizeof(struct sockaddr));
    }
    close(client_data->client_socket);
    return 0;
}

void *initialise_client(void *data) {
    //Set up variables for client socket its address and destination address
    Peer node_list[CONNECT_N];
    ssize_t bytes_received, bytes_sent;
    struct sockaddr_in destination_addr = *(struct sockaddr_in *) data;
    int client_socket;
    struct Protocol p;

    // Create client's socket from which he will connect
    socklen_t addr_len = sizeof(struct sockaddr);
    if ((client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        fprintf(stderr, "failed to create a client socket errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    // Set up destination address (address of the server)
    current_connect++;
    strcpy(this_node.peer_list[current_connect - 1].ip_address, inet_ntoa(destination_addr.sin_addr));
    this_node.peer_list[current_connect - 1].port = ntohs(destination_addr.sin_port);

    //Connect to the server
    if (connect(client_socket, (struct sockaddr *) &destination_addr, addr_len) == -1) {
        fprintf(stderr, "failed to connect to server by client errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }
    p.type = ADD;
    //Send protocol type
    bytes_sent = sendto(client_socket, (void *) &p, sizeof(p), 0,
                        (struct sockaddr *) &destination_addr,
                        sizeof(struct sockaddr));
    if (bytes_sent == -1) {
        fprintf(stderr, "error on send protocol on client errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    //Send data about self
    bytes_sent = sendto(client_socket, (void *) &this_node.self, sizeof(this_node), 0,
                        (struct sockaddr *) &destination_addr,
                        sizeof(struct sockaddr));
    if (bytes_sent == -1) {
        fprintf(stderr, "error on send self info errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    //Receive data about other nodes
    bytes_received = recvfrom(client_socket, (void *) &node_list, sizeof(this_node), 0,
                              (struct sockaddr *) &destination_addr,
                              &addr_len);
    if (bytes_received == -1) {
        fprintf(stderr, "error on receive info errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    close(client_socket);
    for (int j = current_connect; j < CONNECT_N; ++j) {
        Peer new_node = node_list[j];
        if (!member(new_node) && new_node.port != this_node.self.port &&
            strcmp(new_node.ip_address, this_node.self.ip_address) != 0) {
            printf("Got new node! Name:%s:%s:%u\n", new_node.name, new_node.ip_address,
                   new_node.port);
            if ((client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
                fprintf(stderr, "failed to create a socket to connect to new nodes errno: %d\n", errno);
                exit(EXIT_FAILURE);
            }
            struct sockaddr_in dest;
            dest.sin_family = AF_INET;
            dest.sin_port = htons(new_node.port);
            dest.sin_addr.s_addr = inet_addr(new_node.ip_address);
            if (connect(client_socket, (struct sockaddr *) &dest, sizeof(struct sockaddr)) == -1) {
                fprintf(stderr, "failed to connect to new node errno:%d\n", errno);
                exit(EXIT_FAILURE);
            }
        }
    }
    return 0;
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
        struct sockaddr_in dest;
        char ip[20];
        uint16_t port;
        char buf[2];
        printf("What do you want to do?\n");
        printf("To connect - 1. (Server works on background)\n");
        read(0, buf, sizeof(buf));
        buf[1] = '\0';
        if (strcmp(buf, "1") == 0) {
            printf("Enter IP:Port of the server\n");
            dest.sin_family = AF_INET;
            scanf("%s:%hu\n", ip, &port);
            dest.sin_port = htons(port);
            dest.sin_addr.s_addr = inet_addr(ip);
            pthread_create(&client, NULL, initialise_client, (void *) &dest);
        }
        sleep(1);
    }
}