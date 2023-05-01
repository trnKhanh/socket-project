#pragma once
#include <WinSock2.h>
#include <string>

using std::string;

void *getInAddress(sockaddr *addr);
string getIpStr(sockaddr *addr);