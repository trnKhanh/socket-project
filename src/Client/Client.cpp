#include "Client.h"

#include <WinSock2.h>
#include <Windows.h>
#include <cstring>
#include <iostream>
#include <fstream>

#include "../Message/Request.h"
#include "../Message/Response.h"
#include "../Utils/InUtils.h"
#include "../Utils/ConvertUtils.h"

using std::cerr;
using std::cout;
using std::cin;
using std::ios;
using std::ofstream;

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
    retCode = this->discover(servers);
    
    if (retCode == -1) 
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
        cerr << "Client: getaddrinfo " << gai_strerror(status) << "\n";
        exit(1);
    }

    addrinfo *p = servinfo;
    for (;p != NULL; p = p->ai_next){
        this->sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (this->sockfd == INVALID_SOCKET){
            cerr << "Client: socket\n";
            continue;
        }
        int status = connect(this->sockfd, p->ai_addr, p->ai_addrlen);
        if (status == SOCKET_ERROR){
            cerr << "Client: connect\n";
            closesocket(this->sockfd);
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);

    if (p == NULL){
        cerr << "Client: fail to connect\n";
        exit(1);
    }
    cout << "Connected to server.\n";
}

Client::~Client(){
    closesocket(this->sockfd);
    WSACleanup();
}

int Client::discover(vector<string> &servers){
    int status;
    SOCKET disfd;
    int yes = 1;
    sockaddr_storage serverAddr;
    socklen_t addrlen = sizeof(serverAddr);
    addrinfo hints, *addr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    status = getaddrinfo(NULL, CLIENT_PORT, &hints, &addr);
    if (status != 0){
        cerr << "Client: getaddrinfo: " << gai_strerror(status) << "\n";
        return -1;
    }

    addrinfo *p = addr;
    for (;p != NULL; p = p->ai_next){
        disfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (disfd == INVALID_SOCKET){
            cerr << "Client: socket\n";
            continue;
        }

        int status = setsockopt(disfd, SOL_SOCKET, SO_BROADCAST, (char*)&yes, sizeof(yes));
        if (status == SOCKET_ERROR){
            cerr << "Client: setsockopt\n";
            freeaddrinfo(addr);
            return -1;
        }

        status = setsockopt(disfd, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes));
        if (status == SOCKET_ERROR){
            cerr << "Client: setsockopt\n";
            freeaddrinfo(addr);
            closesocket(disfd);
            return -1;
        }

        status = bind(disfd, p->ai_addr, p->ai_addrlen);
        if (status == SOCKET_ERROR){
            cerr << "Client: bind\n";
            closesocket(disfd);
            continue;
        }
        break;
    }

    if (p == NULL){
        cerr << "Client: Fail to bind socket\n";
        freeaddrinfo(addr);
        return -1;
    }

    cout << "Broadcast discover message from " << getIpStr(p->ai_addr) << "::"; 
    cout << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << "\n";

    freeaddrinfo(addr);
    status = getaddrinfo("255.255.255.255", SERVER_PORT, &hints, &addr);
    if (status != 0){
        cerr << "Client: getaddrinfo: " << gai_strerror(status) << "\n";
        return -1;
    }
    p = addr;
    cout << "Broadcast discover message to " << getIpStr(p->ai_addr) << "::"; 
    cout << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << "\n";
    if (p == NULL){
        cerr << "Client: broadcast fail\n";
        freeaddrinfo(addr);
        return -1;
    }

    Request msg(DISCOVER_REQUEST, 0, NULL);
    sendtoRequest(disfd, msg, 0, p->ai_addr, p->ai_addrlen);
    freeaddrinfo(addr);

    WSAPOLLFD pfds[1];
    pfds[0].events = POLLIN;
    pfds[0].fd = disfd;
    while(true){
        int rv = WSAPoll(pfds, 1, 1000);
        if (rv == -1){
            cerr << "poll\n";
            closesocket(disfd); 
            return -1;
        }
        if (rv == 0){ // time out
            cout << "Time out...\n";
            break;
        }
        
        Response buffer;
        addrlen = sizeof(serverAddr);
        int retCode = recvfromResponse(disfd, buffer, 0, (sockaddr*)&serverAddr, &addrlen);
        if(retCode == -1)
            continue;
        if (buffer.type() == DISCOVER_RESPONSE)
            servers.push_back(getIpStr((sockaddr*) &serverAddr));
    }
    closesocket(disfd);
    return 0;
}

