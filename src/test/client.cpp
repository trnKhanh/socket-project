#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <stdio.h>
#include "menu.h"

#pragma comment(lib, "Ws2_32.lib")

void listApp();
void listProcess(int iResult, SOCKET& connectSock);
void screenCap();
void keyLog();
void DFS();
void pause();

#define DEFAULT_PORT 8080

int main(int argc, char *argv[]) {
    SOCKET a;
    cout << a;
    return 0;
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }
    cout << "Initialize Winsock.\n";
    pause();

    // Create a TCP socket
    SOCKET connectSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSock == INVALID_SOCKET) {
        printf("socket failed: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    cout << "Create a TCP socket.\n";
    pause();

    // Connect to the server
    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP address
    serverAddr.sin_port = htons(DEFAULT_PORT); // Task Manager service port
    iResult = connect(connectSock, (SOCKADDR*) &serverAddr, sizeof(serverAddr));
    if (iResult == SOCKET_ERROR) {
        printf("connect failed: %d\n", WSAGetLastError());
        closesocket(connectSock);
        WSACleanup();
        return 1;
    }

    cout << "Connect to the server.\n";
    pause();

    // Send the request to the server
    while(true){
        int type = menu();
        bool iexit = false;
        switch(type){
            case 1: listApp(); break;
            case 2: listProcess(iResult, connectSock); break;
            case 3: screenCap(); break;
            case 4: keyLog(); break;
            case 5: DFS(); break;
            case 6: iexit = true; break;
        }
        if(iexit == true)
            break;
    }

    // Clean up
    closesocket(connectSock);
    WSACleanup();

    return 0;
}

void listApp(){
    return;
}

void listProcess(int iResult, SOCKET& connectSock){
    const char* request = "LISTPROCESSES";
    iResult = send(connectSock, request, strlen(request), 0);
    if (iResult == SOCKET_ERROR) {
        cout << "Send failed: " << WSAGetLastError() << '\n';
        closesocket(connectSock);
        WSACleanup();
        return;
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
    } 
    else if (iResult == 0) {
        // Server closed connection
        printf("Server disconnected\n");
    } 
    else {
        printf("recv failed: %d\n", WSAGetLastError());
    }
}

void screenCap(){
    return;
}

void keyLog(){
    return;
}

void DFS(){
    return;
}

void pause(){
    system("pause");
}
