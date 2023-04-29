#pragma once
#include <iostream>

int sendAll(int sockfd, const void *msg, size_t len, int flag);
int recvAll(int sockfd, void *msg, size_t len, int flag);