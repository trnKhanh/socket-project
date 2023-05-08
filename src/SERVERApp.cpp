#include "Server/Server.h"

#include "GlobalConstant.h"

int main(){
    Server* s = new Server(SERVER_PORT);
    s->start();
    delete s;
}