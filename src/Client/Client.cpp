#include <WinSock2.h>
#include <cstring>
#include <iostream>
#include "Windows.h"

#include "Client.h"
#include "../Message/Request.h"
#include "../Message/Response.h"
#include "../Utils/InUtils.h"

using std::cerr;
using std::cout;
using std::cin;

Client::Client(){
    WSADATA wsaData;
    auto wVersionRequested = MAKEWORD(2, 2); // Get version of winsock
    int retCode = WSAStartup(wVersionRequested, &wsaData);

    if (retCode != 0)
        cout << "Startup failed: " << retCode << "\n";
        
    cout << "Return Code: " << retCode << "\n";
    cout << "Version Used: " << (int) LOBYTE(wsaData.wVersion) << "." << (int) HIBYTE(wsaData.wVersion) << "\n";
    cout << "Version Supported: " << (int) LOBYTE(wsaData.wHighVersion) << "." << (int) HIBYTE(wsaData.wHighVersion) << "\n";
    cout << "Implementation: " << wsaData.szDescription << "\n";
    cout << "System Status: " << wsaData.szSystemStatus << "\n";
    cout << "\n";

    if(LOBYTE(wsaData.wVersion) != LOBYTE(wVersionRequested) || HIBYTE(wsaData.wVersion) != HIBYTE(wVersionRequested)){
        cout << "Supported Version is too low.\n";
        WSACleanup();
        exit(0);
    }

    cout << "WSAStartup sucess.\n\n";

    vector <string> servers;
    if (this->discover(servers) == -1) 
        exit(1);
    
    cout << "Found following server:\n";
    for (auto server: servers)
        cout << server << "\n";

    cout << "Choose server to connect: ";
    string name = servers[0];
    cin >> name;

    addrinfo hints, *servinfo;
    int status, yes = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((status = getaddrinfo(name.c_str(), SERVER_PORT, &hints, &servinfo)) != 0){
        cerr << "client: getaddrinfo " << gai_strerror(status) << "\n";
        exit(1);
    }

    addrinfo *p = servinfo;
    for (;p != NULL; p = p->ai_next){
        if ((this->sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("client: socket");
            continue;
        }
        if (connect(this->sockfd, p->ai_addr, p->ai_addrlen) == -1){
            perror("client: connect");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);

    if (p == NULL){
        cerr << "Client: fail to connect\n";
        exit(1);
    }
}

Client::~Client(){
    closesocket(this->sockfd);
    WSACleanup();
}

int Client::discover(vector<string> &servers){
    int status;
    int disfd;
    int yes = 1;
    sockaddr_storage serverAddr;
    socklen_t addrlen = sizeof(serverAddr);
    addrinfo hints, *addr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, CLIENT_PORT, &hints, &addr)) != 0){
        cerr << "client: getaddrinfo: " << gai_strerror(status) << "\n";
        return -1;
    }

    addrinfo *p = addr;
    for (;p != NULL; p = p->ai_next){
        if ((disfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("client: socket");
            continue;
        }
        if (setsockopt(disfd, SOL_SOCKET, SO_BROADCAST, (char*)&yes, sizeof(yes)) == -1){
            perror("client: setsockopt");
            freeaddrinfo(addr);
            return -1;
        }
        if (setsockopt(disfd, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes)) == -1){
            perror("client: setsockopt");
            freeaddrinfo(addr);
            return -1;
        }
        if (bind(disfd, p->ai_addr, p->ai_addrlen) == -1){
            perror("client: bind");
            closesocket(disfd);
            continue;
        }
        break;
    }

    if (p == NULL){
        cerr << "client: fail to bind socket\n";
        freeaddrinfo(addr);
        return -1;
    }

    cout << "Broadcast discover message from " << getIpStr(p->ai_addr) << "::"; 
    cout << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << "\n";

    freeaddrinfo(addr);
    if ((status = getaddrinfo("255.255.255.255", SERVER_PORT, &hints, &addr)) != 0){
        cerr << "client: getaddrinfo: " << gai_strerror(status) << "\n";
        return -1;
    }
    p = addr;
    cout << "Broadcast discover message to " << getIpStr(p->ai_addr) << "::"; 
    cout << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << "\n";
    if (p == NULL){
        cerr << "client: broadcast fail\n";
        freeaddrinfo(addr);
        return -1;
    }

    Request msg(DISCOVER_REQUEST, 0, NULL);
    sendtoRequest(disfd, msg, 0, p->ai_addr, p->ai_addrlen);
    freeaddrinfo(addr);

    pollfd pfds[1];
    pfds[0].events = POLLIN;
    pfds[0].fd = disfd;
    while(true){
        int rv = WSAPoll(pfds, 1, 5000);
        if (rv == -1){
            closesocket(disfd); 
            perror("poll");
            return -1;
        }
        if (rv == 0) // time out
            break;
        
        Response buffer;
        recvfromResponse(disfd, buffer, 0, (sockaddr*)&serverAddr, &addrlen);
        if (buffer.type() == DISCOVER_RESPONSE)
            servers.push_back(getIpStr((sockaddr *)&serverAddr));
    }
    closesocket(disfd);
    return 0;
}

int Client::listApp(){
    return 0;
}

int Client::startApp(const char *appName){
    return 0;
}

int Client::stopApp(const char *appName){
    return 0;
}

int Client::listProcesss(){
    const char* request = "LISTPROCESSES";
    int iResult = send(sockfd, request, strlen(request), 0);
    if (iResult == SOCKET_ERROR) {
        cout << "Send failed: " << WSAGetLastError() << '\n';
        return -1;
    }

    // Receive the response from the server
    char recvbuf[512];
    iResult = recv(sockfd, recvbuf, sizeof(recvbuf), 0);
    if (iResult > 0) {
        // Print the process list to the console
        printf("Running processes:\n");
        printf("%.*s", iResult, recvbuf);
        while (iResult = recv(sockfd, recvbuf, sizeof(recvbuf), 0)) {
            printf("%.*s", iResult, recvbuf);
        }
    } 
    else if (iResult == 0) {
        // Server closed connection
        cout << "Server disconnected\n";
    }
    else {
        cout << "Recv failed: " << WSAGetLastError() << '\n';
    }
    return 0;
}

int Client::screenshot(){
    return 0;
}

int Client::startKeylog(){
    return 0;
}

int Client::stopKeylog(){
    return 0;
}

int Client::dirTree(){
    return 0;
}