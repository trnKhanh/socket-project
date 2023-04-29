#pragma once
#include <stdint.h>

class Response {
    uint8_t type;
    uint32_t errcode; 
    uint64_t length;
    void *data;
public:
    Response(uint8_t type, uint32_t errcode, uint64_t length, void *data);
    Response();
    ~Response();

    friend int sendResponse(int sockfd, const Response &msg, int flag);
    friend int recvResponse(int sockfd, Response &msg, int flag);
};