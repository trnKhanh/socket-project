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
    #define closesocket close
    typedef int* sockopt_type;
#elif _WIN32
    #define WIN32_LEAN_AND_MEAN 

    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include "Utils/ConvertUtils.h"
    #define poll WSAPoll

    typedef char* sockopt_type;
#endif

#include "GlobalConstant.h"
#include "Message/Request.h"
#include "Message/Response.h"
#include "Utils/InUtils.h"