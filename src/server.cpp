#include "server.h"
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
#include "utils.h"
#include <signal.h>
#include <string>

using namespace std;

Server::~Server()
{
    cout << "closed" << "\n";
    close(listener);
}
Server::Server(const char* port)
{
    addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int rv;
    if ((rv = getaddrinfo(NULL, port, &hints, &res)) == -1)
    {
        cerr << "getaddrinfo: " << gai_strerror(rv) << "\n";
        exit(1);
    }
    addrinfo *p = NULL;
    int yes = 1;
    for (p = res; p != NULL; p = p->ai_next)
    {
        if ((listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("socket");
            continue;
        }
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1); 
        }
        if (bind(listener, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("bind");
            close(listener);
            continue;
        }
        break;
    }
    char ipstr[INET6_ADDRSTRLEN];
    cout << "Server started on " << inet_ntop(p->ai_family, get_in_address((sockaddr *)p->ai_addr), ipstr, INET6_ADDRSTRLEN) << "::"; 
    cout << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << "\n";
    freeaddrinfo(res);
    if (p == NULL)
    {
        cerr << "server: fail to bind\n";
        exit(1);
    }
    if (listen(listener, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }
    pfds.emplace_back();
    pfds.back().fd = listener;
    pfds.back().events = POLLIN;
}
void Server::start()
{
    int newfd;

    sockaddr_storage remote_address;
    socklen_t addrlen;
    char ipstr[INET6_ADDRSTRLEN];
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
                    addrlen = sizeof remote_address;
                    newfd = accept(listener, (sockaddr *)&remote_address, &addrlen);
                    if (newfd == -1)
                    {
                        perror("accept");
                    } else
                    {
                        pfds.emplace_back();
                        pfds.back().fd = newfd;
                        pfds.back().events = POLLIN;

                        
                        cout << "New connection from " << inet_ntop(remote_address.ss_family, get_in_address((sockaddr*)&remote_address), ipstr, INET6_ADDRSTRLEN)
                             << " on socket " << newfd << "\n";
                    }
                } else
                {
                    int nbyte = recv(pfd.fd, buffer, sizeof buffer, 0);

                    // error or connection was closed
                    if (nbyte <= 0)
                    {
                        cout << "Close socket " << pfd.fd  << "\n";
                        close(pfd.fd);
                        pfds[i] = pfds.back();
                        pfds.pop_back();
                    } else
                    {
                        buffer[nbyte] = 0;
                        // receive message
                        
                        string msg(buffer);
                        while (iswspace(msg.back()))
                            msg.pop_back();
                        
                        string res;
                        list_file(msg, res);
                        send_all(pfd.fd, res, 0);
                    }
                }
            }
        }
    }
    
}
