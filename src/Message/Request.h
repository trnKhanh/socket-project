#pragma once
#include <stdint.h>

#define DISCOVER_MSG 0x


class Request {
    uint8_t type;
    uint64_t length;
    void *data;
public:
    Request(uint8_t type, uint64_t length, void *data);
    Request();
    ~Request();

    friend int sendRequest(int sockfd, const Request &msg, int flag);
    friend int recvRequest(int sockfd, Request &msg, int flag);
};