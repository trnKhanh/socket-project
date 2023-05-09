#pragma once
#include <stdint.h>

#include "WinSock2.h"
#include <WS2tcpip.h>

// Structure Macro message
#define DISCOVER_REQUEST (uint8_t)0x80
#define NOTCMD_REQUEST (uint8_t)0x00

#define LIST_APP_REQUEST (uint8_t)0x09
#define START_APP_REQUEST (uint8_t)0x0a
#define STOP_APP_REQUEST (uint8_t)0x0b

#define LIST_PROC_REQUEST (uint8_t)0x11
#define KILL_PROC_REQUEST (uint8_t)0x12

#define SCREENSHOT_REQUEST (uint8_t)0x19

#define START_KEYLOG_REQUEST (uint8_t)0x21
#define STOP_KEYLOG_REQUEST (uint8_t)0x22

#define DIR_TREE_REQUEST (uint8_t)0x29

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

    friend int sendRequest(SOCKET sockfd, const Request &msg, int flag);
    friend int recvRequest(SOCKET sockfd, Request &msg, int flag);

    friend int sendtoRequest(SOCKET sockfd, const Request &msg, int flag, const sockaddr *addr, socklen_t addrlen);
    friend int recvfromRequest(SOCKET sockfd, Request &msg, int flag, sockaddr *addr, socklen_t *addrlen);
};