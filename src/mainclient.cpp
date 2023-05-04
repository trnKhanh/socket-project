#include "client/Client.h"
#include <iostream>

int main()
{
    Client c;
    if (c.dirTree("."))
    {
        std::cout << "Request fail\n" << "\n";
    }
}