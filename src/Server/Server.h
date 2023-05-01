#pragma once
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

#define SERVER_PORT "6910"
#define BACKLOG 10

using std::vector;
using std::cout;
using std::cin;
using std::cerr;

class Server{
private:
    int disfd;
    int listener;
    vector <pollfd> pfds;
public:
    Server(const char* port);
    ~Server();

    void start();
};