int Client::listApp(){
    Request requestToServer(LIST_APP_REQUEST, 0, NULL);
    int status = sendRequest(this->sockfd, requestToServer, 0);
    if(status == SOCKET_ERROR)
        return SOCKET_ERROR;

    Response responseFromServer;
    status = recvResponse(this->sockfd, responseFromServer, 0);
    if(status == SOCKET_ERROR)
        return SOCKET_ERROR;

    if(responseFromServer.errCode() == FAIL_CODE)
        return SOCKET_ERROR;

    cout << (char*) responseFromServer.data() << '\n';
    return 0;
}

int Client::startApp(const char *appName){
    Request requestToServer(START_APP_REQUEST, strlen(appName) + 1, (void*)appName);
    int status = sendRequest(this->sockfd, requestToServer, 0);
    if(status == SOCKET_ERROR)
        return SOCKET_ERROR;

    Response responseFromServer;
    status = recvResponse(this->sockfd, responseFromServer, 0);
    if(status == SOCKET_ERROR)
        return SOCKET_ERROR;

    if(responseFromServer.errCode() == FAIL_CODE){
        cout << "Client: Can't start " << appName << ".\n";
        return -1;
    }

    cout << "Client: Started " << appName << ".\n";
    return 0;
}

int Client::stopApp(const char *appName){
    Request requestToServer(STOP_APP_REQUEST, strlen(appName) + 1, (void*)appName);
    int status = sendRequest(this->sockfd, requestToServer, 0);
    if(status == SOCKET_ERROR)
        return SOCKET_ERROR;

    Response responseFromServer;
    status = recvResponse(this->sockfd, responseFromServer, 0);
    if(status == SOCKET_ERROR)
        return SOCKET_ERROR;

    if(responseFromServer.errCode() == FAIL_CODE){
        cout << "Can't stop " << appName << ".\n";
        return -1;
    }

    cout << "Client: Stopped " << appName << ".\n";
    return 0;
}

int Client::listProcesss(){
    Request requestToServer(LIST_PROCESS_REQUEST, 0, NULL);
    int status = sendRequest(this->sockfd, requestToServer, 0);
    if(status == SOCKET_ERROR)
        return SOCKET_ERROR;

    Response responseFromServer;
    status = recvResponse(this->sockfd, responseFromServer, 0);
    if(status == SOCKET_ERROR)
        return SOCKET_ERROR;

    if(responseFromServer.errCode() == FAIL_CODE)
        return -1;

    cout << (char*) responseFromServer.data() << '\n';
    return 0;
}

int Client::screenShot(){
    Request requestToServer(SCREENSHOT_REQUEST, 0, NULL);
    int status = sendRequest(this->sockfd, requestToServer, 0);
    if(status == SOCKET_ERROR)
        return SOCKET_ERROR;

    Response responseFromServer;
    status = recvResponse(this->sockfd, responseFromServer, 0);
    if(status == SOCKET_ERROR)
        return SOCKET_ERROR;

    if(responseFromServer.errCode() == FAIL_CODE)
        return -1;

    ofstream fo("screenShotFromServer.png", ios::binary);
    if(fo.fail()){
        cerr << "Client: Can't create file screenshot png.\n";
        return -1;
    }
    fo.write((char*)responseFromServer.data(), responseFromServer.length());
    fo.close();
    cout << "Client: Created screenShotFromServer.png from installed data.\n";
    return 0;
}

int Client::startKeylog(){
    return 0;
}

int Client::stopKeylog(){
    return 0;
}

int Client::dirTree(){
    Request requestToServer(DIR_TREE_REQUEST, 0, NULL);
    int status = sendRequest(this->sockfd, requestToServer, 0);
    if(status == SOCKET_ERROR)
        return SOCKET_ERROR;

    Response responseFromServer;
    status = recvResponse(this->sockfd, responseFromServer, 0);
    if(status == SOCKET_ERROR)
        return SOCKET_ERROR;

    if(responseFromServer.errCode() == FAIL_CODE)
        return -1;

    cout << (char*) responseFromServer.data() << '\n';
    return 0;
}

int Client::disconnect(){
    Request requestToServer(DISCONNECT_REQUEST, 0, NULL);
    int status = sendRequest(this->sockfd, requestToServer, 0);
    if(status == SOCKET_ERROR)
        return SOCKET_ERROR;

    Response responseFromServer;
    status = recvResponse(this->sockfd, responseFromServer, 0);
    if(status == SOCKET_ERROR)
        return SOCKET_ERROR;

    if(responseFromServer.errCode() == FAIL_CODE)
        return -1;

    cout << "Client: Disconnecting...\n";
    return 0;
}