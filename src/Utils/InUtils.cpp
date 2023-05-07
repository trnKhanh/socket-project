#include "InUtils.h"

#ifdef _WIN32
    #include <ws2ipdef.h>
    #include <WS2tcpip.h>
#endif

void *getInAddress(sockaddr *addr){
    //IPv4
    if (addr->sa_family == AF_INET)
        return &(((sockaddr_in*)addr)->sin_addr);
    //IPv6
    return &(((sockaddr_in6*)addr)->sin6_addr);
}

std::string getIpStr(sockaddr *addr){
    char buffer[INET6_ADDRSTRLEN];
    inet_ntop(addr->sa_family, getInAddress(addr), buffer, sizeof(buffer));
    std::string res(buffer);
    return res;
}