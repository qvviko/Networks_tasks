#include "node.h"


int peer_cmp(Peer p1, Peer p2) {
    int acc;
    acc = strcmp(p1.ip_address, p2.ip_address);
    if (acc == 0 && p1.port == p2.port)
        return TRUE;
    else
        return FALSE;
}

void add_peer(struct LinkedList *list, struct Peer item) {
    if (list->length == 0) {
        list->self = (struct LinkedNode *) malloc(sizeof(struct LinkedNode));
        list->self->value = item;
        list->self->previous = NULL;
        list->self->next = NULL;
    } else {
        struct LinkedNode *prev = list->self;
        struct LinkedNode *cur = list->self->next;
        while (cur != NULL) {
            prev = cur;
            cur = cur->next;
        }
        prev->next = (struct LinkedNode *) malloc(sizeof(struct LinkedNode));
        prev->next->previous = prev;
        prev->next->next = NULL;
        prev->next->value = item;
    }
    list->length++;
}

int find_peer(struct LinkedList *list, struct Peer item) {
    if (list->length == 0)
        return FALSE;
    else if (peer_cmp(item, this_node.self) == TRUE) {
        return FALSE;
    } else {
        struct LinkedNode *cur = list->self;
        while (cur != NULL && peer_cmp(item, cur->value) == FALSE) {
            cur = cur->next;
        }
        if (cur == NULL) {
            return FALSE;
        }
        return TRUE;
    }
}

void remove_peer(struct LinkedList *list, struct Peer item) {
    if (list->length == 0) {
        return;
    } else {
        struct LinkedNode *prev = NULL;
        struct LinkedNode *cur = list->self;
        while (cur != NULL && peer_cmp(item, cur->value) == TRUE) {
            prev = cur;
            cur = cur->next;
        }
        if (cur == NULL)
            return;
        else {
            if (prev == NULL) {
                list->self = cur->next;
            } else
                prev->next = cur->next;
            if (cur->next != NULL) {
                cur->next->previous = cur->previous;
            }
            free(cur);
        }
    }
    list->length--;
}

void get_peers(struct LinkedList list, struct Peer *items) {
    struct LinkedNode *cur = list.self;
    for (int i = 0; i < list.length; ++i) {
        items[i] = cur->value;
        cur = cur->next;
    }
}

