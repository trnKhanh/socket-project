#include "Server.h"
#include <string.h> 
#include <sys/types.h> 
#include <signal.h>
#include <string>
#include <psapi.h>
#include <filesystem>
#include <tlhelp32.h>

#include "../Utils/InUtils.h"
#include "../Utils/ConvertUtils.h"
#include "../Utils/MsgTransport.h"
#include "../Message/Request.h"
#include "../Message/Response.h"

using std::cin;
using std::cout;
using std::cerr;

Server::~Server(){
    cout << "Server closed." << "\n";
    closesocket(this->listener);
    closesocket(this->disfd);
}

Server::Server(const char* port){
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
        
    int status;
    int yes = 1;
    addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    status = getaddrinfo(NULL, port, &hints, &res);
    if (status == -1){
        cerr << "getaddrinfo: " << gai_strerror(status) << "\n";
        exit(1);
    }

    addrinfo *p = NULL;
    for (p = res; p != NULL; p = p->ai_next){
        this->listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (this->listener == INVALID_SOCKET){
            cerr << "Server: socket\n";
            continue;
        }
        if (setsockopt(this->listener, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int)) == -1) {
            cerr << "Server: setsockopt\n";
            exit(1); 
        }
        if (bind(this->listener, p->ai_addr, p->ai_addrlen) == -1){
            cerr << "Server: bind\n";
            closesocket(this->listener);
            continue;
        }
        break;
    }
        
    cout << "Server started on " << getIpStr(p->ai_addr) << "::" 
        << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << "\n";
        
    freeaddrinfo(res);
    if (p == NULL){
        cerr << "Server: fail to bind\n";
        exit(1);
    }

    status = listen(this->listener, BACKLOG);
    if (status == -1){
        cerr << "Server: listen\n";
        exit(1);
    }

    this->pfds.emplace_back();
    this->pfds.back().fd = this->listener;
    this->pfds.back().events = POLLIN;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    status = getaddrinfo(NULL, port, &hints, &res);
    if (status == SOCKET_ERROR){
        cerr << "getaddrinfo: " << gai_strerror(status) << "\n";
        exit(1);
    }

    for (p = res; p != NULL; p = p->ai_next){
        this->disfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (this->disfd == INVALID_SOCKET){
            cerr << "Server: socket\n";
            continue;
        }
        if (setsockopt(this->disfd, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(int)) == SOCKET_ERROR) {
            cerr << "Server: setsockopt";
            exit(1); 
        }
        if (bind(this->disfd, p->ai_addr, p->ai_addrlen) == SOCKET_ERROR){
            cerr << "Server: bind\n";
            closesocket(this->disfd);
            continue;
        }
        break;
    }
        
    cout << "Discover server started on " << getIpStr(p->ai_addr) << "::"; 
    cout << ntohs(((sockaddr_in *)p->ai_addr)->sin_port) << "\n";
        
    freeaddrinfo(res);
    if (p == NULL){
        cerr << "Server: fail to bind\n";
        exit(1);
    }

    this->pfds.emplace_back();
    this->pfds.back().fd = this->disfd;
    this->pfds.back().events = POLLIN;
}

void Server::start(){
    SOCKET newfd;

    sockaddr_storage remote_address;
    socklen_t addrlen;

    cout << "Server is running...\n";
    char buffer[256];
    while (true){
        int poll_count = WSAPoll(&pfds[0], pfds.size(), -1); // wait util events occur
        if (poll_count == -1){
            cerr << "poll\n";
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
                        cerr << "accept\n";
                    } 
                    else{
                        pfds.emplace_back();
                        pfds.back().fd = newfd;
                        pfds.back().events = POLLIN;
                        
                        cout << "New connection from " << getIpStr((sockaddr *)&remote_address)
                             << " on socket " << newfd << "\n";
                    }
                } 
                else if (pfd.fd == disfd){ // receive discover message  
                    Request req;
                    addrlen = sizeof(remote_address);
                    recvfromRequest(disfd, req, 0, (sockaddr*)&remote_address, &addrlen);
                    if (req.type() == DISCOVER_REQUEST){
                        cout << "Receive discover message from " << getIpStr((sockaddr *)&remote_address) << "::"
                        << ntohs(((sockaddr_in *)&remote_address)->sin_port) << '\n';
                        Response msg(DISCOVER_RESPONSE, OK_ERRCODE, 0, NULL);
                        
                        sendtoResponse(disfd, msg, 0, (sockaddr *)&remote_address, addrlen);
                        continue;
                    }
                    
                } 
                else{
                    Request req;
                    addrlen = sizeof(remote_address);
                    recvfromRequest(pfd.fd, req, 0, (sockaddr*)&remote_address, &addrlen);
                    if (req.type() == LIST_PROCESS_REQUEST) {
                        DWORD aProcesses[1024], cbNeeded, cProcesses;
                        if (EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
                            // Calculate how many process identifiers were returned
                            cProcesses = cbNeeded / sizeof(DWORD);
                            // Get the process names and send them to the client
                            char numProc[20];
                            my_itos(numProc, cProcesses);
                            cout << cProcesses << '\n';
                            send(pfd.fd, numProc, strlen(numProc), 0);
                            for (DWORD i = 0; i < cProcesses; i++) {
                                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
                                if (hProcess != NULL) {
                                    char procName[MAX_PATH];
                                    if (GetModuleBaseNameA(hProcess, NULL, procName, sizeof(procName)) > 0) {
                                        send(pfd.fd, procName, strlen(procName), 0);
                                        send(pfd.fd, "\n", 1, 0);
                                    }
                                    CloseHandle(hProcess);
                                }
                            }
                        }
                    }
                    else if(req.type() == LIST_APP_REQUEST){
                        // Run the WMIC command to retrieve the list of installed applications
                        FILE* fp = _popen("WMIC /Node:localhost product get name,InstallLocation", "r");
                        if(fp == NULL){
                            cerr << "Failed to excute command\n";
                            continue;
                        }
                        char buffer[1024];
                        // Read the output of the WMIC command and send it to the client over the socket
                        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
                            // cout << buffer << '\n';
                            send(pfd.fd, buffer, strlen(buffer), 0);
                        }
                        _pclose(fp);
                    }
                    else if(req.type() == START_APP_REQUEST){
                        this->startApp("", pfd.fd);
                    }
                    else if(req.type() == STOP_APP_REQUEST){
                        this->stopApp("", pfd.fd);
                    }
                    else if (req.type() == DISCONNECT_REQUEST)
                        break;   
                    else {
                        // Unknown request
                        const char* response = "UNKNOWN REQUEST\n";
                        send(pfd.fd, response, strlen(response), 0);
                    }
                }
            }
        }
    } 
}

