#include "Client/Client.h"

int main(){
    Client* c = new Client();
    // c->listProcesss();
    // c->listApp();
    c->startApp("");
    delete c;
}