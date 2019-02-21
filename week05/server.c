//Taken from Abhishek Sagar

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

/*Server process is running on this port no. Client has to send data to this port no*/
#define SERVER_PORT     2000

test_struct_t test_struct;
result_struct_t res_struct;
char data_buffer[1024];

void setup_tcp_server_communication() {

    /*Socket handle and other variables*/
    /*Master socket file descriptor, used to accept new client connection only, no data exchange*/
    int master_socket = 0;
    ssize_t sent_recv_bytes = 0;
    socklen_t addr_len = 0;

    /*client specific communication socket file descriptor,
     * used for only data exchange/communication between client and server*/
    int comm_socket_fd = 0;
    /*variables to hold server information*/
    struct sockaddr_in server_addr, client_addr;

    /*step 2: udp master socket creation*/
    if ((master_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        printf("socket creation failed\n");
        exit(1);
    }

    memset((void *) &server_addr, 0, sizeof(server_addr));
    /*Step 3: specify server Information*/
    server_addr.sin_family = AF_INET;/*This socket will process only ipv4 network packets*/
    server_addr.sin_port = htons(SERVER_PORT);/*Server will process any data arriving on port no 2000*/
    /*3232249957; //( = 192.168.56.101); Server's IP address,
    //means, Linux will send all data whose destination address = address of any local interface
    //of this machine, in this case it is 192.168.56.101*/

    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    addr_len = sizeof(struct sockaddr);

    /* Bind the server. Binding means, we are telling kernel(OS) that any data
     * you recieve with dest ip address = 192.168.56.101, and tcp port no = 2000, pls send that data to this process
     * bind() is a mechnism to tell OS what kind of data server process is interested in to recieve. Remember, server machine
     * can run multiple server processes to process different data and service different clients. Note that, bind() is
     * used on server side, not on client side*/
    if (bind(master_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        printf("socket bind failed\n");
        exit(1);
    }
    /* Server infinite loop for servicing the client*/

    while (1) {
        memset(data_buffer, 0, sizeof(data_buffer));
        printf("blocked on receive call");
        fflush(stdout);
        if ((sent_recv_bytes = recvfrom(master_socket, data_buffer, sizeof(data_buffer), 0,
                                        (struct sockaddr *) &client_addr, &addr_len)) == -1) {
            printf("error on recieve");
            exit(1);
        }
        printf("i'm here\n");
        fflush(stdout);

        printf("Server recvd %lu bytes from client %s:%u\n", sent_recv_bytes,
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        test_struct_t *client_data = (test_struct_t *) data_buffer;

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


        /* Server replying back to client now*/
        sent_recv_bytes = sendto(master_socket, &result, sizeof(result_struct_t), 0,
                                 (struct sockaddr *) &client_addr, sizeof(struct sockaddr));

        printf("Server sent %lu bytes in reply to client\n", sent_recv_bytes);
        /*Goto state machine State 3*/
    }
}

int
main(void) {

    setup_tcp_server_communication();
    return 0;
}
