#include "MsgTransport.h"

int sendAll(int sockfd, const void *msg, size_t len, int flag)
{
    int totalByteSent = 0;

    while (totalByteSent < len)
    {
        int byteSent = send(sockfd, (char*)msg + totalByteSent, len - totalByteSent, flag);

        if (byteSent <= 0)
            return -1;

        totalByteSent += byteSent;
    }
    
    return 0;
}
int recvAll(int sockfd, void *msg, size_t len, int flag)
{
    int totalByteRecv = 0;
    while (totalByteRecv < len)
    {
        int byteRecv = recv(sockfd, (char*)msg + totalByteRecv, len - totalByteRecv, flag);
        if (byteRecv <= 0)
            return -1;

        totalByteRecv += byteRecv;
    }

    return 0;
}

int sendtoAll(int sockfd, const void *msg, size_t len, int flag, const sockaddr* addr, socklen_t addrlen)
{
    int totalByteSent = 0;

    while (totalByteSent < len)
    {
        int byteSent = sendto(sockfd, (char*)msg + totalByteSent, len - totalByteSent, flag, addr, addrlen);

        if (byteSent <= 0)
            return -1;

        totalByteSent += byteSent;
    }
    return 0;
}
int recvfromAll(int sockfd, void *msg, size_t len, int flag, sockaddr* addr, socklen_t *addrlen)
{
    int totalByteRecv = 0;
    while (totalByteRecv < len)
    {
        int byteRecv = recvfrom(sockfd, (char*)msg + totalByteRecv, len - totalByteRecv, flag, addr, addrlen);

        if (byteRecv <= 0)
            return -1;

        totalByteRecv += byteRecv;
    }

    return 0;
}