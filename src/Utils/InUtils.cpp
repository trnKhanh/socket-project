#include "InUtils.h"

void *getInAddress(sockaddr *addr)
{
    if (addr->sa_family == AF_INET)
        return &(((sockaddr_in*)addr)->sin_addr);
    else 
        return &(((sockaddr_in6*)addr)->sin6_addr);
}
std::string getIpStr(sockaddr *addr)
{
    char buffer[INET6_ADDRSTRLEN];
    inet_ntop(addr->sa_family, getInAddress(addr), buffer, sizeof(buffer));
    std::string res(buffer);
    return res;
}