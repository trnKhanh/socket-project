#include "Server.h"

#include <string.h> 
#include <sys/types.h> 
#include <signal.h>
#include <psapi.h>
#include <filesystem>
#include <tlhelp32.h>
#include <sstream>
#include <fstream>
#include <gdiplus.h> //screenshot
#include <windows.h>
#include <iostream>

#include "../Utils/InUtils.h"
#include "../Utils/ConvertUtils.h"
#include "../Utils/MsgTransport.h"
#include "../Message/Request.h"
#include "../Message/Response.h"

using std::cin;
using std::cout;
using std::cerr;
using std::stringstream;
using std::ifstream;
using std::ios;

using namespace Gdiplus;

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
                        cerr << "Connection closed\n";
                        break;
                    }

                    if(requestFromClient.type() == LIST_APP_REQUEST)
                        responseToClient = this->listApp();
                    else if(requestFromClient.type() == START_APP_REQUEST)
                        responseToClient = this->startApp((char*)requestFromClient.data());
                    else if(requestFromClient.type() == STOP_APP_REQUEST)
                        responseToClient = this->stopApp((char*)requestFromClient.data());
                    else if (requestFromClient.type() == LIST_PROCESS_REQUEST) 
                        responseToClient = this->listProcess();
                    else if (requestFromClient.type() == SCREENSHOT_REQUEST)
                        responseToClient = this->screenShot();
                    else if (requestFromClient.type() == DIR_TREE_REQUEST)
                        responseToClient = this->dirTree();
                    else if (requestFromClient.type() == DISCONNECT_REQUEST)
                        responseToClient = this->disconnect();
                    else 
                        responseToClient = Response(UNKNOWN_RESPONSE, OK_CODE, 0, NULL);

                    status = sendResponse(pfd.fd, responseToClient, 0);
                    if(status == SOCKET_ERROR)
                        cerr << "Can't send response.\n";
                }
            }
        }
    } 
}

Response Server::listApp(){
    cout << "Server: Received listing installed applications instruction.\n";

    uint32_t errCode;
    string result;

    // Run the WMIC command to retrieve the list of installed applications
    FILE* fp = _popen("WMIC /Node:localhost product get name,version", "r");
    if(fp == NULL){
        errCode = FAIL_CODE;
        cerr << "Error: Failed to excute command\n";
    }
    else{
        errCode = OK_CODE;
        stringstream builder;
        char buffer[1024];
        // Read the output of the WMIC command and send it to the client over the socket

        int i = 0;
        while (fgets(buffer, sizeof(buffer), fp) != NULL) 
            builder << buffer;
        result = builder.str();
        _pclose(fp);
    }

    cout << "Server: All installed applications are listed.\n";
    return Response(LIST_APP_RESPONSE, errCode, result.size() + 1, (void *)result.c_str());
}

Response Server::startApp(const char* appName){
    // The name of the application to retrieve information for

    // Get the size of the buffer required to store the application information
    stringstream stream;
    stream << "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\" << appName << ".exe";
    string path = stream.str();

    cout << path << '\n';
    DWORD dataSize = 0;
    if (RegGetValue(
        HKEY_LOCAL_MACHINE, 
        path.c_str(), 
        nullptr, 
        RRF_RT_REG_SZ, 
        nullptr, 
        nullptr, 
        &dataSize) != ERROR_SUCCESS){
        cerr << "Error: Failed to get data size\n";
        return Response(START_APP_RESPONSE, FAIL_CODE, 0, NULL);
    }

    // Allocate a buffer to store the application information
    vector<uint8_t> data(dataSize);

    // Retrieve the application information
    if (RegGetValue(
        HKEY_LOCAL_MACHINE, 
        path.c_str(), 
        nullptr, 
        RRF_RT_REG_SZ, 
        nullptr, 
        &data[0], 
        &dataSize) != ERROR_SUCCESS){
        cerr << "Error: Failed to get data\n";
        return Response(START_APP_RESPONSE, FAIL_CODE, 0, NULL);
    }

    // Convert the data to a wide string
    string appPath(reinterpret_cast<char*>(&data[0]));

    // Check if the file exists
    if (!std::filesystem::exists(appPath.c_str())){
        cerr << "File not found\n";
        return Response(START_APP_RESPONSE, FAIL_CODE, 0, NULL);
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
        cerr << "CreateProcess failed (" << GetLastError() << ").\n";
        delete[] cmd;
        return Response(START_APP_RESPONSE, FAIL_CODE, 0, NULL);
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    delete[] cmd;
    return Response(START_APP_RESPONSE, OK_CODE, 0, NULL);
}

Response Server::stopApp(const char* appName){
    stringstream stream;
    stream << appName << ".exe";
    string fullappName = stream.str();
    const char* processName = fullappName.c_str();

    // Get the process ID of the running process to stop
    DWORD dwProcessId = GetProcessIdByName(processName); // Replace with the actual process ID
    
    // Open the process
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
    if (hProcess == NULL) {
        cerr << "Error: Failed to open process.\n";
        return Response(START_APP_RESPONSE, FAIL_CODE, 0, NULL);                     
    }

    // Terminate the process
    if (!TerminateProcess(hProcess, 0)) {
        cerr << "Error: Failed to terminate process.\n";
        CloseHandle(hProcess);
        return Response(START_APP_RESPONSE, FAIL_CODE, 0, NULL);  
    }
    // Close the process handle
    CloseHandle(hProcess);
    return Response(START_APP_RESPONSE, OK_CODE, 0, NULL);  
}

Response Server::listProcess(){
    cout << "Server: Received listing processing instruction.\n";
    stringstream builder;
    string result;

    DWORD aProcesses[1024], cbNeeded, cProcesses;
    if (EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)){
        // Calculate how many process identifiers were returned
        cProcesses = cbNeeded / sizeof(DWORD);
        // Get the process names and send them to the client

        builder << "Running Process (" << cProcesses << "): \n";
        for (DWORD i = 0; i < cProcesses; i++) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
            if (hProcess != NULL) {
                char procName[MAX_PATH];
                if (GetModuleBaseNameA(hProcess, NULL, procName, sizeof(procName)) > 0) 
                    builder << procName << '\n';
            CloseHandle(hProcess);
            }
        }
    }

    uint32_t errCode = OK_CODE;
    result = builder.str();

    cout << "Server: All processes are listed.\n";
    return Response(LIST_PROCESS_RESPONSE, errCode, result.size() + 1, (void *)result.c_str());
}

