#include "Server/Server.h"
#include "GlobalConstant.h"
#include <iostream>
int main(int argc, char *argv[])
{
    Server s;
    // std::cout << (char*)s.listProcesss().data() << std::endl;
    s.start();
}