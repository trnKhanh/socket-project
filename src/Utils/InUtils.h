#pragma once
#include <arpa/inet.h>
#include <string>

void *getInAddress(sockaddr *addr);
std::string getIpStr(sockaddr *addr);