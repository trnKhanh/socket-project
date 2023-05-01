#include "Client/Client.h"

int main(){
    Client* c = new Client();
    c->listProcesss();
    delete c;
}