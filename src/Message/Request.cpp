#include "Request.h"
#include "../Utils/MsgTransport.h"
#include <cstring>

Request::Request(uint8_t type, uint64_t length, void *data)
{
    this->type = type;
    this->length = htonll(length);
    this->data = malloc(length);
    memcpy(this->data, data, length);
}
Request::Request()
{
    this->type = 0;
    this->length = 0;
    this->data = NULL;
}
Request::~Request()
{
    if (this->data != NULL)
        free(this->data);
}

int sendRequest(int sockfd, const Request &msg, int flag)
{
    if (sendAll(sockfd, &msg.type, sizeof(msg.type), 0) == -1)
        return -1;

    uint64_t length = htonll(msg.length);
    if (sendAll(sockfd, &length, sizeof(length), 0) == -1)
        return -1;
    
    if (sendAll(sockfd, msg.data, msg.length, 0) == -1)
        return -1;

    return 0;
}
int recvRequest(int sockfd, Request &msg, int flag)
{
    if (recvAll(sockfd, &msg.type, sizeof(msg.type), 0) == -1)
        return -1;
    
    if (recvAll(sockfd, &msg.length, sizeof(msg.length), 0) == -1)
        return -1;
    msg.length = ntohll(msg.length);
    
    if (msg.data != NULL) free(msg.data);
    msg.data = malloc(msg.length);
    if (recvAll(sockfd, msg.data, msg.length, 0) == -1)
        return -1;

    return 0;
}