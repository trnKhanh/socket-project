#ifdef __APPLE__

#include "Screenshot.h"
#include <stdlib.h>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <iostream>

int screenshotHelper(std::vector<char> &buffer)
{
    if (system("screencapture screenshot.png"))
    {
        return -1;
    }
    std::ifstream is;
    is.open("screenshot.png", std::ios::binary);
    if (!is)
    {
        if (remove("screenshot.png"))
        {
            return -1;
        }
    }
    buffer = std::vector<char>(std::istreambuf_iterator<char>(is), {});
    is.close();
    if (remove("screenshot.png"))
    {
        return -1;
    }
    return 0;
}

#endif