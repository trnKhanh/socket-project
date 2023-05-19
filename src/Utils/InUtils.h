#pragma once
#ifdef __APPLE__
    #include <arpa/inet.h>
#elif _WIN32
    #include <winsock2.h>
    #include <WS2tcpip.h>
#endif
#include <string>

void *getInAddress(sockaddr *addr);
std::string getIpStr(sockaddr *addr);