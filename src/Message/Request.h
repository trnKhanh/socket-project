#pragma once
#include <stdint.h>
#include <arpa/inet.h>

#define DISCOVER_REQUEST 0x80
#define NOTCMD_REQUEST 0x00

#define LIST_APP_REQUEST 0x09
#define START_APP_REQUEST 0x0a
#define STOP_APP_REQUEST 0x0b

#define LIST_PROC_REQUEST 0x11
#define KILL_PROC_REQUEST 0x12

#define SCREENSHOT_REQUEST 0x19

#define START_KEYLOG_REQUEST 0x21
#define STOP_KEYLOG_REQUEST 0x22

#define DIR_TREE_REQUEST 0x29

class Request {
    struct {
        uint8_t _type;
        uint64_t _length;
    } _header;
    void *_data;
public:
    Request(uint8_t type, uint64_t length, void *data);
    Request();
    ~Request();
    Request(const Request &r);
    Request& operator = (const Request &r);

    uint8_t type();
    void * data();

    friend int sendRequest(int sockfd, const Request &msg, int flag);
    friend int recvRequest(int sockfd, Request &msg, int flag);

    friend int sendtoRequest(int sockfd, const Request &msg, int flag, const sockaddr *addr, socklen_t addrlen);
    friend int recvfromRequest(int sockfd, Request &msg, int flag, sockaddr *addr, socklen_t *addrlen);
};