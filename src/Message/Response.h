#pragma once
#include <stdint.h>
#include <WS2tcpip.h>

// Structure Macro message

#define DISCOVER_RESPONSE ((1 << 7))
#define DISCONNECT_RESPONSE ((1 << 7) | (1 << 0))

#define LIST_APP_RESPONSE ((1 << 3) | (1 << 0))
#define START_APP_RESPONSE ((1 << 3) | (1 << 1))
#define STOP_APP_RESPONSE ((1 << 3) | (1 << 1) | (1 << 0))

#define LIST_PROCESS_RESPONSE ((1 << 4) | (1 << 0))
#define KILL_PROCESS_RESPONSE ((1 << 4) | (1 << 1))

#define SCREENSHOT_RESPONSE ((1 << 4) | (1 << 3) | (1 << 0))

#define START_KEYLOG_RESPONSE ((1 << 5) | (1 << 0))
#define STOP_KEYLOG_RESPONSE ((1 << 5) | (1 << 1))

#define DIR_TREE_RESPONSE ((1 << 5) | (1 << 3) | (1 << 0))

#define OK_CODE 0
#define FAIL_CODE 1  

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
