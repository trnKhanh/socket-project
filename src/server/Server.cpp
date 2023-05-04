#include "Server.h"
#include <iostream>
#include <netdb.h>
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <poll.h>
#include <vector>
#include <netinet/in.h>
#include <signal.h>
#include <string>
#include "../Utils/InUtils.h"
#include "helper.h"

Server::~Server()
{
    std::cout << "closed" << "\n";
    close(this->listener);
    close(this->disfd);
}
Server::Server()
{
    char port[] = SERVER_PORT;
    int status;
    int yes = 1;
    addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, port, &hints, &res)) == -1)
    {
        std::cerr << "getaddrinfo: " << gai_strerror(status) << "\n";
        exit(1);
    }

    addrinfo *p = NULL;
    for (p = res; p != NULL; p = p->ai_next)
    {
        if ((this->listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }
        if (setsockopt(this->listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("server: setsockopt");
            exit(1); 
        }
        if (bind(this->listener, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("server: bind");
            close(this->listener);
            continue;
        }
        break;
    }
    
    std::cout << "Server started on " << getIpStr(p->ai_addr) << ":"; 
    std::cout << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << "\n";
    
    freeaddrinfo(res);
    if (p == NULL)
    {
        std::cerr << "server: fail to bind\n";
        exit(1);
    }
    if (listen(this->listener, BACKLOG) == -1)
    {
        perror("server: listen");
        exit(1);
    }
    this->pfds.push_back(pollfd());
    this->pfds.back().fd = this->listener;
    this->pfds.back().events = POLLIN;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, port, &hints, &res)) == -1)
    {
        std::cerr << "getaddrinfo: " << gai_strerror(status) << "\n";
        exit(1);
    }

    p = NULL;
    for (p = res; p != NULL; p = p->ai_next)
    {
        if ((this->disfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("server discover: socket");
            continue;
        }
        if (setsockopt(this->disfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("server discover: setsockopt");
            exit(1); 
        }
        if (bind(this->disfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("server discover: bind");
            close(this->disfd);
            continue;
        }
        break;
    }
    
    std::cout << "Discover server started on " << getIpStr(p->ai_addr) << ":"; 
    std::cout << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << "\n";
    
    freeaddrinfo(res);
    if (p == NULL)
    {
        std::cerr << "server: fail to bind\n";
        exit(1);
    }

    this->pfds.push_back(pollfd());
    this->pfds.back().fd = this->disfd;
    this->pfds.back().events = POLLIN;
}
void Server::start()
{
    int newfd;

    sockaddr_storage remote_address;
    socklen_t addrlen;
    int cnt = 0;
    while (cnt++ < 5)
    {
        int poll_count = poll(this->pfds.data(), pfds.size(), -1);
        if (poll_count == -1)
        {
            perror("poll");
            exit(1);
        }
        for (int i = 0; i < this->pfds.size(); ++i)
        {
            auto pfd = this->pfds[i];
            // get one ready to read
            if (pfd.revents & POLLIN)
            {
                // listener ready to read - new connection
                if (pfd.fd == this->listener)
                {
                    addrlen = sizeof(remote_address);
                    newfd = accept(this->listener, (sockaddr *)&remote_address, &addrlen);
                    if (newfd == -1)
                    {
                        perror("accept");
                    } else
                    {
                        this->pfds.push_back(pollfd());
                        this->pfds.back().fd = newfd;
                        this->pfds.back().events = POLLIN;

                        std::cout << "New connection from " << getIpStr((sockaddr *)&remote_address)
                             << " on socket " << newfd << "\n";
                    }
                } 
                else if (pfd.fd == this->disfd) // receive discover message
                {  
                    Request r;
                    addrlen = sizeof(remote_address);
                    if (recvfromRequest(this->disfd, r, 0, (sockaddr *)&remote_address, &addrlen) == -1)
                    {
                        perror("discover request receive");
                    }
                    else if (r.type() == DISCOVER_REQUEST)
                    {
                        std::cout << "Receive discover message from " << getIpStr((sockaddr *)&remote_address) << ":"; 
                        std::cout << ntohs(((sockaddr_in *)&remote_address)->sin_port) << "\n";
                        Response msg(DISCOVER_RESPONSE, OK_CODE, 0, NULL);
                        
                        if (sendtoResponse(this->disfd, msg, 0, (sockaddr *)&remote_address, addrlen) == -1)
                        {
                            perror("send offer");
                        }
                    }
                } 
                else
                {
                    Request buffer;
                    
                    if (recvRequest(pfd.fd, buffer, 0) == -1)
                    {
                        pfds[i] = pfds.back();
                        pfds.pop_back();
                        std::cerr << "Connection closed\n";
                    } 
                    else
                    {
                        Response res;
                        
                        if (buffer.type() == LIST_APP_REQUEST)
                            res = this->listApp();
                        else if (buffer.type() == START_APP_REQUEST)
                            res = this->startApp((char *)buffer.data());
                        else if (buffer.type() == STOP_APP_REQUEST)
                            res = this->stopApp((char *)buffer.data());
                        else if (buffer.type() == LIST_PROC_REQUEST)
                            res = this->listProcesss();
                        else if (buffer.type() == SCREENSHOT_REQUEST)
                            res = this->screenshot();
                        else if (buffer.type() == START_KEYLOG_REQUEST)
                            res = this->startKeylog();
                        else if (buffer.type() == STOP_KEYLOG_REQUEST)
                            res = this->stopKeylog();
                        else if (buffer.type() == DIR_TREE_REQUEST)
                            res = this->dirTree((char *)buffer.data());
                        else 
                        {
                            std::cerr << "Invalid request received on socket " << pfd.fd << std::endl;
                            continue;
                        }
                        if (sendResponse(pfd.fd, res, 0) == -1)
                        {
                            perror("sendResponse");
                        }
                    }

                    
                }
            }
        }
    }
}

Response Server::listApp()
{
    std::string buffer;
    uint32_t errCode;
    if (listAppHelper(buffer) == -1)
    {
        buffer.clear();
        errCode = FAIL_CODE;
    }
    else errCode = OK_CODE;

    return Response(CMD_RESPONSE_STR, errCode, buffer.size() + 1, (void *)buffer.c_str());
}
Response Server::startApp(const char *appName)
{
    uint32_t errCode;
    if (startAppHelper(appName) == -1)
    {
        errCode = FAIL_CODE;
    }
    else errCode = OK_CODE;

    return Response(CMD_RESPONSE_EMPTY, errCode, 0, NULL);
}
Response Server::stopApp(const char *appName)
{
    uint32_t errCode;
    if (stopAppHelper(appName) == -1)
    {
        errCode = FAIL_CODE;
    }
    else errCode = OK_CODE;

    return Response(CMD_RESPONSE_EMPTY, errCode, 0, NULL);
}

Response Server::listProcesss()
{
    std::string buffer;
    int status = listProcessesStrHelper(buffer);
    uint32_t errCode;
    if (status == -1)
    {
        errCode = FAIL_CODE;
        buffer.clear();
    }
    else errCode = OK_CODE;

    return Response(CMD_RESPONSE_STR, errCode, buffer.size() + 1, (void *)buffer.c_str());
}

Response Server::screenshot()
{
    std::vector<char> buffer;
    uint32_t errCode;
    if (screenshotHelper(buffer))
        errCode = FAIL_CODE;
    else errCode = OK_CODE;

    return Response(CMD_RESPONSE_PNG, errCode, buffer.size(), buffer.data());
}

Response Server::startKeylog()
{
    return Response();
}
Response Server::stopKeylog()
{
    return Response();
}

Response Server::dirTree(const char *pathName)
{
    std::cout << "Processing list directory tree rooted at " << pathName << std::endl;
    std::string buffer;
    int status = listDirTreeHelper(pathName, buffer);
    uint32_t errCode;
    if (status == -1)
    {
        errCode = FAIL_CODE;
        buffer.clear();
    }
    else errCode = OK_CODE;

    return Response(CMD_RESPONSE_STR, errCode, buffer.size() + 1, (void *)buffer.c_str());
}