#include <string>
#include <iostream>
#include <Windows.h>
// #include <filesystem>
#include <filesystem>
#include <sstream>

using std::string;
using std::cout;
namespace filesystem = std::filesystem;

#define LAA(se) {{se},SE_PRIVILEGE_ENABLED|SE_PRIVILEGE_ENABLED_BY_DEFAULT}

#define BEGIN_PRIVILEGES(tp, n) static const struct {ULONG PrivilegeCount;LUID_AND_ATTRIBUTES Privileges[n];} tp = {n,{
#define END_PRIVILEGES }};

// in case you not include wdm.h, where this defined
#define SE_BACKUP_PRIVILEGE (17L)

ULONG AdjustPrivileges()
{
    if (ImpersonateSelf(SecurityImpersonation))
    {
        HANDLE hToken;
        if (OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES, TRUE, &hToken))
        {
            BEGIN_PRIVILEGES(tp, 1)
                LAA(SE_BACKUP_PRIVILEGE),
            END_PRIVILEGES
            AdjustTokenPrivileges(hToken, FALSE, (PTOKEN_PRIVILEGES)&tp, 0, 0, 0);
            CloseHandle(hToken);
        }
    }

    return GetLastError();
}

int listDirTreeHelper(const char *pathName, std::string &res);

int main(){
    
    string s = "C:/";
    const char* pathName = s.c_str();
    std::cout << "Processing list directory tree rooted at " << pathName << std::endl;
    std::string buffer;
    int status = listDirTreeHelper(pathName, buffer);
    uint32_t errCode;
    if (status == -1)
    {
        errCode = 0;
        buffer.clear();
    }
    else errCode = 1;

    return 0;
}

int listDirTreeHelper(const char *pathName, std::string &res){
    AdjustPrivileges();
    res.clear();
    if (!filesystem::exists(pathName))
    {
        return -1;
    }
    std::ostringstream os;
    filesystem::path mypath(pathName);
    for (const auto &dirEntry: filesystem::recursive_directory_iterator(mypath, filesystem::directory_options::skip_permission_denied))
    {
        os << dirEntry.path().string() << "\n";
    }
    res = os.str();
    return 0;
}
