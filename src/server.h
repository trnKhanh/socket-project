#pragma once
#include <vector>
#include <poll.h>

#define SERVER_PORT "6910"
#define BACKLOG 10

using namespace std;

class Server {
private:
    int listener;
    vector<pollfd> pfds;
public:
    Server(const char* port);
    ~Server();

    void start();
};