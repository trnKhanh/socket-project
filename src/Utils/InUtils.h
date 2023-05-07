#pragma once

#include <string>

#ifdef _WIN32
    #include <WinSock2.h>
#endif


void *getInAddress(sockaddr *addr);
std::string getIpStr(sockaddr *addr);