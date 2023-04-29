#pragma once
#include <stdint.h>
#include <arpa/inet.h>

#define DISCOVER_REQUEST 0x80



class Request {
    struct {
        uint8_t type;
        uint64_t length;
    } header;
    void *data;
public:
    Request(uint8_t type, uint64_t length, void *data);
    Request();
    ~Request();
    Request(const Request &r);
    Request& operator = (const Request &r);

    uint8_t type();

    friend int sendRequest(int sockfd, const Request &msg, int flag);
    friend int recvRequest(int sockfd, Request &msg, int flag);

    friend int sendtoRequest(int sockfd, const Request &msg, int flag, const sockaddr *addr, socklen_t addrlen);
    friend int recvfromRequest(int sockfd, Request &msg, int flag, sockaddr *addr, socklen_t *addrlen);
};