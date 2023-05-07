#include "AppCMD.h"

#include <sstream>
#include <tlhelp32.h>

using std::stringstream;

int listAppHelper(string& res){
    // Run the WMIC command to retrieve the list of installed applications
    FILE* fp = _popen("WMIC /Node:localhost product get name,version", "r");
    if(fp == NULL)
        return -1;

    std::stringstream builder;
    char buffer[1024];
    // Read the output of the WMIC command and send it to the client over the socket

    while (fgets(buffer, sizeof(buffer), fp) != NULL) 
        builder << buffer;
    res = builder.str();
    _pclose(fp);

    return 0;
}

int startAppHelper(const string& appName){
    std::stringstream os;
    os << "start " << appName;
    string cmd = os.str();
    if(system(cmd.c_str()) == 0)
        return 0;
    return -1;
}

int stopAppHelper(const string& appName){
    std::stringstream stream;
    stream << appName << ".exe";
    string fullappName = stream.str();
    const char* processName = fullappName.c_str();

    // Get the process ID of the running process to stop
    DWORD dwProcessId = GetProcessIdByName(processName); // Replace with the actual process ID

    // Open the process
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
    if (hProcess == NULL) 
        return -1;                    

    // Terminate the process
    if (!TerminateProcess(hProcess, 0)) {
        CloseHandle(hProcess);
        return -1;  
    }
    // Close the process handle
    CloseHandle(hProcess);
    return 0;
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
                string currentProcessName((char*)processEntry.szExeFile);
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