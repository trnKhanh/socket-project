#pragma once
#include <stdint.h>
#include <arpa/inet.h>

#define DISCOVER_RESPONSE 0x80


#define OK_ERRCODE 0
#define FAIL_ERRCODE 1  

class Response {
    struct {
        uint8_t type;
        uint32_t errcode; 
        uint64_t length;
    } header;
    void *data;
public:
    Response(uint8_t type, uint32_t errcode, uint64_t length, void *data);
    Response();
    ~Response();
    Response(const Response &r);
    Response& operator = (const Response &r);
    uint8_t type();

    friend int sendResponse(int sockfd, const Response &msg, int flag);
    friend int recvResponse(int sockfd, Response &msg, int flag);

    friend int sendtoResponse(int sockfd, const Response &msg, int flag, const sockaddr *addr, socklen_t addrlen);
    friend int recvfromResponse(int sockfd, Response &msg, int flag, sockaddr *addr, socklen_t *addrlen);
};