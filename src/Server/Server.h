#pragma once

#include <string>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

#include "../Message/Response.h"

#define SERVER_PORT "6910"
#define BACKLOG 10

using std::vector;
using std::string;

class Server{
private:
    SOCKET disfd;
    SOCKET listener;
    vector <pollfd> pfds;
public:
    Server(const char* port);
    ~Server();

    void start();
    
    Response listApp();
    Response startApp(const char* appName);
    Response stopApp(const char* appName);

    Response listProcess();

    Response screenShot();

    Response keyLog();

    Response dirTree(const char* pathName); 
};