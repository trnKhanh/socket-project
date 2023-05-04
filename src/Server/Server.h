#pragma once
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

#define SERVER_PORT "6910"
#define BACKLOG 10

using std::vector;

class Server{
private:
    SOCKET disfd;
    SOCKET listener;
    vector <WSAPOLLFD> pfds;
public:
    Server(const char* port);
    ~Server();

    void start();
    
    int listApp(SOCKET& fd);
    int startApp(const char* appName, SOCKET& fd);
    int stopApp(const char* appName, SOCKET& fd);

    int listProcess(SOCKET& fd);

    int screenShot(SOCKET& fd);

    int keyLog(SOCKET& fd);

    int dirTree(SOCKET& fd); 
};

DWORD GetProcessIdByName(const char* processName);
void printDirectoryTree(const char* path, int indent);