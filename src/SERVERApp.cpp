#include "Server/Server.h"

int main(){
    Server* s = new Server(SERVER_PORT);
    s->start();
    delete s;
}