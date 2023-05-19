#include "Response.h"
#include "../Utils/MsgTransport.h"
#include <cstring>

Response::Response(uint8_t _type, uint8_t _errcode, uint64_t _length, void *_data)
{
    this->_header._type = _type;
    this->_header._errcode = _errcode;
    this->_header._length = _length;
    if (_data != NULL)
    {
        this->_data = malloc(_length);
        memcpy(this->_data, _data, _length);
    } else this->_data = NULL;
}
Response::Response()
{
    this->_header._type = 0;
    this->_header._errcode = 0;
    this->_header._length = 0;
    this->_data = NULL;
}
Response::~Response()
{
    if (this->_data != NULL)
        free(this->_data);
}
Response::Response(const Response &r)
{
    this->_header._type = r._header._type;
    this->_header._errcode = r._header._errcode;
    this->_header._length = r._header._length;
    this->_data = malloc(this->_header._length);
    memcpy(this->_data, r._data, this->_header._length);
}
Response& Response::operator = (const Response &r)
{
    this->_header._type = r._header._type;
    this->_header._errcode = r._header._errcode;
    this->_header._length = r._header._length;
    this->_data = malloc(this->_header._length);
    memcpy(this->_data, r._data, this->_header._length);
    return *this;
}
uint8_t Response::type()
{
    return this->_header._type;
}
void *Response::data()
{
    return this->_data;
}
uint8_t Response::errCode()
{
    return this->_header._errcode;
}
uint64_t Response::length()
{
    return this->_header._length;
}
int sendResponse(int sockfd, const Response &msg, int flag)
{
    struct {
        uint8_t _type;
        uint32_t _errcode; 
        uint64_t _length;
    } _header;
    _header._type = msg._header._type;
    _header._errcode = htonl(msg._header._errcode);
    _header._length = htonll(msg._header._length);
    
    if (sendAll(sockfd, &_header, sizeof(_header), 0) == -1)
        return -1;
    
    if (sendAll(sockfd, msg._data, msg._header._length, 0) == -1)
        return -1;

    return 0;
}
int recvResponse(int sockfd, Response &msg, int flag)
{
    if (recvAll(sockfd, &msg._header, sizeof(msg._header), 0) == -1)
        return -1;

    msg._header._errcode = ntohl(msg._header._errcode);
    msg._header._length = ntohll(msg._header._length);

    if (msg._data != NULL) free(msg._data);
    msg._data = malloc(msg._header._length);
    if (recvAll(sockfd, msg._data, msg._header._length, 0) == -1)
        return -1;
    return 0;
}

int sendtoResponse(int sockfd, const Response &msg, int flag, const sockaddr *addr, socklen_t addrlen)
{
    struct {
        uint8_t _type;
        uint32_t _errcode; 
        uint64_t _length;
    } _header;
    _header._type = msg._header._type;
    _header._errcode = htonl(msg._header._errcode);
    _header._length = htonll(msg._header._length);
    if (sendtoAll(sockfd, &_header, sizeof(_header), 0, addr, addrlen) == -1)
        return -1;
    
    if (sendtoAll(sockfd, msg._data, msg._header._length, 0, addr, addrlen) == -1)
        return -1;

    return 0;
}
int recvfromResponse(int sockfd, Response &msg, int flag, sockaddr *addr, socklen_t *addrlen)
{
    if (recvfromAll(sockfd, &msg._header, sizeof(msg._header), 0, addr, addrlen) == -1)
        return -1;

    msg._header._errcode = ntohl(msg._header._errcode);
    msg._header._length = ntohll(msg._header._length);

    if (msg._data != NULL) free(msg._data);
    msg._data = malloc(msg._header._length);
    if (recvfromAll(sockfd, msg._data, msg._header._length, 0, addr, addrlen) == -1)
        return -1;

    return 0;
}