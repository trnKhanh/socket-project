#include "AppCMD.h"

#include <sstream>
#include <tlhelp32.h>

int listAppHelper(std::string& res){
    // // Run the WMIC command to retrieve the list of installed applications
    // FILE* fp = _popen("WMIC /Node:localhost product get name,version", "r");
    // if(fp == NULL)
    //     return -1;

    // std::stringstream builder;
    // char buffer[1024];
    // // Read the output of the WMIC command and send it to the client over the socket

    // while (fgets(buffer, sizeof(buffer), fp) != NULL) 
    //     builder << buffer;
    // res = builder.str();
    // _pclose(fp);

    std::set <std::string> myAppsList;

    EnumerateInstalledApplications(myAppsList, HKEY_LOCAL_MACHINE, "Software\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
    EnumerateInstalledApplications(myAppsList, HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
    EnumerateInstalledApplications(myAppsList, HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
    EnumerateInstalledApplications(myAppsList, HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\AppModel\\Repository\\Packages");

    // EnumerateInstalledApplications(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths");
    // std::cout << s.size() << '\n';
    std::stringstream myStream;
    for (auto App: myAppsList)
        myStream << App << '\n';
    res.clear();
    res = myStream.str();
    return 0;
}

int startAppHelper(const std::string& appName){
    std::stringstream os;
    os << "start " << appName;
    std::string cmd = os.str();
    if(system(cmd.c_str()) == 0)
        return 0;
    return -1;
}

int stopAppHelper(const std::string& appName){
    std::stringstream stream;
    stream << appName << ".exe";
    std::string fullappName = stream.str();
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
    std::stringstream stream;
    stream << processName;
    std::string processName1 = stream.str();
    DWORD processId = 0;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 processEntry = { 0 };
        processEntry.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(hSnapshot, &processEntry)) {
            do {
                std::string currentProcessName((char*)processEntry.szExeFile);
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

void EnumerateInstalledApplications(std::set <std::string>& s, HKEY key, const char* path){
    int cnt = 0;
    HKEY hUninstallKey;
    if (RegOpenKeyEx(key, path, 0, KEY_READ, &hUninstallKey) == ERROR_SUCCESS){
        char subKeyName[256];
        DWORD subKeyNameSize = sizeof(subKeyName);

        for (DWORD index = 0; RegEnumKeyEx(hUninstallKey, index, subKeyName, &subKeyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS; ++index){
            HKEY hAppKey;
            if (RegOpenKeyEx(hUninstallKey, subKeyName, 0, KEY_READ, &hAppKey) == ERROR_SUCCESS)
            {
                char displayName[256];
                DWORD tmp;
                DWORD tmpsize;
                DWORD displayNameSize = sizeof(displayName);

                if (RegQueryValueEx(hAppKey, "DisplayName", NULL, NULL, reinterpret_cast<LPBYTE>(displayName), &displayNameSize) == ERROR_SUCCESS){
                    s.insert(displayName);
                }

                RegCloseKey(hAppKey);
            }

            subKeyNameSize = sizeof(subKeyName);
        }

        RegCloseKey(hUninstallKey);
    }
}