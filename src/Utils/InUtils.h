#pragma once
#include "../InLibs.h"
#include <string>

void *getInAddress(sockaddr *addr);
std::string getIpStr(sockaddr *addr);