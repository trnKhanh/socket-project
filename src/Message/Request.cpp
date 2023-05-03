#include <cstring>

#include "Request.h"
#include "../Utils/MsgTransport.h"
#include "../Utils/ConvertUtils.h"

Request::Request(uint8_t type, uint64_t length, void *data){
    this->header.type = type;
    this->header.length = my_htonll(length);
    if (data != NULL){
        this->data = malloc(length);
        memcpy(this->data, data, length);
    }
    else
        this->data = NULL;
}

Request::Request(){
    this->header.type = 0;
    this->header.length = 0;
    this->data = NULL;
}

Request::~Request(){
    if (this->data != NULL)
        free(this->data);
}

Request::Request(const Request &r){
    this->header.type = r.header.type;
    this->header.length = r.header.length;
    this->data = malloc(this->header.length);
    memcpy(this->data, r.data, this->header.length);
}

Request& Request::operator = (const Request &r){
    this->header.type = r.header.type;
    this->header.length = r.header.length;
    this->data = malloc(this->header.length);
    memcpy(this->data, r.data, this->header.length);
    return *this;
}

uint8_t Request::type(){
    return this->header.type;
}

int sendRequest(int sockfd, const Request &msg, int flag){
    struct {
        uint8_t type;
        uint64_t length;
    } header;
    header.type = msg.header.type;
    header.length = my_htonll(msg.header.length);
    
    if (sendAll(sockfd, &header, sizeof(header), 0) == -1)
        return -1;
    
    if (sendAll(sockfd, msg.data, msg.header.length, 0) == -1)
        return -1;

    return 0;
}

int recvRequest(int sockfd, Request &msg, int flag){
    if (recvAll(sockfd, &msg.header, sizeof(msg.header), 0) == -1)
        return -1;

    msg.header.length = my_ntohll(msg.header.length);

    if (msg.data != NULL) free(msg.data);
    msg.data = malloc(msg.header.length);
    if (recvAll(sockfd, msg.data, msg.header.length, 0) == -1)
        return -1;

    return 0;
}

int sendtoRequest(int sockfd, const Request &msg, int flag, const sockaddr *addr, socklen_t addrlen){
    struct {
        uint8_t type;
        uint64_t length;
    } header;
    header.type = msg.header.type;
    header.length = my_htonll(msg.header.length);
    
    if (sendtoAll(sockfd, &header, sizeof(header), 0, addr, addrlen) == -1)
        return -1;
    
    if (sendtoAll(sockfd, msg.data, msg.header.length, 0, addr, addrlen) == -1)
        return -1;

    return 0;
}

int recvfromRequest(int sockfd, Request &msg, int flag, sockaddr *addr, socklen_t *addrlen){
    if (recvfromAll(sockfd, &msg.header, sizeof(msg.header), 0, addr, addrlen) == -1)
        return -1;

    msg.header.length = my_ntohll(msg.header.length);

    if (msg.data != NULL) free(msg.data);
    msg.data = malloc(msg.header.length);
    if (recvfromAll(sockfd, msg.data, msg.header.length, 0, addr, addrlen) == -1)
        return -1;

    return 0;
}