#pragma once
#include <vector>
#include <arpa/inet.h>
#include <string>
#include "../GlobalConstant.h"
#include <thread>
#include <fstream>

class Client {
    std::ofstream _keylogFile;
    int sockfd;
    int _keylogfd;
    std::string _serverName;
    std::thread _keylogThread;
    void recvKeylog();
public:
    Client();
    ~Client();

    int discover(std::vector<std::string> &servers);

    int listApp();  
    int startApp(const std::string &appName);
    int stopApp(const std::string &appName);
    
    int listProcesss();

    int screenshot();

    int startKeylog();
    int stopKeylog();

    int dirTree(const std::string pathName); 
};