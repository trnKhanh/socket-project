#pragma once
#include <vector>
#include <poll.h>
#include "../GlobalConstant.h"
#include "../Message/Request.h"
#include "../Message/Response.h"
#include <thread>
#define BACKLOG 10

class Server {
private:
    int listener;
    int disfd;
    std::vector<pollfd> pfds;
    std::thread keylogThread;
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