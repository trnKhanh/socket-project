#pragma once
#include <vector>
#include <poll.h>
#include "../GlobalConstant.h"
#include "../Message/Request.h"
#include "../Message/Response.h"
#define BACKLOG 10

class Server {
private:
    int listener;
    int disfd;
    std::vector<pollfd> pfds;
public:
    Server();
    ~Server();

    void start();

    Response listApp();  
    Response startApp(const char *appName);
    Response stopApp(const char *appName);
    
    Response listProcesss();

    Response screenshot();

    Response startKeylog();
    Response stopKeylog();

    Response dirTree(const char *pathName); 
};