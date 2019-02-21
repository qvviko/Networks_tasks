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
#include "pthread.h"
#include "time.h"

#define SERVER_PORT     2000 //Server will be running on this port number
#define TRUE 1
#define BUF_SIZE 1024
#define THREAD_N 10

struct handle_data {
    int socket_id;
    int thread_id;
};

void *handle_socket(void *data) {
    while (TRUE) {
        // Set up data buffer and client address storage
        struct handle_data *d = (struct handle_data *) data;
        char data_buf[BUF_SIZE];
        socklen_t client_addr_len = 0;
        client_addr_len = sizeof(struct sockaddr);
        ssize_t bytes_received, bytes_sent;
        struct sockaddr_in client_addr;


        printf("Thread %d blocked on receive call\n", d->thread_id);
        fflush(stdout);
        //Threads will listen to the same socket until one of them will catch the input and handle it
        if ((bytes_received = recvfrom(d->socket_id, data_buf, sizeof(data_buf), 0,
                                       (struct sockaddr *) &client_addr, &client_addr_len)) == -1) {
            fprintf(stderr, "failed to receive data errno: %d", errno);
            exit(EXIT_FAILURE);
        }
        time_t clk = time(NULL);
        test_struct_t *client_data = (test_struct_t *) data_buf;

        //Output data received
        printf("Thread %d. Client data: Name: %s Age: %d Group_N: %d. Timestamp:%s", d->thread_id, client_data->name,
               client_data->age,
               client_data->group_number, ctime(&clk));

        //Handle data
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

        //Sleep for 10 sec
        sleep(10);
        clk = time(NULL);

        //Slept, output result and sent it to the client
        printf("Thread %d, has slept. Result to send: %s. Timestamp:%s", d->thread_id, result.result, ctime(&clk));
        if ((bytes_sent = sendto(d->socket_id, &result, sizeof(result_struct_t), 0,
                                 (struct sockaddr *) &client_addr, sizeof(struct sockaddr))) == -1) {
            fprintf(stderr, "failed to sent data errno: %d", errno);
            exit(EXIT_FAILURE);
        }
    }
}

int setup_connection() {
    //Create socket and server addresses for binding
    int server_socket;
    struct sockaddr_in server_addr;
    //Array for storing all our threads
    pthread_t threads[THREAD_N];

    //Create server socket that is datagram for udp transmissions
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        fprintf(stderr, "failed to create a socket errno: %d", errno);
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr)); //Clean up server address
    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //Bind server socket to server address we set up earlier
    if ((bind(server_socket, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1)) {
        fprintf(stderr, "failed to bind errno: %d", errno);
        return -1;
    }

    //Create N threads for our server
    for (int i = 0; i < THREAD_N; i++) {
        struct handle_data *data = (struct handle_data *) malloc(sizeof(struct handle_data));
        data->thread_id = i + 1;
        data->socket_id = server_socket;
        pthread_create(&threads[i], NULL, handle_socket, (void *) data);
    }

    //Server will not close while at least one of the threads is alive
    for (int j = 0; j < THREAD_N; j++) {
        pthread_join(threads[j], NULL);
    }
    return 0;
}

int main(void) {
    return setup_connection();
}