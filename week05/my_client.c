

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include "common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP_ADDRESS "localhost"
#define SERVER_PORT     2000 //Server will be running on this port number
#define TRUE 1

int setup_connection() {
    int client_socket;
    ssize_t bytes_received, bytes_sent;
    struct sockaddr_in client_addr, destination_addr;
    socklen_t addr_len = 0;
    test_struct_t client_data;
    result_struct_t result;


    if ((client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        fprintf(stderr, "failed to create a socket errno: %d", errno);
        return -1;
    }

    memset(&client_addr, 0, sizeof(client_addr)); //Clean up server address
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(SERVER_PORT + 1);
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    addr_len = sizeof(struct sockaddr);
    if ((bind(client_socket, (struct sockaddr *) &client_addr, sizeof(struct sockaddr)) == -1)) {
        fprintf(stderr, "failed to bind errno: %d", errno);
        return -1;
    }

    destination_addr.sin_family = AF_INET;

    destination_addr.sin_port = htons(SERVER_PORT);
    struct hostent *host = gethostbyname(SERVER_IP_ADDRESS);
    destination_addr.sin_addr = *((struct in_addr *) host->h_addr);


    //Ready to receive data
    while (TRUE) {
        printf("Enter name : ?\n");
        ssize_t num_read = read(0, client_data.name, sizeof(client_data.name) - 1);
        client_data.name[num_read - 1] = '\0';
        printf("Enter age : ?\n");
        scanf("%u", &client_data.age);
        printf("Enter group number :? \n");
        scanf("%u", &client_data.group_number);

        printf("Sending data ...\n");
        if ((bytes_sent = sendto(client_socket, &client_data, sizeof(test_struct_t), 0,
                                 (struct sockaddr *) &destination_addr, sizeof(struct sockaddr))) == -1) {
            fprintf(stderr, "failed to sent errno: %d", errno);
        }

        printf("No of bytes sent = %lu\n", bytes_sent);

        printf("Blocked on receive\n");
        bytes_received = recvfrom(client_socket, &result, sizeof(result_struct_t), 0,
                                  (struct sockaddr *) &destination_addr, &addr_len);
        printf("No of bytes received = %lu\n", bytes_received);
        printf("Result received = %s\n", result.result);

    }
}

int main(void) {
    return setup_connection();
}