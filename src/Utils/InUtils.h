#pragma once
#include <string>

#ifdef _WIN32
    #include <WinSock2.h>
#elif __APPLE__
    #include <arpa/inet.h>
#enfif

void *getInAddress(sockaddr *addr);
std::string getIpStr(sockaddr *addr);