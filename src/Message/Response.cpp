#include "Response.h"

#include <cstring>

#include "../Utils/MsgTransport.h"
#include "../Utils/ConvertUtils.h"

Response::Response(uint8_t _type, uint32_t _errcode, uint64_t _length, void *_data){
    this->_header._type = _type;
    this->_header._errcode = _errcode;
    this->_header._length = _length;
    if (_data != NULL){
        this->_data = malloc(_length);
        memcpy(this->_data, _data, _length);
    } 
    else 
        this->_data = NULL;
}

Response::Response(){
    this->_header._type = 0;
    this->_header._errcode = 0;
    this->_header._length = 0;
    this->_data = NULL;
}

Response::~Response(){
    if (this->_data != NULL)
        free(this->_data);
}

Response::Response(const Response &r){
    this->_header._type = r._header._type;
    this->_header._errcode = r._header._errcode;
    this->_header._length = r._header._length;
    this->_data = malloc(this->_header._length);
    memcpy(this->_data, r._data, this->_header._length);
}

Response& Response::operator = (const Response &r){
    this->_header._type = r._header._type;
    this->_header._errcode = r._header._errcode;
    this->_header._length = r._header._length;
    this->_data = malloc(this->_header._length);
    memcpy(this->_data, r._data, this->_header._length);
    return *this;
}

uint8_t Response::type(){
    return this->_header._type;
}

void *Response::data(){
    return this->_data;
}

uint32_t Response::errCode(){
    return this->_header._errcode;
}

uint64_t Response::length(){
    return this->_header._length;
}

int sendResponse(SOCKET sockfd, const Response &msg, int flag){
    struct {
        uint8_t _type;
        uint32_t _errcode; 
        uint64_t _length;
    } _header;
    _header._type = msg._header._type;
    _header._errcode = htonl(msg._header._errcode);
    _header._length = my_htonll(msg._header._length);
    
    int status = sendAll(sockfd, &_header, sizeof(_header), 0);
    if (status == -1)
        return -1;
    
    status = sendAll(sockfd, msg._data, msg._header._length, 0);
    if (status == SOCKET_ERROR)
        return -1;

    return 0;
}

int recvResponse(SOCKET sockfd, Response &msg, int flag){
    int status = recvAll(sockfd, &msg._header, sizeof(msg._header), 0);
    if (status == SOCKET_ERROR)
        return -1;

    msg._header._errcode = ntohl(msg._header._errcode);
    msg._header._length = my_ntohll(msg._header._length);

    if (msg._data != NULL) 
        free(msg._data);
    
    msg._data = malloc(msg._header._length);
    status = recvAll(sockfd, msg._data, msg._header._length, 0);
    if (status == SOCKET_ERROR)
        return -1;
    
    return 0;
}


int sendtoResponse(SOCKET sockfd, const Response &msg, int flag, const sockaddr *addr, socklen_t addrlen){
    struct {
        uint8_t _type;
        uint32_t _errcode; 
        uint64_t _length;
    } _header;
    _header._type = msg._header._type;
    _header._errcode = htonl(msg._header._errcode);
    _header._length = my_htonll(msg._header._length);

    int status = sendtoAll(sockfd, &_header, sizeof(_header), 0, addr, addrlen);
    if (status == SOCKET_ERROR)
        return -1;
    
    status = sendtoAll(sockfd, msg._data, msg._header._length, 0, addr, addrlen);
    if (status == SOCKET_ERROR)
        return -1;

    return 0;
}

int recvfromResponse(SOCKET sockfd, Response &msg, int flag, sockaddr *addr, socklen_t *addrlen){
    int status = recvfromAll(sockfd, &msg._header, sizeof(msg._header), 0, addr, addrlen);
    if (status == SOCKET_ERROR)
        return -1;

    msg._header._errcode = ntohl(msg._header._errcode);
    msg._header._length = my_ntohll(msg._header._length);

    if (msg._data != NULL) free(msg._data);
    msg._data = malloc(msg._header._length);

    status = recvfromAll(sockfd, msg._data, msg._header._length, 0, addr, addrlen);
    if (status == SOCKET_ERROR)
        return -1;

    return 0;
}