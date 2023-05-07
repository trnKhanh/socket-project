#pragma once
#include <vector>
#include <string>

#include "WinSock2.h"
#include "../GlobalConstant.h"

using std::vector;
using std::string;

class Client {
    SOCKET sockfd;
public:
    Client();
    ~Client();

    int discover(vector <string> &servers); // find all server can connect

    int listApp();  
    int startApp(const char *appName);
    int stopApp(const char *appName);
    
    int listProcesss();

    int screenShot();

    int keyLog();

    int dirTree(const char* pathName);
};