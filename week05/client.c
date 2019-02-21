#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <unistd.h>
#include "common.h"
#include "errno.h"

#define DEST_PORT            2000
#define SERVER_IP_ADDRESS   "127.0.0.1"
test_struct_t client_data;
result_struct_t result;

void setup_tcp_communication() {
    /*Step 1 : Initialization*/
    /*Socket handle*/
    int sockfd = 0;
    ssize_t sent_recv_bytes = 0;

    socklen_t addr_len = 0;

    addr_len = sizeof(struct sockaddr);

    /*to store socket addesses : ip address and port*/
    struct sockaddr_in dest, self_addr;

    /*Step 2: specify server information*/
    /*Ipv4 sockets, Other values are IPv6*/
    dest.sin_family = AF_INET;
    dest.sin_port = htons(DEST_PORT);
    struct hostent *host = gethostbyname(SERVER_IP_ADDRESS);
    dest.sin_addr = *((struct in_addr *) host->h_addr);

    /*Step 3 : Create a TCP socket*/
    /*Create a socket finally. socket() is a system call, which asks for three paramemeters*/
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        fprintf(stderr, "Error on creating a socket");
        exit(1);
    }

    memset((void *) &self_addr, 0, sizeof(self_addr));

    self_addr.sin_family = AF_INET;
    self_addr.sin_port = htons(DEST_PORT + 1);
    self_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *) &self_addr, sizeof(self_addr)) == -1) {
        fprintf(stderr, "socket bind failed errno: %d\n", errno);
        exit(1);
    }

    /*Step 4 : get the data to be sent to server*/
    /*Our client is now ready to send data to server. sendto() sends data to Server*/

    while (1) {

        /*Prompt the user to enter data*/
        /*You will want to change the promt for the second task*/
        printf("Enter name : ?\n");
        ssize_t num_read = read(0, client_data.name, sizeof(client_data.name) - 1);
        client_data.name[num_read - 1] = '\0';
        printf("Enter age : ?\n");
        scanf("%u", &client_data.age);
        printf("Enter group number :? \n");
        scanf("%u", &client_data.group_number);

        printf("Sending data ...\n");
        sent_recv_bytes = sendto(sockfd,
                                 &client_data,
                                 sizeof(test_struct_t),
                                 0,
                                 (struct sockaddr *) &dest,
                                 sizeof(struct sockaddr));
        printf("No of bytes sent = %lu\n", sent_recv_bytes);

        /*Step 6 : Client also wants a reply from server after sending data*/

        /*recvfrom is a blocking system call, meaning the client program will not run past this point
         * until the data arrives on the socket from server*/
        printf("blocked on receive\n");
        sent_recv_bytes = recvfrom(sockfd, &result, sizeof(result_struct_t), 0,
                                   (struct sockaddr *) &dest, &addr_len);
        printf("No of bytes received = %lu\n", sent_recv_bytes);

        printf("Result received = %s\n", result.result);
    }
}


int main(int argc, char **argv) {
    setup_tcp_communication();
    printf("application quits\n");
    return 0;
}

