#include "Client.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <future>
#include <chrono>
#include <sstream>
#include <filesystem>

Client::Client()
{
    #ifdef _WIN32
        WSADATA wsaData;
        auto wVersionRequested = MAKEWORD(2, 2); // Get version of winsock
        int retCode = WSAStartup(wVersionRequested, &wsaData);

        if (retCode != 0)
            std::cout << "Startup failed: " << retCode << std::endl;
            
        std::cout << "Return Code: " << retCode << std::endl;
        std::cout << "Version Used: " << (int) LOBYTE(wsaData.wVersion) << "." << (int) HIBYTE(wsaData.wVersion) << std::endl;
        std::cout << "Version Supported: " << (int) LOBYTE(wsaData.wHighVersion) << "." << (int) HIBYTE(wsaData.wHighVersion) << std::endl;
        std::cout << "Implementation: " << wsaData.szDescription << std::endl;
        std::cout << "System Status: " << wsaData.szSystemStatus << std::endl;
        std::cout << std::endl;

        if(LOBYTE(wsaData.wVersion) != LOBYTE(wVersionRequested) || HIBYTE(wsaData.wVersion) != HIBYTE(wVersionRequested)){
            std::cout << "Supported Version is too low.\n";
            WSACleanup();
            exit(0);
        }

        std::cout << "WSAStartup sucess.\n\n";
    #endif
    int choice;
    std::vector<std::string> servers;
    do{
        if (this->discover(servers) == -1) 
            exit(1);
        if (servers.empty())
        {
            std::cout << "Found no server\n";
            exit(1);
        }
        std::cout << "Found following server:\n";
        for (int i = 0; i < servers.size(); ++i)
        {
            std::cout << i + 1 << ". " << servers[i] << std::endl;
        }
        std::cout << servers.size() + 1 << ". Retry\n";
        std::cout << servers.size() + 2 << ". Exit\n";
        while(1)
        {
            try{
                std::string buffer;
                std::cout << "Choose server to connect (1" << '-' << servers.size() + 2 << "): ";
                std::getline(std::cin, buffer);
                choice = std::stoi(buffer);
            }
            catch(...)
            {
                choice = 0;
            }
            if (choice < 1 || choice > servers.size() + 2) 
            {
                std::cerr << "Error: Invalid selection.\n";
            } else break;
        }
        std::cout << "\033[2J\033[1;1H";
    } while (choice == servers.size() + 1);
    if (choice == servers.size() + 2)
        exit(0);
    std::string name = servers[choice - 1];
    // std::cin >> name;
    this->_serverName = name;
    addrinfo hints, *servinfo;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((status = getaddrinfo(this->_serverName.c_str(), SERVER_PORT, &hints, &servinfo)) != 0)
    {
        std::cerr << "client: getaddrinfo " << gai_strerror(status) << std::endl;
        exit(1);
    }

    addrinfo *p = servinfo;
    for (;p != NULL; p = p->ai_next)
    {
        if ((this->sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }
        if (connect(this->sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("client: connect");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);
    if (p == NULL)
    {
        std::cerr << "client: fail to connect\n";
        exit(1);
    }
}
Client::~Client()
{
    if (this->_keylogThread.joinable())
    {
        if (this->stopKeylog())
            this->_keylogThread.join();
    }
    closesocket(this->sockfd);
    #ifdef _WIN32
        WSACleanup();
    #endif
}

int Client::discover(std::vector<std::string> &servers)
{
    servers.clear();
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

    if ((status = getaddrinfo(NULL, "0", &hints, &addr)) != 0)
    {
        std::cerr << "client: getaddrinfo: " << gai_strerror(status) << std::endl;
        return -1;
    }

    addrinfo *p = addr;
    for (;p != NULL; p = p->ai_next)
    {
        if ((disfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }
        if (setsockopt(disfd, SOL_SOCKET, SO_BROADCAST, (sockopt_type)&yes, sizeof(yes)) == -1)
        {
            perror("client: setsockopt");
            freeaddrinfo(addr);
            return -1;
        }
        if (setsockopt(disfd, SOL_SOCKET, SO_REUSEADDR, (sockopt_type)&yes, sizeof(yes)) == -1)
        {
            perror("client: setsockopt");
            freeaddrinfo(addr);
            return -1;
        }
        if (bind(disfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("client: bind");
            closesocket(disfd);
            continue;
        }
        break;
    }

    if (p == NULL)
    {
        std::cerr << "client: fail to bind socket\n";
        freeaddrinfo(addr);
        return -1;
    }

    std::cout << "Broadcast discover message from " << getIpStr(p->ai_addr) << "::"; 
    std::cout << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << std::endl;

    freeaddrinfo(addr);
    if ((status = getaddrinfo("255.255.255.255", SERVER_PORT, &hints, &addr)) != 0)
    {
        std::cerr << "client: getaddrinfo: " << gai_strerror(status) << std::endl;
        return -1;
    }
    p = addr;
    std::cout << "Broadcast discover message to " << getIpStr(p->ai_addr) << "::"; 
    std::cout << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << std::endl;
    if (p == NULL)
    {
        std::cerr << "client: broadcast fail\n";
        freeaddrinfo(addr);
        return -1;
    }

    Request msg(DISCOVER_REQUEST, 0, NULL);
    sendtoRequest(disfd, msg, 0, p->ai_addr, p->ai_addrlen);
    freeaddrinfo(addr);

    pollfd pfds[1];
    pfds[0].events = POLLIN;
    pfds[0].fd = disfd;
    while(1)
    {
        int rv = poll(pfds, 1, 1000);
        if (rv == -1)
        {
            closesocket(disfd); 
            perror("poll");
            return -1;
        }
        if (rv == 0) // time out
            break;
        
        Response buffer;
        addrlen = sizeof(serverAddr);
        recvfromResponse(disfd, buffer, 0, (sockaddr*)&serverAddr, &addrlen);
        if (buffer.type() == DISCOVER_RESPONSE)
            servers.push_back(getIpStr((sockaddr *)&serverAddr));
    }
    closesocket(disfd);
    return 0;
}

void Client::recvKeylog()
{
    pollfd fds[1];
    fds[0].fd = this->_keylogfd;
    fds[0].events = POLLIN;
    while (1)
    {
        int rv = poll(fds, 1, -1);
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
            closesocket(this->_keylogfd);
            this->_keylogFile.close();
            break;
        }
        this->_keylogFile << (char*) res.data() << std::flush;
    }
}
int Client::listApp(std::string &result)
{
    Request req(LIST_APP_REQUEST, 0, NULL);
    if (sendRequest(this->sockfd, req, 0))
        return -1;
    Response res;
    if (recvResponse(this->sockfd, res, 0))
        return -1;
    if (res.errCode() == FAIL_CODE || res.type() != CMD_RESPONSE_STR)
        return -1;
    result = std::string((char*)res.data());
    return 0;
}
int Client::startApp(const std::string &appName)
{
    Request req(START_APP_REQUEST, appName.size() + 1, (void *)appName.c_str());
    if (sendRequest(this->sockfd, req, 0))
        return -1;
    Response res;
    if (recvResponse(this->sockfd, res, 0))
        return -1;
    if (res.errCode() == FAIL_CODE || res.type() != CMD_RESPONSE_EMPTY)
        return -1;
    return 0;
}
int Client::stopApp(const std::string &appName)
{
    Request req(STOP_APP_REQUEST, appName.size() + 1, (void *)appName.c_str());
    if (sendRequest(this->sockfd, req, 0))
        return -1;
    Response res;
    if (recvResponse(this->sockfd, res, 0))
        return -1;
    if (res.errCode() == FAIL_CODE || res.type() != CMD_RESPONSE_EMPTY)
        return -1;
    return 0;
}

int Client::listProcesss(std::string &result)
{
    Request req(LIST_PROC_REQUEST, 0, NULL);
    if (sendRequest(this->sockfd, req, 0))
        return -1;
    Response res;
    if (recvResponse(this->sockfd, res, 0))
        return -1;
    if (res.errCode() == FAIL_CODE || res.type() != CMD_RESPONSE_STR)
        return -1;
    result = std::string((char*)res.data());
    return 0;
}

int Client::screenshot()
{
    Request req(SCREENSHOT_REQUEST, 0, NULL);
    if (sendRequest(this->sockfd, req, 0))
        return -1;
    Response res;
    if (recvResponse(this->sockfd, res, 0))
        return -1;
    if (res.errCode() == FAIL_CODE || res.type() != CMD_RESPONSE_PNG)
    {
        return -1;
    }
    std::ofstream os("screenshot.png", std::ios::binary);
    if (!os)
    {
        return -1;
    }
    os.write((char *)res.data(), res.length());
    return 0;
}

int Client::startKeylog()
{
    if (this->_keylogThread.joinable())
        return -1;
    Request req(START_KEYLOG_REQUEST, 0, NULL);
    addrinfo hints, *servinfo;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((status = getaddrinfo(this->_serverName.c_str(), SERVER_PORT, &hints, &servinfo)) != 0)
    {
        std::cerr << "client: getaddrinfo " << gai_strerror(status) << std::endl;
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
    if (p == NULL)
    {
        std::cerr << "client: fail to connect\n";
        exit(1);
    }


    if (sendRequest(this->_keylogfd, req, 0))
    {
        closesocket(this->_keylogfd);
        return -1;
    }
    Response res;
    if (recvResponse(this->_keylogfd, res, 0))
    {
        closesocket(this->_keylogfd);
        return -1;
    }
    if (res.errCode() == FAIL_CODE || res.type() != CMD_RESPONSE_EMPTY)
    {
        closesocket(this->_keylogfd);
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
int Client::stopKeylog()
{
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

int Client::dirTree(const std::string &pathName, std::string &result)
{
    Request req(DIR_TREE_REQUEST, pathName.size() + 1, (void *)pathName.c_str());
    if (sendRequest(this->sockfd, req, 0))
        return -1;
    Response res;
    if (recvResponse(this->sockfd, res, 0))
        return -1;
    if (res.errCode() == FAIL_CODE || res.type() != CMD_RESPONSE_STR)
    {
        return -1;
    }
    result = std::string((char*)res.data());
    return 0;
}