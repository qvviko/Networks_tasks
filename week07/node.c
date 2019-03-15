#include "node.h"

char *create_key(char ip[20], uint16_t port) {
    char *to_return = (char *) malloc(sizeof(char) * KEY_SIZE);
    char small_buf[16];
    memset(small_buf, 0, sizeof(small_buf));
    memset(to_return, 0, sizeof(char) * KEY_SIZE);
    strcpy(to_return, ip);
    sprintf(small_buf, "%hu", port);
    strcpy(to_return + strlen(to_return), small_buf);
    return to_return;
}

void *initialise_server(void *data) {

    //Create socket and server addresses for binding
    int server_socket, clients_fd, current_connect = 0;
    pthread_t clients[CONNECT_N], pinger;
    socklen_t addrlen;
    struct sockaddr_in server_addr;

    addrlen = sizeof(struct sockaddr_in);
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
        struct greet_client_data c_data;
        struct sockaddr_in client_addr;
        clients_fd = accept(server_socket,
                            (struct sockaddr *) &client_addr,
                            &addrlen);
        c_data.client_socket = clients_fd;
        c_data.client_addr = client_addr;
        c_data.number = current_connect;
        pthread_create(&clients[current_connect], NULL, handle_client, (void *) &c_data);
        current_connect = (current_connect + 1) % CONNECT_N;
    }
}

