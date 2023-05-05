#pragma once
#include <stdint.h>
#include "WinSock2.h"
#include <WS2tcpip.h>

// Structure Macro message

#define DISCOVER_REQUEST ((1 << 7))
#define DISCONNECT_REQUEST ((1 << 7) | (1 << 0))
#define UNKNOWN_REQUEST ((1 << 7) | (1 << 1))

#define LIST_APP_REQUEST ((1 << 3) | (1 << 0))
#define START_APP_REQUEST ((1 << 3) | (1 << 1))
#define STOP_APP_REQUEST ((1 << 3) | (1 << 1) | (1 << 0))

#define LIST_PROCESS_REQUEST ((1 << 4) | (1 << 0))
#define KILL_PROCESS_REQUEST ((1 << 4) | (1 << 1))

#define SCREENSHOT_REQUEST ((1 << 4) | (1 << 3) | (1 << 0))

#define START_KEYLOG_REQUEST ((1 << 5) | (1 << 0))
#define STOP_KEYLOG_REQUEST ((1 << 5) | (1 << 1))

#define DIR_TREE_REQUEST ((1 << 5) | (1 << 3) | (1 << 0))

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