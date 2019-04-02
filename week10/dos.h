//
// Created by vlad on 31.03.19.
//

#ifndef NETWORKS_TASKS_DOS_H
#define NETWORKS_TASKS_DOS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>

struct Victim {
    char ip_address[20];
    uint16_t port;
};
#define PROT_SYNC 1
#define TRUE 1
#define MAX_CONNECT 100
#endif //NETWORKS_TASKS_DOS_H
