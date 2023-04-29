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
#include "../Message/Request.h"
#include "../Message/Response.h"
using namespace std;

Server::~Server()
{
    cout << "closed" << "\n";
    close(this->listener);
    close(this->disfd);
}
Server::Server(const char* port)
{
    int status;
    int yes = 1;
    addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, port, &hints, &res)) == -1)
    {
        cerr << "getaddrinfo: " << gai_strerror(status) << "\n";
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
    
    cout << "Server started on " << getIpStr(p->ai_addr) << "::"; 
    cout << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << "\n";
    
    freeaddrinfo(res);
    if (p == NULL)
    {
        cerr << "server: fail to bind\n";
        exit(1);
    }
    if (listen(this->listener, BACKLOG) == -1)
    {
        perror("server: listen");
        exit(1);
    }
    this->pfds.emplace_back();
    this->pfds.back().fd = this->listener;
    this->pfds.back().events = POLLIN;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, port, &hints, &res)) == -1)
    {
        cerr << "getaddrinfo: " << gai_strerror(status) << "\n";
        exit(1);
    }

    p = NULL;
    for (p = res; p != NULL; p = p->ai_next)
    {
        if ((this->disfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }
        if (setsockopt(this->disfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("server: setsockopt");
            exit(1); 
        }
        if (bind(this->disfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("server: bind");
            close(this->disfd);
            continue;
        }
        break;
    }
    
    cout << "Discover server started on " << getIpStr(p->ai_addr) << "::"; 
    cout << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << "\n";
    
    freeaddrinfo(res);
    if (p == NULL)
    {
        cerr << "server: fail to bind\n";
        exit(1);
    }

    this->pfds.emplace_back();
    this->pfds.back().fd = this->disfd;
    this->pfds.back().events = POLLIN;
}
void Server::start()
{
    int newfd;

    sockaddr_storage remote_address;
    socklen_t addrlen;

    char buffer[256];
    while (1)
    {
        int poll_count = poll(&pfds[0], pfds.size(), -1);
        if (poll_count == -1)
        {
            perror("poll");
            exit(1);
        }
        for (int i = 0; i < pfds.size(); ++i)
        {
            auto pfd = pfds[i];
            // get one ready to read
            if (pfd.revents & POLLIN)
            {
                // listener ready to read - new connection
                if (pfd.fd == listener)
                {
                    addrlen = sizeof(remote_address);
                    newfd = accept(listener, (sockaddr *)&remote_address, &addrlen);
                    if (newfd == -1)
                    {
                        perror("accept");
                    } else
                    {
                        pfds.emplace_back();
                        pfds.back().fd = newfd;
                        pfds.back().events = POLLIN;

                        
                        cout << "New connection from " << getIpStr((sockaddr *)&remote_address)
                             << " on socket " << newfd << "\n";
                    }
                } 
                else if (pfd.fd == disfd) // receive discover message
                {  
                    Request r;
                    addrlen = sizeof(remote_address);
                    recvfromRequest(disfd, r, 0, (sockaddr *)&remote_address, &addrlen);
                    if (r.type() == DISCOVER_REQUEST)
                    {
                        cout << "Receive discover message from " << getIpStr((sockaddr *)&remote_address) << "::"; 
                        cout << ntohs(((sockaddr_in *)&remote_address)->sin_port) << "\n";
                        Response msg(DISCOVER_RESPONSE, OK_ERRCODE, 0, NULL);
                        
                        sendtoResponse(disfd, msg, 0, (sockaddr *)&remote_address, addrlen);
                    }
                } 
                else
                {
                    int nbyte = recv(pfd.fd, buffer, sizeof buffer, 0);

                    
                }
            }
        }
    }
    
}
