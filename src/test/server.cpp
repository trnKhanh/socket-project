#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <psapi.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Psapi.lib")

#define DEFAULT_PORT 8080

using std::cout;

int listProcess(int iResult, SOCKET& listenSock);
void itos(char* res, int n){
    int i = 0;
    while(n){
        res[i++] = char(n % 10 + '0');
        n /= 10;
    }
    res[i] = '\0';
}

void pause();

int main(int argc, char *argv[]) {
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        cout << "WSAStartup failed: " << iResult << '\n';
        return 1;
    }

    cout << "WSAStartup sucess.\n";
    pause();

    // Create a TCP socket
    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock == INVALID_SOCKET) {
        cout << "socket failed: " << WSAGetLastError() << '\n';
        WSACleanup();
        return 1;
    }
    cout << "Create Socket.\n";
    pause();

    // Bind the socket to the Task Manager service port
    SOCKADDR_IN service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(DEFAULT_PORT);
    iResult = bind(listenSock, (SOCKADDR*) &service, sizeof(service));
    if (iResult == SOCKET_ERROR) {
        cout << "Bind failed: " << WSAGetLastError() << '\n';
        closesocket(listenSock);
        WSACleanup();
        return 1;
    }
    cout << "Bind the socket to port: " << DEFAULT_PORT << ".\n";
    pause();

    listProcess(iResult, listenSock);

    // Clean up
    closesocket(listenSock);
    WSACleanup();

    return 0;
}

int listProcess(int iResult, SOCKET& listenSock){

    // Listen for incoming connections
    iResult = listen(listenSock, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        cout << "listen failed: " << WSAGetLastError() << '\n';
        closesocket(listenSock);
        WSACleanup();
        return 1;
    }

    cout << "Server running...\n";

    // Accept incoming connections and handle requests
    while (true) {
        SOCKET clientSock = accept(listenSock, NULL, NULL);

        if (clientSock == INVALID_SOCKET) {
            cout << "accept failed: " << WSAGetLastError() << '\n';
            continue;
        }

        cout << "Client connected\n";

        // Receive the request from the client
        while(true){
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
                        char numProc[20];
                        itos(numProc, cProcesses);
                        cout << cProcesses << '\n';
                        //send(clientSock, numProc, strlen(numProc), 0);
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
                }
                else if (strncmp(recvbuf, "DISCONNECT", 10) == 0)
                    break;   
                else {
                    // Unknown request
                    const char* response = "UNKNOWN REQUEST\n";
                    send(clientSock, response, strlen(response), 0);
                }
            }
            else if (iResult == 0) {
                // Client closed connection
                break;
            } 
            else{
                cout << "rev failed: " << WSAGetLastError() << "\n";
                break;
            }
            pause();
        }
        // Clean up
        cout << "Client disconnect.\n";
        closesocket(clientSock);
    }
}

void pause(){
    system("pause");
}