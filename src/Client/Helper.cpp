#include "Helper.h"

#ifdef __APPLE__
#elif _WIN32
    int close(int fd) {return closesocket(fd);}
#endif