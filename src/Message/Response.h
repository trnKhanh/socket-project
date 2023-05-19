#pragma once
#include <stdint.h>
#include "../InLibs.h"

#define DISCOVER_RESPONSE (uint8_t)0x80
#define CMD_RESPONSE_EMPTY (uint8_t)0x01
#define CMD_RESPONSE_STR (uint8_t)0x02
#define CMD_RESPONSE_PNG (uint8_t)0x03

#define OK_CODE 0
#define FAIL_CODE 1  

class Response {
    struct {
        uint8_t _type;
        uint8_t _errcode; 
        uint64_t _length;
    } _header;
    void *_data;
public:
    Response(uint8_t _type, uint8_t _errcode, uint64_t _length, void *_data);
    Response();
    ~Response();
    Response(const Response &r);
    Response& operator = (const Response &r);
    uint8_t type();
    void *data();
    uint8_t errCode();
    uint64_t length();

    friend int sendResponse(int sockfd, const Response &msg, int flag);
    friend int recvResponse(int sockfd, Response &msg, int flag);

    friend int sendtoResponse(int sockfd, const Response &msg, int flag, const sockaddr *addr, socklen_t addrlen);
    friend int recvfromResponse(int sockfd, Response &msg, int flag, sockaddr *addr, socklen_t *addrlen);
};