#pragma once
#include <iostream>
#ifdef __APPLE__
    #include <sys/socket.h>
    #include <arpa/inet.h>
#elif _WIN32
    #include <winsock2.h>
    #include <WS2tcpip.h>
#endif

int sendAll(int sockfd, const void *msg, size_t len, int flag);
int recvAll(int sockfd, void *msg, size_t len, int flag);

int sendtoAll(int sockfd, const void *msg, size_t len, int flag, const sockaddr* addr, socklen_t addrlen);
int recvfromAll(int sockfd, void *msg, size_t len, int flag, sockaddr* addr, socklen_t *addrlen);