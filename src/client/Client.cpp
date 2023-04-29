#include "Client.h"
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <iostream>
#include "../Message/Request.h"
#include "../Message/Response.h"
#include "../Utils/InUtils.h"
#include <poll.h>

Client::Client()
{
    std::vector<std::string> servers;
    this->discover(servers);
    std::cout << "Found following server:\n";
    for (auto u: servers)
    {
        std::cout << u << "\n";
    }
}
Client::~Client()
{
    close(this->sockfd);
}

int Client::discover(std::vector<std::string> &servers)
{
    int status;
    int disfd;
    int yes = 1;
    sockaddr_storage serverAddr;
    socklen_t addrlen = sizeof(serverAddr);
    addrinfo hints, *addr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, CLIENT_PORT, &hints, &addr)) != 0)
    {
        std::cerr << "client: getaddrinfo: " << gai_strerror(status) << "\n";
        return -1;
    }

    addrinfo *p = addr;
    for (;p != NULL; p = p->ai_next)
    {
        if ((disfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }
        if (setsockopt(disfd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes)) == -1)
        {
            perror("client: setsockopt");
            freeaddrinfo(addr);
            return -1;
        }
        if (setsockopt(disfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
        {
            perror("client: setsockopt");
            freeaddrinfo(addr);
            return -1;
        }
        if (bind(disfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("client: bind");
            close(disfd);
            continue;
        }
        break;
    }

    if (p == NULL)
    {
        std::cerr << "client: fail to bind socket\n";
        freeaddrinfo(addr);
        return -1;
    }

    std::cout << "Broadcast discover message from " << getIpStr(p->ai_addr) << "::"; 
    std::cout << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << "\n";

    freeaddrinfo(addr);
    if ((status = getaddrinfo("255.255.255.255", SERVER_PORT, &hints, &addr)) != 0)
    {
        std::cerr << "client: getaddrinfo: " << gai_strerror(status) << "\n";
        return -1;
    }
    p = addr;
    std::cout << "Broadcast discover message to " << getIpStr(p->ai_addr) << "::"; 
    std::cout << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << "\n";
    if (p == NULL)
    {
        std::cerr << "client: broadcast fail\n";
        freeaddrinfo(addr);
        return -1;
    }

    Request msg(DISCOVER_REQUEST, 0, NULL);
    sendtoRequest(disfd, msg, 0, p->ai_addr, p->ai_addrlen);
    freeaddrinfo(addr);

    pollfd pfds[1];
    pfds[0].events = POLLIN;
    pfds[0].fd = disfd;
    while(1)
    {
        int rv = poll(pfds, 1, 5000);
        if (rv == -1)
        {
            close(disfd); 
            perror("poll");
            return -1;
        }
        if (rv == 0) // time out
            break;
        
        Response buffer;
        recvfromResponse(disfd, buffer, 0, (sockaddr*)&serverAddr, &addrlen);
        if (buffer.type() == DISCOVER_RESPONSE)
            servers.push_back(getIpStr((sockaddr *)&serverAddr));
    }
    close(disfd);
    return 0;
}