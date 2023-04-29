#include "Response.h"
#include "../Utils/MsgTransport.h"
#include <cstring>

Response::Response(uint8_t type, uint32_t errcode, uint64_t length, void *data)
{
    this->type = type;
    this->errcode = errcode;
    this->length = length;
    this->data = malloc(length);
    memcpy(this->data, data, length);
}
Response::Response()
{
    this->type = 0;
    this->errcode = 0;
    this->length = 0;
    this->data = NULL;
}
Response::~Response()
{
    if (this->data != NULL)
        free(this->data);
}

int sendResponse(int sockfd, const Response &msg, int flag)
{
    if (sendAll(sockfd, &msg.type, sizeof(msg.type), 0) == -1)
        return -1;

    uint32_t errcode = htonl(msg.errcode);
    if (sendAll(sockfd, &errcode, sizeof(errcode), 0) == -1)
        return -1;
    
    uint64_t length = htonll(msg.length);
    if (sendAll(sockfd, &length, sizeof(length), 0) == -1)
        return -1;
    
    if (sendAll(sockfd, msg.data, msg.length, 0) == -1)
        return -1;

    return 0;
}
int recvResponse(int sockfd, Response &msg, int flag)
{
    if (recvAll(sockfd, &msg.type, sizeof(msg.type), 0) == -1)
        return -1;
    
    if (recvAll(sockfd, &msg.errcode, sizeof(msg.errcode), 0) == -1)
        return -1;    
    msg.errcode = ntohl(msg.errcode);

    if (recvAll(sockfd, &msg.length, sizeof(msg.length), 0) == -1)
        return -1;
    msg.length = ntohll(msg.length);

    if (msg.data != NULL) free(msg.data);
    msg.data = malloc(msg.length);
    if (recvAll(sockfd, msg.data, msg.length, 0) == -1)
        return -1;

    return 0;
}