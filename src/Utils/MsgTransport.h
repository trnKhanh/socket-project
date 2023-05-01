#pragma once
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

int sendAll(int sockfd, const void *msg, size_t len, int flag);
int recvAll(int sockfd, void *msg, size_t len, int flag);

int sendtoAll(int sockfd, const void *msg, size_t len, int flag, const sockaddr* addr, socklen_t addrlen);
int recvfromAll(int sockfd, void *msg, size_t len, int flag, sockaddr* addr, socklen_t *addrlen);