void connect_to_peer(struct Peer peer) {
    ssize_t bytes_sent;
    int client_socket;
    struct Protocol p;
    struct sockaddr_in destination_addr;
    // Create client's socket from which he will connect
    socklen_t addr_len = sizeof(struct sockaddr);
    if ((client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        fprintf(stderr, "failed to create a client socket errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    // Add address to list (address of the server)
    if (find_peer(&this_node.peers, peer) == FALSE) {
        // Add address to list (address of the server)
        add_peer(&this_node.peers, peer);
        printf("Got new node! Name:%s:%u\n", peer.ip_address,
               peer.port);
    }
    destination_addr.sin_family = AF_INET;
    destination_addr.sin_addr.s_addr = inet_addr(peer.ip_address);
    destination_addr.sin_port = htons(peer.port);

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
    close(client_socket);
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
        pthread_create(&clients[current_connect], NULL, handle_client, (void *) &c_data);
        current_connect = (current_connect + 1) % CONNECT_N;
    }
}

void *ping_clients(void *data) {
    struct Peer *peers = malloc(sizeof(char) * 0);
    struct Protocol p;
    struct Peer peer_buf[PEER_BUF];
    ssize_t bytes_received, bytes_sent;
    p.type = PROT_PING;
    int connect_fd, peer_num, peer_sync_num;
    struct sockaddr_in server_addr;
    socklen_t addr_len;

    while (TRUE) {
        sleep(PING_INTERVAL);

        peer_num = this_node.peers.length;
        peers = (Peer *) realloc(peers, sizeof(Peer) * peer_num);
        get_peers(this_node.peers, peers);
        for (int i = 0; i < peer_num; ++i) {
            addr_len = sizeof(server_addr);
            p.type = PROT_PING;
            if ((connect_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
                fprintf(stderr, "failed to create a socket to ping clients errno: %d\n", errno);
                exit(EXIT_FAILURE);
            }
            printf("pinging :%s:%s:%u \n", peers[i].name,
                   peers[i].ip_address, peers[i].port);
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(peers[i].port);
            server_addr.sin_addr.s_addr = inet_addr(peers[i].ip_address);

            if (connect(connect_fd, (struct sockaddr *) &server_addr, addr_len) == -1) {
                if (errno == ECONNREFUSED) {
                    printf("Node Name:%s:%s:%u left\n", peers[i].name,
                           peers[i].ip_address, peers[i].port);
                    //Remove item from the hashmap)
                    remove_peer(&this_node.peers, peers[i]);
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
                    remove_peer(&this_node.peers, peers[i]);
                    close(connect_fd);
                    continue;
                } else {
                    fprintf(stderr, "error on receive ping errno: %d\n", errno);
                    exit(EXIT_FAILURE);
                }
            }

            //Begin SYNC

            //SYNC PEERS
            p.type = PROT_SYNC_PEERS;
            bytes_sent = sendto(connect_fd, (void *) &p, sizeof(p), 0,
                                (struct sockaddr *) &server_addr,
                                sizeof(struct sockaddr));
            if (bytes_sent == -1) {
                fprintf(stderr, "error on send ping errno: %d\n", errno);
                exit(EXIT_FAILURE);
            }
            // Get num of peers
            bytes_received = recvfrom(connect_fd, (void *) &peer_sync_num, sizeof(peer_sync_num), 0,
                                      (struct sockaddr *) &server_addr,
                                      &addr_len);
            if (bytes_received == -1) {
                fprintf(stderr, "error on receive ping errno: %d\n", errno);
                exit(EXIT_FAILURE);
            }
            while (peer_sync_num > 0) {
                bytes_received = recvfrom(connect_fd, (void *) &peer_buf, sizeof(peer_buf), 0,
                                          (struct sockaddr *) &server_addr,
                                          &addr_len);
                if (bytes_received == -1) {
                    fprintf(stderr, "error on receive ping errno: %d\n", errno);
                    exit(EXIT_FAILURE);
                }
                for (int j = 0; j < PEER_BUF; ++j) {
                    if (find_peer(&this_node.peers, peer_buf[i]) == FALSE) {
                        connect_to_peer(peer_buf[i]);
                    }
                }
            }


            //SYNC FILES
            p.type = PROT_SYNC_FILES;
            bytes_sent = sendto(connect_fd, (void *) &p, sizeof(p), 0,
                                (struct sockaddr *) &server_addr,
                                sizeof(struct sockaddr));
            if (bytes_sent == -1) {
                fprintf(stderr, "error on send ping errno: %d\n", errno);
                exit(EXIT_FAILURE);
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

        if (find_peer(&this_node.peers, new_node) == FALSE) {
            printf("Got new node! Name:%s:%s:%u\n", new_node.name, new_node.ip_address,
                   new_node.port);
            //Add item
            add_peer(&this_node.peers, new_node);
        }
    } else if (p.type == PROT_SYNC_PEERS) {
        int peer_size = this_node.peers.length;
        struct Peer *sync_peers = malloc(peer_size * sizeof(struct Peer));
        struct Peer peer_buf[PEER_BUF];
        bytes_sent = sendto(client_data->client_socket, (void *) &peer_size, sizeof(int), 0,
                            (struct sockaddr *) &client_data->client_addr,
                            sizeof(struct sockaddr));
        if (bytes_sent == -1) {
            fprintf(stderr, "Error on sending ack errno : %d\n", errno);
            exit(EXIT_FAILURE);
        }
        while (peer_size > 0) {
            memcpy(peer_buf, sync_peers, sizeof(peer_buf));
            sync_peers += sizeof(peer_buf);
            peer_size -= PEER_BUF;
            bytes_sent = sendto(client_data->client_socket, (void *) &peer_buf, sizeof(peer_buf), 0,
                                (struct sockaddr *) &client_data->client_addr,
                                sizeof(struct sockaddr));
            if (bytes_sent == -1) {
                fprintf(stderr, "Error on sending ack errno : %d\n", errno);
                exit(EXIT_FAILURE);
            }
        }

    } else if (p.type == PROT_SYNC_FILES) {
        
    }
    close(client_data->client_socket);
    return 0;
}

void *initialise_client(void *data) {
    //Set up variables for client socket its address and destination address
    Peer new_peer;
    ssize_t bytes_sent;
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
    strcpy(new_peer.ip_address, inet_ntoa(destination_addr.sin_addr));
    new_peer.port = ntohs(destination_addr.sin_port);
    if (find_peer(&this_node.peers, new_peer) == FALSE) {
        // Add address to list (address of the server)
        add_peer(&this_node.peers, new_peer);
        printf("Got new node! Name:%s:%u\n", new_peer.ip_address,
               new_peer.port);
    }

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
    return 0;
}

int main(void) {
    pthread_t client, server;
    ssize_t bytes_read;
    memset(&this_node, 0, sizeof(this_node));
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
    }
}