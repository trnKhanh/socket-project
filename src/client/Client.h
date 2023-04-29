#pragma once
#include <vector>
#include <arpa/inet.h>
#include <string>

#define SERVER_PORT "6910"
#define CLIENT_PORT "6911"

class Client {
    int sockfd;
public:
    Client();
    ~Client();

    int discover(std::vector<std::string> &servers);

    int listApp();  
    int startApp(const char *appName);
    int stopApp(const char *appName);
    
    int listProcesss();

    int screenshot();

    int startKeylog();
    int stopKeylog();

    int dirTree(); 
};