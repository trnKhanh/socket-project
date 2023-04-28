#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char *argv[]) {
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    // Create a TCP socket
    SOCKET connectSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSock == INVALID_SOCKET) {
        printf("socket failed: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Connect to the server
    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP address
    serverAddr.sin_port = htons(5555); // Task Manager service port
    iResult = connect(connectSock, (SOCKADDR*) &serverAddr, sizeof(serverAddr));
    if (iResult == SOCKET_ERROR) {
        printf("connect failed: %d\n", WSAGetLastError());
        closesocket(connectSock);
        WSACleanup();
        return 1;
    }

    // Send the request to the server
    const char* request = "LISTPROCESSES";
    iResult = send(connectSock, request, strlen(request), 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(connectSock);
        WSACleanup();
        return 1;
    }

    // Receive the response from the server
    char recvbuf[512];
    iResult = recv(connectSock, recvbuf, sizeof(recvbuf), 0);
    if (iResult > 0) {
        // Print the process list to the console
        printf("Running processes:\n");
        printf("%.*s", iResult, recvbuf);
        while (iResult = recv(connectSock, recvbuf, sizeof(recvbuf), 0)) {
            printf("%.*s", iResult, recvbuf);
        }
    } else if (iResult == 0) {
        // Server closed connection
        printf("Server disconnected\n");
    } else {
        printf("recv failed: %d\n", WSAGetLastError());
    }

    // Clean up
    closesocket(connectSock);
    WSACleanup();

    return 0;
}
