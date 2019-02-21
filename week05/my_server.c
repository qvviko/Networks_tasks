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

#define SERVER_PORT     2000 //Server will be running on this port number
#define TRUE 1
#define BUF_SIZE 1024

int setup_connection() {
    int server_socket;
    char data_buf[BUF_SIZE];
    ssize_t bytes_received, bytes_sent;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = 0;


    if ((server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        fprintf(stderr, "failed to create a socket errno: %d", errno);
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr)); //Clean up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    client_addr_len = sizeof(struct sockaddr);

    if ((bind(server_socket, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1)) {
        fprintf(stderr, "failed to bind errno: %d", errno);
        return -1;
    }

    //Ready to receive data

    while (TRUE) {
        printf("Blocked on receive\n");
        fflush(stdout);

        if ((bytes_received = recvfrom(server_socket, data_buf, sizeof(data_buf), 0,
                                       (struct sockaddr *) &client_addr, &client_addr_len)) == -1) {
            fprintf(stderr, "failed to receive data errno: %d", errno);
            return -1;
        }
        printf("No of bytes received = %lu\n", bytes_received);

        test_struct_t *client_data = (test_struct_t *) data_buf;

        char buf[20];
        result_struct_t result;
        int cur_int = 0;
        result.result[cur_int] = '(';
        cur_int++;
        strcpy(result.result + cur_int, client_data->name);
        cur_int += strlen(client_data->name);

        result.result[cur_int] = ',';
        result.result[cur_int + 1] = ' ';
        cur_int += 2;
        snprintf(buf, 10, "%d", client_data->age);
        strcpy(result.result + cur_int, buf);
        cur_int += strlen(buf);
        result.result[cur_int] = ',';
        result.result[cur_int + 1] = ' ';

        cur_int += 2;
        sprintf(buf, "%d", client_data->group_number);
        strcpy(result.result + cur_int, buf);

        cur_int += strlen(buf);
        result.result[cur_int] = ')';
        result.result[cur_int + 1] = '\0';

        if ((bytes_sent = sendto(server_socket, &result, sizeof(result_struct_t), 0,
                                 (struct sockaddr *) &client_addr, sizeof(struct sockaddr))) == -1) {
            fprintf(stderr, "failed to sent data errno: %d", errno);
            return -1;
        }
        printf("No of bytes sent = %lu\n", bytes_sent);

    }
}

int main(void) {
    return setup_connection();
}