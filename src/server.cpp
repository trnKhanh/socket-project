#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <psapi.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Psapi.lib")

using std::cout;

void pause();

int main(int argc, char *argv[]) {
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    cout << "WSAStartup sucess.\n";
    pause();

    // Create a TCP socket
    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock == INVALID_SOCKET) {
        printf("socket failed: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    cout << "Create Socket.\n";
    pause();

    // Bind the socket to the Task Manager service port
    SOCKADDR_IN service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(5555); // Task Manager service port
    iResult = bind(listenSock, (SOCKADDR*) &service, sizeof(service));
    if (iResult == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        closesocket(listenSock);
        WSACleanup();
        return 1;
    }
    cout << "Bind the socket to the Task Manager service port (5555).\n";
    pause();

    // Listen for incoming connections
    iResult = listen(listenSock, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed: %d\n", WSAGetLastError());
        closesocket(listenSock);
        WSACleanup();
        return 1;
    }

    cout << "Server running...\n";

    // Accept incoming connections and handle requests
    while (true) {
        SOCKET clientSock = accept(listenSock, NULL, NULL);
        if (clientSock == INVALID_SOCKET) {
            printf("accept failed: %d\n", WSAGetLastError());
            continue;
        }

        printf("Client connected\n");

        // Receive the request from the client
        char recvbuf[512];
        iResult = recv(clientSock, recvbuf, sizeof(recvbuf), 0);
        if (iResult > 0) {
            // Process the request and send the response
            if (strncmp(recvbuf, "LISTPROCESSES", 13) == 0) {
                DWORD aProcesses[1024], cbNeeded, cProcesses;
                if (EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
                    // Calculate how many process identifiers were returned
                    cProcesses = cbNeeded / sizeof(DWORD);
                    // Get the process names and send them to the client
                    for (DWORD i = 0; i < cProcesses; i++) {
                        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
                        if (hProcess != NULL) {
                            char procName[MAX_PATH];
                            if (GetModuleBaseNameA(hProcess, NULL, procName, sizeof(procName)) > 0) {
                                send(clientSock, procName, strlen(procName), 0);
                                send(clientSock, "\n", 1, 0);
                            }
                            CloseHandle(hProcess);
                        }
                    }
                }
            } else {
                // Unknown request
                const char* response = "UNKNOWN REQUEST\n";
                send(clientSock, response, strlen(response), 0);
            }
        } else if (iResult == 0) {
            // Client closed connection
            printf("Client disconnected\n");
        } else {
            printf("recv failed: %d\n", WSAGetLastError());
        }

        // Clean up
        closesocket(clientSock);
    }

    // Clean up
    closesocket(listenSock);
    WSACleanup();

    return 0;
}

void pause(){
    system("pause");
}