#pragma once
#include <vector>
#include <poll.h>

#define SERVER_PORT "6910"
#define CLIENT_PORT "6911"
#define BACKLOG 10

using namespace std;

class Server {
private:
    int listener;
    int disfd;
    vector<pollfd> pfds;
public:
    Server(const char* port);
    ~Server();

    void start();
};