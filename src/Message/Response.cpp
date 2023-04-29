#include "Response.h"
#include "../Utils/MsgTransport.h"
#include <cstring>

Response::Response(uint8_t type, uint32_t errcode, uint64_t length, void *data)
{
    this->header.type = type;
    this->header.errcode = errcode;
    this->header.length = length;
    if (data != NULL)
    {
        this->data = malloc(length);
        memcpy(this->data, data, length);
    } else this->data = NULL;
}
Response::Response()
{
    this->header.type = 0;
    this->header.errcode = 0;
    this->header.length = 0;
    this->data = NULL;
}
Response::~Response()
{
    if (this->data != NULL)
        free(this->data);
}
Response::Response(const Response &r)
{
    this->header.type = r.header.type;
    this->header.errcode = r.header.errcode;
    this->header.length = r.header.length;
    this->data = malloc(this->header.length);
    memcpy(this->data, r.data, this->header.length);
}
Response& Response::operator = (const Response &r)
{
    this->header.type = r.header.type;
    this->header.errcode = r.header.errcode;
    this->header.length = r.header.length;
    this->data = malloc(this->header.length);
    memcpy(this->data, r.data, this->header.length);
    return *this;
}
uint8_t Response::type()
{
    return this->header.type;
}
int sendResponse(int sockfd, const Response &msg, int flag)
{
    struct {
        uint8_t type;
        uint32_t errcode; 
        uint64_t length;
    } header;
    header.type = msg.header.type;
    header.errcode = htonl(msg.header.errcode);
    header.length = htonll(msg.header.length);
    
    if (sendAll(sockfd, &header, sizeof(header), 0) == -1)
        return -1;
    
    if (sendAll(sockfd, msg.data, msg.header.length, 0) == -1)
        return -1;

    return 0;
}
int recvResponse(int sockfd, Response &msg, int flag)
{
    if (recvAll(sockfd, &msg.header, sizeof(msg.header), 0) == -1)
        return -1;

    msg.header.errcode = ntohl(msg.header.errcode);
    msg.header.length = ntohll(msg.header.length);

    if (msg.data != NULL) free(msg.data);
    msg.data = malloc(msg.header.length);
    if (recvAll(sockfd, msg.data, msg.header.length, 0) == -1)
        return -1;

    return 0;
}


int sendtoResponse(int sockfd, const Response &msg, int flag, const sockaddr *addr, socklen_t addrlen)
{
    struct {
        uint8_t type;
        uint32_t errcode; 
        uint64_t length;
    } header;
    header.type = msg.header.type;
    header.errcode = htonl(msg.header.errcode);
    header.length = htonll(msg.header.length);
    if (sendtoAll(sockfd, &header, sizeof(header), 0, addr, addrlen) == -1)
        return -1;
    
    if (sendtoAll(sockfd, msg.data, msg.header.length, 0, addr, addrlen) == -1)
        return -1;

    return 0;
}
int recvfromResponse(int sockfd, Response &msg, int flag, sockaddr *addr, socklen_t *addrlen)
{
    if (recvfromAll(sockfd, &msg.header, sizeof(msg.header), 0, addr, addrlen) == -1)
        return -1;

    msg.header.errcode = ntohl(msg.header.errcode);
    msg.header.length = ntohll(msg.header.length);

    if (msg.data != NULL) free(msg.data);
    msg.data = malloc(msg.header.length);
    if (recvfromAll(sockfd, msg.data, msg.header.length, 0, addr, addrlen) == -1)
        return -1;

    return 0;
}