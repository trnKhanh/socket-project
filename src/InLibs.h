#pragma once
#ifdef __APPLE__
    #include <sys/types.h> 
    #include <sys/socket.h> 
    #include <arpa/inet.h> 
    #include <poll.h>
    #include <netdb.h>
    #include <unistd.h> 
    #include <netinet/in.h>
    #include <signal.h>
#elif _WIN32
    #include <winsock2.h>
    #include <windows.h>
    #include <ws2tcpip.h>
    #include "Utils/ConvertUtils.h"
    #define close closesocket
    #define poll WSAPoll
#endif

#include "GlobalConstant.h"
#include "Message/Request.h"
#include "Message/Response.h"
#include "Utils/InUtils.h"