Response Server::screenShot(){
    cout << "Server: Received screen capture instruction.\n";

    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);

    SelectObject(hdcMem, hBitmap);
    BitBlt(hdcMem, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY);

    CLSID clsid;
    int status = GetEncoderClsid(L"image/png", &clsid);
    if(status == -1){
        cerr << "Server: Can't capture screen.\n";

        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);

        GdiplusShutdown(gdiplusToken);
        return Response(SCREENSHOT_RESPONSE, FAIL_CODE, 0, NULL); 
    }

    Bitmap bmp(hBitmap, (HPALETTE)0);
    bmp.Save(L"screenshot.png", &clsid, NULL);

    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    //GdiplusShutdown(gdiplusToken);

    ifstream fi("screenshot.png", ios::binary);
    if(fi.fail()){
        cerr << "Server: Can't open file screenshot.png\n";
        return Response(SCREENSHOT_RESPONSE, FAIL_CODE, 0, NULL);
    }

    vector <char> buffer = vector<char>(std::istreambuf_iterator<char>(fi), {});

    fi.close();
    status = remove("screenshot.png");
    if(status)
        cerr << "Server: Can't delete file screenshot png or file not exits\n";

    cout << "Server: Sent screenshot.png to client.\n";
    return Response(SCREENSHOT_RESPONSE, OK_CODE, buffer.size(), buffer.data());  
}

Response Server::keyLog(){
    return Response(START_KEYLOG_RESPONSE, OK_CODE, 0, NULL); 
}
    
Response Server::dirTree(){
    cout << "Server: Received directory tree instruction.\n";
    string result = printDirectoryTree("C:\\", 0);

    cout << "Server: Directoried tree.\n";
    return Response(DIR_TREE_RESPONSE, OK_CODE, result.size() + 1, (void*)result.c_str());
}

Response Server::disconnect(){
    cout << "Server: Received disconnect instruction.\n";
    cout << "Server: Client disconnected.\n";

    return Response(DISCONNECT_RESPONSE, OK_CODE, 0, NULL);
}

// Function to get the PID of a process given its name
DWORD GetProcessIdByName(const char* processName) {
    stringstream stream;
    stream << processName;
    string processName1 = stream.str();
    DWORD processId = 0;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 processEntry = { 0 };
        processEntry.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(hSnapshot, &processEntry)) {
            do {
                string currentProcessName(processEntry.szExeFile);
                if (currentProcessName == processName1) {
                    processId = processEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &processEntry));
        }

        CloseHandle(hSnapshot);
    }

    return processId;
}

string printDirectoryTree(const char* path, int indent){
    stringstream stream;
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA((string(path) + "\\*").c_str(), &fd);

    if (hFind != INVALID_HANDLE_VALUE){
        do{
            if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0){
                for (int i = 0; i < indent; i++)
                    stream << "  ";

                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
                    stream << "[+] " << fd.cFileName << '\n';
                    stream << printDirectoryTree((string(path) + "\\" + fd.cFileName).c_str(), indent + 1);
                }
                else{
                    stream << "    " << fd.cFileName << '\n';
                }
            }
        } while (FindNextFileA(hFind, &fd));

        FindClose(hFind);
    }

    string result = stream.str();
    return result;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid){
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if(size == 0)
        return -1;  // Failure

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if(pImageCodecInfo == NULL)
        return -1;  // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for(UINT j = 0; j < num; ++j){
        if(wcscmp(pImageCodecInfo[j].MimeType, format) == 0){
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}

