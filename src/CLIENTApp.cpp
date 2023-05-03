#include "Client/Client.h"

int main(){
    Client* c = new Client();
    // c->listApp();
    c->startApp("");
    // c->stopApp("");
    // c->listProcesss();
    // c->screenShot();
    // c-> dirTree();
    delete c;
}