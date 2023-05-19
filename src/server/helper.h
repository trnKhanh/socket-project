#pragma once
#ifdef __APPLE__
    #include "function_unix/ListProcesses.h"
    #include "function_unix/AppCMD.h"
    #include "function_unix/Screenshot.h"
    #include "function_unix/Keylog.h"
#elif __WIN32__
    #include "function_window/ListProcesses.h"
    #include "function_window/AppCMD.h"
    #include "function_window/Screenshot.h"
    #include "function_window/Keylog.h"
#endif

#include "function_cross/ListDirTree.h"