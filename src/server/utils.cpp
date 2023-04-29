#include "utils.h"
#include <filesystem>
#include <sstream>

void *get_in_address(sockaddr *addr)
{
    if (addr->sa_family == AF_INET)
        return &(((sockaddr_in*)addr)->sin_addr);
    else 
        return &(((sockaddr_in6*)addr)->sin6_addr);
}

void list_file(string path_name, string &res)
{
    if (!filesystem::exists(path_name))
    {
        res = "";
        return;
    }
    filesystem::path dir(path_name);
    ostringstream os;
    for (auto const& dir_entry: filesystem::directory_iterator(dir))
    {
        os << dir_entry.path() << "\n";
    }
    res = os.str();
}

void send_all(int &sockfd, const string &buffer, int flag)
{
    int sent = 0;
    int len = buffer.size();
    while (1)
    {
        int byte_sent = send(sockfd, &buffer[sent], len - sent, flag);

        if (byte_sent <= 0)
            return;

        sent += byte_sent;

        if (sent == len)
            return;
    }
}
void recv_all(int &sockfd, string &res, int flag)
{
    int recieved = 0;
    while (1)
    {
        char buffer[256];
        int byte_recieved = recv(sockfd, buffer, sizeof buffer, flag);

        if (byte_recieved <= 0)
            break;

        recieved += byte_recieved;
        res.append(buffer);
    }
}