void *ping_clients(void *data) {
// TODO: FIX THIS
    struct Peer *peers = malloc(sizeof(char) * 0);
    struct Protocol p;
    ssize_t bytes_received, bytes_sent;
    p.type = PROT_PING;
    int connect_fd, peer_num;
    struct sockaddr_in server_addr;
    socklen_t addr_len;
    addr_len = sizeof(server_addr);

    while (TRUE) {
        sleep(PING_INTERVAL);

        peer_num = this_node.PeerList->length;
        peers = (Peer *) realloc(peers, sizeof(Peer) * peer_num);
        get_all(this_node.PeerList, peers);

        for (int i = 0; i < peer_num; ++i) {
            p.type = PROT_PING;
            if ((connect_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
                fprintf(stderr, "failed to create a socket to ping clients errno: %d\n", errno);
                exit(EXIT_FAILURE);
            }
            printf("about to make addr in ping \n");
            printf("Node Name:%s:%s:%u is being pinged\n", peers[i].name,
                   peers[i].ip_address, peers[i].port);
            fflush(stdout);
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(peers[i].port);
            server_addr.sin_addr.s_addr = inet_addr(peers[i].ip_address);

            if (connect(connect_fd, (struct sockaddr *) &server_addr, addr_len) == -1) {
                if (errno == ECONNREFUSED) {
                    printf("Node Name:%s:%s:%u left\n", peers[i].name,
                           peers[i].ip_address, peers[i].port);
                    //Remove item from the hashmap)
                    void *key = create_key(peers[i].ip_address, peers[i].port);
                    void *buf = remove_item(this_node.PeerList, key);
                    free(key);
                    free(buf);

                    close(connect_fd);
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
                    printf("Node Name:%s:%s:%u left\n", peers[i].name,
                           peers[i].ip_address, peers[i].port);
                    //Remove item from the hashmap)
                    void *key = create_key(peers[i].ip_address, peers[i].port);
                    void *buf = remove_item(this_node.PeerList, key);
                    free(key);
                    free(buf);

                    close(connect_fd);
                    continue;
                } else {
                    fprintf(stderr, "error on receive ping errno: %d\n", errno);
                    exit(EXIT_FAILURE);
                }
            }
            close(connect_fd);
        }
    }
}

void *handle_client(void *data) {
    struct greet_client_data *client_data = (struct greet_client_data *) data;
    socklen_t addr_len;
    ssize_t received_bytes, bytes_sent;
    struct Peer new_node;
    struct Protocol p;
    char *key;
    void *member;

    //Receive protocol data from client
    received_bytes = recvfrom(client_data->client_socket, (void *) &p, sizeof(p), 0,
                              (struct sockaddr *) &client_data->client_addr, &addr_len);
    if (received_bytes == -1) {
        fprintf(stderr, "Error on recv protocol errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    //Check protocol type, do appropriate things according to it
    if (p.type == PROT_PING) {
        p.type = PROT_ACK;
        bytes_sent = sendto(client_data->client_socket, (void *) &p, sizeof(p), 0,
                            (struct sockaddr *) &client_data->client_addr,
                            sizeof(struct sockaddr));
        if (bytes_sent == -1) {
            fprintf(stderr, "Error on sending ack errno : %d \n", errno);
            exit(EXIT_FAILURE);
        }
        printf("Got pinged, answering\n");
    } else if (p.type == PROT_ADD_PEER) {
        //Receive data about self from new client
        received_bytes = recvfrom(client_data->client_socket, (void *) &new_node, sizeof(new_node), 0,
                                  (struct sockaddr *) &client_data->client_addr, &addr_len);

        if (received_bytes == -1) {
            fprintf(stderr, "Error on recv self info about client errno: %d\n", errno);
            exit(EXIT_FAILURE);
        }

        key = create_key(new_node.ip_address, new_node.port);
        member = find(this_node.PeerList, key);
        if (member == NULL) {
            printf("Got new node! Name:%s:%s:%u\n", new_node.name, new_node.ip_address,
                   new_node.port);
            //Add item
            add_item(this_node.PeerList, key, (void *) &new_node);
            Peer *h = find(this_node.PeerList, key);
            printf("Node Name:%s:%s:%u is being added\n", h->name, h->ip_address, h->port);
        }
        free(member);
        free(key);
        //Send known nodes to the new client
        //TODO: FIX this
//        bytes_sent = sendto(client_data->client_socket, this_node.peer_list, sizeof(this_node.peer_list), 0,
//                            (struct sockaddr *) &client_data->client_addr, sizeof(struct sockaddr));
//        if (bytes_sent == -1) {
//            fprintf(stderr, "Error on sending ack errno : %d \n", errno);
//            exit(EXIT_FAILURE);
//        }
    }
    close(client_data->client_socket);
    return 0;
}

void *initialise_client(void *data) {
    //Set up variables for client socket its address and destination address
    char *key;
    Peer new_peer;
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

    // Don't add if already in list
    key = create_key(inet_ntoa(destination_addr.sin_addr), ntohs(destination_addr.sin_port));
    void *member = find(this_node.PeerList, key);
    if (member == NULL) {
        // Add address to list (address of the server)
        strcpy(new_peer.ip_address, inet_ntoa(destination_addr.sin_addr));
        new_peer.port = ntohs(destination_addr.sin_port);
        add_item(this_node.PeerList, key, (void *) &new_peer);
        printf("Got new node! Name:%s:%u\n", new_peer.ip_address,
               new_peer.port);
    }
    free(member);
    free(key);

    //Connect to the server
    if (connect(client_socket, (struct sockaddr *) &destination_addr, addr_len) == -1) {
        fprintf(stderr, "failed to connect to server by client errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }
    p.type = PROT_ADD_PEER;
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
    // TODO:Fix this
//    //Receive data about other nodes
//    bytes_received = recvfrom(client_socket, (void *) &node_list, sizeof(this_node), 0,
//                              (struct sockaddr *) &destination_addr,
//                              &addr_len);
//    if (bytes_received == -1) {
//        fprintf(stderr, "error on receive info errno: %d\n", errno);
//        exit(EXIT_FAILURE);
//    }
//    // Close connection
//    close(client_socket);
//    for (int j = current_connect; j < CONNECT_N; ++j) {
//        Peer new_node = node_list[j];
//        if (!member(new_node) && new_node.port != this_node.self.port &&
//            strcmp(new_node.ip_address, this_node.self.ip_address) != 0) {
//            printf("Got new node! Name:%s:%s:%u\n", new_node.name, new_node.ip_address,
//                   new_node.port);
//            if ((client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
//                fprintf(stderr, "failed to create a socket to connect to new nodes errno: %d\n", errno);
//                exit(EXIT_FAILURE);
//            }
//            struct sockaddr_in dest;
//            dest.sin_family = AF_INET;
//            dest.sin_port = htons(new_node.port);
//            dest.sin_addr.s_addr = inet_addr(new_node.ip_address);
//            if (connect(client_socket, (struct sockaddr *) &dest, sizeof(struct sockaddr)) == -1) {
//                fprintf(stderr, "failed to connect to new node errno:%d\n", errno);
//                exit(EXIT_FAILURE);
//            }
//        }
//    }
    return 0;
}

int main(void) {
    pthread_t client, server;
    ssize_t bytes_read;
    memset(&this_node, 0, sizeof(this_node));
    // Init current node
    this_node.PeerList = (struct HashMap *) malloc(sizeof(struct HashMap));
    this_node.FileList = (struct HashMap *) malloc(sizeof(struct HashMap));

    //Init hash map
    init_map(this_node.PeerList, sizeof(char) * KEY_SIZE, sizeof(struct Peer), 5);
    //TODO: ADD init hashmap for files
    printf("How should i call you?\n");
    bytes_read = read(0, this_node.self.name, sizeof(this_node.self.name) - 1);
    this_node.self.name[bytes_read - 1] = '\0';
    printf("Ok! Your name is: %s\n", this_node.self.name);
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
            scanf("%[^:]:%hu", ip, &port);
            dest.sin_port = htons(port);
            dest.sin_addr.s_addr = inet_addr(ip);
            pthread_create(&client, NULL, initialise_client, (void *) &dest);
        }
        sleep(1);
    }
}