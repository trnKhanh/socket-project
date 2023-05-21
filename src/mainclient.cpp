#include "Client/Client.h"
#include <iostream>
#include "BasicUI/ShowUI.h"
int main()
{
    std::cout << "\033[2J\033[1;1H";
    Client c;
    showUI(c);
}