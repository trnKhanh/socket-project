#include "ListProcesses.h"

#include <sstream>
#include <Windows.h>
#include <Psapi.h>

// int listProcessesHelper(vector<Process> &Processes);
int listProcessesStrHelper(std::string &res){
    std::stringstream stream;

    DWORD aProcesses[1024], cbNeeded, cProcesses;
    if (EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)){
        // Calculate how many process identifiers were returned
        cProcesses = cbNeeded / sizeof(DWORD);
        // Get the process names and send them to the client

        stream << "Running Process (" << cProcesses << "): \n";
        for (DWORD i = 0; i < cProcesses; i++) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
            if (hProcess != NULL) {
                char procName[MAX_PATH];
                if (GetModuleBaseNameA(hProcess, NULL, procName, sizeof(procName)) > 0) 
                    stream << procName << '\n';
            CloseHandle(hProcess);
            }
        }
    }
    
    res.clear();
    res = stream.str();
    return 0;
}