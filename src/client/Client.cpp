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
#include <fstream>
// TODO: Change STOP_APP TO KILL BY PID
Client::Client()
{
    std::vector<std::string> servers;
    if (this->discover(servers) == -1) 
        exit(1);
    
    std::cout << "Found following server:\n";
    for (auto u: servers)
    {
        std::cout << u << "\n";
    }

    std::cout << "Choose server to connect: ";
    std::string name = servers[0];
    // std::cin >> name;

    addrinfo hints, *servinfo;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((status = getaddrinfo(name.c_str(), SERVER_PORT, &hints, &servinfo)) != 0)
    {
        std::cerr << "client: getaddrinfo " << gai_strerror(status) << "\n";
        exit(1);
    }

    addrinfo *p = servinfo;
    for (;p != NULL; p = p->ai_next)
    {
        if ((this->sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }
        if (connect(this->sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("client: connect");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);
    if (p == NULL)
    {
        std::cerr << "client: fail to connect\n";
        exit(1);
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

    if ((status = getaddrinfo(NULL, "0", &hints, &addr)) != 0)
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
        addrlen = sizeof(serverAddr);
        recvfromResponse(disfd, buffer, 0, (sockaddr*)&serverAddr, &addrlen);
        if (buffer.type() == DISCOVER_RESPONSE)
            servers.push_back(getIpStr((sockaddr *)&serverAddr));
    }
    close(disfd);
    return 0;
}

int Client::listApp()
{
    Request req(LIST_APP_REQUEST, 0, NULL);
    if (sendRequest(this->sockfd, req, 0))
        return -1;
    Response res;
    if (recvResponse(this->sockfd, res, 0))
        return -1;
    if (res.errCode() == FAIL_CODE || res.type() != CMD_RESPONSE_STR)
        return -1;
    std::cout << (char*)res.data() << "\n";
    return 0;
}
int Client::startApp(const std::string &appName)
{
    Request req(START_APP_REQUEST, appName.size() + 1, (void *)appName.c_str());
    if (sendRequest(this->sockfd, req, 0))
        return -1;
    Response res;
    if (recvResponse(this->sockfd, res, 0))
        return -1;
    if (res.errCode() == FAIL_CODE || res.type() != CMD_RESPONSE_EMPTY)
        return -1;
    return 0;
}
int Client::stopApp(const std::string &appName)
{
    Request req(STOP_APP_REQUEST, appName.size() + 1, (void *)appName.c_str());
    if (sendRequest(this->sockfd, req, 0))
        return -1;
    Response res;
    if (recvResponse(this->sockfd, res, 0))
        return -1;
    if (res.errCode() == FAIL_CODE || res.type() != CMD_RESPONSE_EMPTY)
        return -1;
    return 0;
}

int Client::listProcesss()
{
    Request req(LIST_PROC_REQUEST, 0, NULL);
    if (sendRequest(this->sockfd, req, 0))
        return -1;
    Response res;
    if (recvResponse(this->sockfd, res, 0))
        return -1;
    if (res.errCode() == FAIL_CODE || res.type() != CMD_RESPONSE_STR)
        return -1;
    std::cout << (char*)res.data() << "\n";
    return 0;
}

int Client::screenshot()
{
    Request req(SCREENSHOT_REQUEST, 0, NULL);
    if (sendRequest(this->sockfd, req, 0))
        return -1;
    std::cout << "send resquest\n";
    Response res;
    if (recvResponse(this->sockfd, res, 0))
        return -1;
    if (res.errCode() == FAIL_CODE || res.type() != CMD_RESPONSE_PNG)
    {
        std::cout << "Invalid response\n";
        return -1;
    }
    std::cout << "Valid response\n";
    std::ofstream os("screenshot.png", std::ios::binary);
    if (!os)
    {
        return -1;
    }
    os.write((char *)res.data(), res.length());
    return 0;
}

int Client::startKeylog()
{
    Request req(START_KEYLOG_REQUEST, 0, NULL);
    if (sendRequest(this->sockfd, req, 0))
        return -1;
    Response res;
    if (recvResponse(this->sockfd, res, 0))
        return -1;
    if (res.errCode() == FAIL_CODE || res.type() != CMD_RESPONSE_EMPTY)
        return -1;
    while (recvResponse(this->sockfd, res, 0) != -1)
    {
        std::string msg((char *)res.data());
        std::cout << msg << std::endl;
    }
    return 0;
}
int Client::stopKeylog()
{
    Request req(STOP_KEYLOG_REQUEST, 0, NULL);
    if (sendRequest(this->sockfd, req, 0))
        return -1;
    Response res;
    if (recvResponse(this->sockfd, res, 0))
        return -1;
    if (res.errCode() == FAIL_CODE || res.type() != CMD_RESPONSE_EMPTY)
        return -1;
    return 0;
}

int Client::dirTree(const std::string pathName)
{
    Request req(DIR_TREE_REQUEST, pathName.size() + 1, (void *)pathName.c_str());
    if (sendRequest(this->sockfd, req, 0))
        return -1;
    Response res;
    if (recvResponse(this->sockfd, res, 0))
        return -1;
    if (res.errCode() == FAIL_CODE || res.type() != CMD_RESPONSE_STR)
    {
        std::cout << "invalid response\n";
        return -1;
    }
    std::cout << (char*)res.data() << "\n";
    return 0;
}