int Server::listApp(SOCKET& fd){
    return 0;
}

int Server::startApp(const char* appName, SOCKET& fd){
    // The name of the application to retrieve information for

    // Get the size of the buffer required to store the application information
    DWORD dataSize = 0;
    if (RegGetValue(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\calc.exe", nullptr, RRF_RT_REG_SZ, nullptr, nullptr, &dataSize) != ERROR_SUCCESS){
        cerr << "Failed to get data size\n";
        return -1;
    }

    // Allocate a buffer to store the application information
    vector<uint8_t> data(dataSize);

    // Retrieve the application information
    if (RegGetValue(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\calc.exe", nullptr, RRF_RT_REG_SZ, nullptr, &data[0], &dataSize) != ERROR_SUCCESS){
        cerr << "Failed to get data\n";
        return -1;
    }

    // Convert the data to a wide string
    string appPath(reinterpret_cast<char*>(&data[0]));

    // Check if the file exists
    if (!std::filesystem::exists(appPath.c_str())){
        cerr << "File not found\n";
        return -1;
    }

    cout << "Application path: " << appPath << '\n';
    char* cmd = NULL; // works... calc.exe is in windows/system32 
    string_to_listchar(cmd, appPath);
    // char* cmd = "chrome"; // doesn't work... how can I add the path if it's not known (e.g. windows installed on D:\)
    // char* cmd = "c:/program files (x86)/google/chrome/application/chrome"; // works (even without extension .exe)

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Start the child process. 
    if (!CreateProcess(
        NULL,   // No module name (use command line)
        cmd,            // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)           // Pointer to PROCESS_INFORMATION structure
    ){
        printf("CreateProcess failed (%d).\n", GetLastError());
        delete[] cmd;
        return -1;
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    delete[] cmd;
    return 0;
}

int Server::stopApp(const char* appName, SOCKET& fd){
    char* processName = (char*)malloc(strlen(appName) + 5);
    strcpy(processName, appName);
    strcat(processName, ".exe");


    // Get the process ID of the running process to stop
    DWORD dwProcessId = GetProcessIdByName(processName); // Replace with the actual process ID

    // Open the process
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
    if (hProcess == NULL) {
        cerr << "Error: Failed to open process.\n";
        return -1;                        
    }

    // Terminate the process
    if (!TerminateProcess(hProcess, 0)) {
        cerr << "Error: Failed to terminate process.\n";
        CloseHandle(hProcess);
        return -1;
    }
    // Close the process handle
    CloseHandle(hProcess);
    return 0;
}

int Server::listProcess(SOCKET& fd){
    return 0;
}

int Server::screenShot(SOCKET& fd){
    return 0;
}

int Server::keyLog(SOCKET& fd){
    return 0;
}
    
int Server::dirTree(SOCKET& fd){
    return 0;
}

// Function to get the PID of a process given its name
DWORD GetProcessIdByName(const char* processName){
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(entry);

    if (!Process32FirstW(snapshot, &entry))
        return 0;

    do {
        if (strcmp((char*)entry.szExeFile, processName) == 0) {
            CloseHandle(snapshot);
            return entry.th32ProcessID;
        }
    } while (Process32NextW(snapshot, &entry));

    CloseHandle(snapshot);
    return 0;
}