#pragma once
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

int sendAll(SOCKET sockfd, const void *msg, size_t len, int flag);
int recvAll(SOCKET sockfd, void *msg, size_t len, int flag);

int sendtoAll(SOCKET sockfd, const void *msg, size_t len, int flag, const sockaddr* addr, socklen_t addrlen);
int recvfromAll(SOCKET sockfd, void *msg, size_t len, int flag, sockaddr* addr, socklen_t *addrlen);