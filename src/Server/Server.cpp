#include "Server.h"
#include "../Utils/InUtils.h"
#include <iostream>
#include <string.h> 
#include <string>
#include <mutex>
#include <sstream>
#include <time.h>

Server::~Server()
{
    std::cerr << "Server closed" << std::endl;
    for (pollfd pfd: this->pfds)
    {
        closesocket(pfd.fd);
    }
    closesocket(this->listener);
    closesocket(this->disfd);
    #ifdef _WIN32
        WSACleanup();
    #endif
}
Server::Server()
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
    char port[] = SERVER_PORT;
    int status;
    int yes = 1;

    addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, port, &hints, &res)) == -1)
    {
        std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
        exit(1);
    }

    addrinfo *p = NULL;
    for (p = res; p != NULL; p = p->ai_next)
    {
        if ((this->listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }
        if (setsockopt(this->listener, SOL_SOCKET, SO_REUSEADDR, (sockopt_type)&yes, sizeof(int)) == -1) {
            perror("server: setsockopt");
            exit(1); 
        }
        if (bind(this->listener, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("server: bind");
            closesocket(this->listener);
            continue;
        }
        break;
    }

    char host[256];
    struct hostent *host_entry;
    if (gethostname(host, sizeof(host)) == -1)
    {
        perror("gethostname");
        exit(1);
    }
    host_entry = gethostbyname(host); //find host information
    if (host_entry == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }
    std::cout << "Current Host Name: " << host << std::endl;
    std::cout << "Host IP: \n";
    for(int i = 0; host_entry->h_addr_list[i] != NULL; ++i) 
        std::cout << "  " << inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[i])) << std::endl;

    std::cerr << "Server started on " << getIpStr(p->ai_addr) << ":"; 
    std::cerr << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << std::endl;
    
    freeaddrinfo(res);
    if (p == NULL)
    {
        std::cerr << "server: fail to bind\n";
        exit(1);
    }
    if (listen(this->listener, BACKLOG) == -1)
    {
        perror("server: listen");
        exit(1);
    }
    this->pfds.push_back(pollfd());
    this->pfds.back().fd = this->listener;
    this->pfds.back().events = POLLIN;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, port, &hints, &res)) == -1)
    {
        std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
        exit(1);
    }

    p = NULL;
    for (p = res; p != NULL; p = p->ai_next)
    {
        if ((this->disfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("server discover: socket");
            continue;
        }
        if (setsockopt(this->disfd, SOL_SOCKET, SO_REUSEADDR, (sockopt_type)&yes, sizeof(int)) == -1) {
            perror("server discover: setsockopt");
            exit(1); 
        }
        if (bind(this->disfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("server discover: bind");
            closesocket(this->disfd);
            continue;
        }
        break;
    }
    
    std::cerr << "Discover server started on " << getIpStr(p->ai_addr) << ":"; 
    std::cerr << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << std::endl;
    
    freeaddrinfo(res);
    if (p == NULL)
    {
        std::cerr << "server: fail to bind\n";
        exit(1);
    }

    this->pfds.push_back(pollfd());
    this->pfds.back().fd = this->disfd;
    this->pfds.back().events = POLLIN;
}
void Server::start()
{
    int newfd;

    sockaddr_storage remote_address;
    socklen_t addrlen;
    
    while (1)
    {
        int poll_count = poll(this->pfds.data(), pfds.size(), -1);
        if (poll_count == -1)
        {
            perror("poll");
            exit(1);
        }
        for (int i = 0; i < this->pfds.size(); ++i)
        {
            auto pfd = this->pfds[i];
            // get one ready to read
            if (pfd.revents & (POLLIN | POLLHUP))
            {
                // listener ready to read - new connection
                if (pfd.fd == this->listener)
                {
                    addrlen = sizeof(remote_address);
                    newfd = accept(this->listener, (sockaddr *)&remote_address, &addrlen);
                    if (newfd == -1)
                    {
                        perror("accept");
                    } else
                    {
                        time_t now = time(0);
                        std::cerr << ctime(&now) << "    ";
                        std::cerr << "NEW CONNECTION: ";
                        this->pfds.push_back(pollfd());
                        this->pfds.back().fd = newfd;
                        this->pfds.back().events = POLLIN;

                        std::ostringstream os;
                        
                        int addrport = ntohs(((sockaddr_in *)&remote_address)->sin_port);
                        os << getIpStr((sockaddr *)&remote_address) << ":" << addrport;
                        this->_clientIP[newfd] = os.str();
                        std::cerr << "New connection from " << getIpStr((sockaddr *)&remote_address) << ":" << addrport
                             << " on socket " << newfd << std::endl;
                    }
                }
                else if (pfd.fd == this->disfd) // receive discover message
                {  
                    
                    Request r;
                    addrlen = sizeof(remote_address);
                    if (recvfromRequest(this->disfd, r, 0, (sockaddr *)&remote_address, &addrlen) == -1)
                    {
                        perror("discover request receive");
                    }
                    else if (r.type() == DISCOVER_REQUEST)
                    {
                        time_t now = time(0);
                        std::cerr << ctime(&now) << "    ";
                        std::cerr << "DISCOVER: ";
                        std::cerr << "Receive discover message from " << getIpStr((sockaddr *)&remote_address) << ":"; 
                        std::cerr << ntohs(((sockaddr_in *)&remote_address)->sin_port) << std::endl;
                        Response msg(DISCOVER_RESPONSE, OK_CODE, 0, NULL);
                        
                        if (sendtoResponse(this->disfd, msg, 0, (sockaddr *)&remote_address, addrlen) == -1)
                        {
                            perror("send offer");
                        }
                    }
                }
                else
                {
                    Request buffer;
                    
                    if (recvRequest(pfd.fd, buffer, 0) == -1)
                    {
                        int sockfd = pfd.fd;
                        this->pfds[i] = this->pfds.back();
                        this->pfds.pop_back();
                        if (keylogfds.find(sockfd) != keylogfds.end())
                            keylogfds.erase(keylogfds.find(sockfd));
                        time_t now = time(0);
                        std::cerr << ctime(&now) << "    ";
                        std::cerr << "Connection closed from " << this->_clientIP[sockfd] << std::endl;
                        this->_clientIP.erase(this->_clientIP.find(sockfd));
                        closesocket(sockfd);
                    } 
                    else
                    {
                        Response res;
                        time_t now = time(0);
                        std::cerr << ctime(&now) << "    ";
                        std::cerr << "REQUEST: ";
                        if (buffer.type() == LIST_APP_REQUEST)
                        {
                            std::cerr << "Received List App Request from " << this->_clientIP[pfd.fd] << std::endl;
                            res = this->listApp();
                        }
                        else if (buffer.type() == START_APP_REQUEST)
                        {
                            std::cerr << "Received Start App Request (" << (char *)buffer.data() << ") from " << this->_clientIP[pfd.fd] << std::endl;
                            res = this->startApp((char *)buffer.data());
                        }
                        else if (buffer.type() == STOP_APP_REQUEST)
                        {
                            std::cerr << "Received Stop App Request (" << (char *)buffer.data() << ") from " << this->_clientIP[pfd.fd] << std::endl;
                            res = this->stopApp((char *)buffer.data());
                        }
                        else if (buffer.type() == LIST_PROC_REQUEST)
                        {
                            std::cerr << "Received List Process Request from " << this->_clientIP[pfd.fd] << std::endl;
                            res = this->listProcesss();
                        }
                        else if (buffer.type() == SCREENSHOT_REQUEST)
                        {
                            std::cerr << "Received Screenshot Request from " << this->_clientIP[pfd.fd] << std::endl;
                            res = this->screenshot();
                        }
                        else if (buffer.type() == START_KEYLOG_REQUEST)
                        {
                            std::cerr << "Received Start Keylog Request from " << this->_clientIP[pfd.fd] << std::endl;
                            res = this->startKeylog(pfd.fd);
                        }
                        else if (buffer.type() == STOP_KEYLOG_REQUEST)
                        {
                            std::cerr << "Received Stop Keylog Request from " << this->_clientIP[pfd.fd] << std::endl;
                            res = this->stopKeylog(pfd.fd);
                        }
                        else if (buffer.type() == DIR_TREE_REQUEST)
                        {
                            std::cerr << "Received List Directory Tree Request from " << this->_clientIP[pfd.fd] << std::endl;
                            res = this->dirTree((char *)buffer.data());
                        }
                        else 
                        {
                            std::cerr << "Received Invalid Request from " << this->_clientIP[pfd.fd] << std::endl;
                            continue;
                        }
                        if (sendResponse(pfd.fd, res, 0) == -1)
                        {
                            perror("sendResponse");
                        }
                        if (buffer.type() == STOP_KEYLOG_REQUEST)
                        {
                            this->pfds[i] = this->pfds.back();
                            this->pfds.pop_back();
                            closesocket(pfd.fd);
                        }
                    }

                    
                }
            }
        }
    }
}

Response Server::listApp()
{
    std::string buffer;
    uint32_t errCode;
    if (listAppHelper(buffer) == -1)
    {
        buffer.clear();
        errCode = FAIL_CODE;
    }
    else errCode = OK_CODE;

    return Response(CMD_RESPONSE_STR, errCode, buffer.size() + 1, (void *)buffer.c_str());
}
Response Server::startApp(const char *appName)
{
    uint32_t errCode;
    if (startAppHelper(appName) == -1)
    {
        errCode = FAIL_CODE;
    }
    else errCode = OK_CODE;

    return Response(CMD_RESPONSE_EMPTY, errCode, 0, NULL);
}
Response Server::stopApp(const char *appName)
{
    uint32_t errCode;
    if (stopAppHelper(appName) == -1)
    {
        errCode = FAIL_CODE;
    }
    else errCode = OK_CODE;

    return Response(CMD_RESPONSE_EMPTY, errCode, 0, NULL);
}

Response Server::listProcesss()
{
    std::string buffer;
    int status = listProcessesStrHelper(buffer);
    uint32_t errCode;
    if (status == -1)
    {
        errCode = FAIL_CODE;
        buffer.clear();
    }
    else errCode = OK_CODE;

    return Response(CMD_RESPONSE_STR, errCode, buffer.size() + 1, (void *)buffer.c_str());
}

Response Server::screenshot()
{
    std::vector<char> buffer;
    uint32_t errCode;
    if (screenshotHelper(buffer))
        errCode = FAIL_CODE;
    else errCode = OK_CODE;

    return Response(CMD_RESPONSE_PNG, errCode, buffer.size(), buffer.data());
}

Response Server::startKeylog(int sockfd)
{
    std::mutex m;
    std::lock_guard l(m);
    keylogfds.insert(sockfd);
    return Response(CMD_RESPONSE_EMPTY, OK_CODE, 0, NULL);
}
Response Server::stopKeylog(int sockfd)
{
    std::mutex m;
    std::lock_guard l(m);

    if (keylogfds.find(sockfd) != keylogfds.end())
        keylogfds.erase(keylogfds.find(sockfd));

    return Response(CMD_RESPONSE_EMPTY, OK_CODE, 0, NULL);
}

Response Server::dirTree(const char *pathName)
{
    std::cerr << "Processing list directory tree rooted at " << pathName << std::endl;
    std::string buffer;
    int status = listDirTreeHelper(pathName, buffer);
    uint32_t errCode;
    if (status == -1)
    {
        errCode = FAIL_CODE;
        buffer.clear();
    }
    else errCode = OK_CODE;

    return Response(CMD_RESPONSE_STR, errCode, buffer.size() + 1, (void *)buffer.c_str());
}