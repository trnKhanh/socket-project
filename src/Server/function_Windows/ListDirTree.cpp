#include "ListDirTree.h"

#include <Windows.h>
#include <sstream>

using std::stringstream;

int listDirTreeHelper(const char* path, string& buffer){
    buffer.clear();
    buffer = listDirectoryTree(path, 0);
    return 0;
}

string listDirectoryTree(const char* path, int indent){
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
                    stream << listDirectoryTree((string(path) + "\\" + fd.cFileName).c_str(), indent + 1);
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