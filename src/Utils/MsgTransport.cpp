#include "MsgTransport.h"
#include <sys/socket.h>

int sendAll(int sockfd, const void *msg, size_t len, int flag)
{
    int totalByteSent = 0;

    while (totalByteSent < len)
    {
        int byteSent = send(sockfd, msg + totalByteSent, len - totalByteSent, flag);

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
        int byteRecv = recv(sockfd, msg + totalByteRecv, len - totalByteRecv, flag);

        if (byteRecv <= 0)
            return -1;

        totalByteRecv += byteRecv;
    }

    return 0;
}