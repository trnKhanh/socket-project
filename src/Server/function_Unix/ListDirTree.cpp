#ifdef __APPLE__

#include "ListDirTree.h"
#include <filesystem>
#include <sstream>

int listDirTreeHelper(const char *pathName, std::string &res)
{
    res.clear();
    if (!std::filesystem::exists(pathName))
    {
        return -1;
    }
    std::ostringstream os;
    try
    {
        dfs(pathName, os, 0);
        res = os.str();
    }
    catch(...)
    {
        return -1;
    }
    return 0;
}

int dfs(const char *pathName, std::ostringstream &os, int depth)
{
    if (!std::filesystem::is_directory(std::filesystem::path(pathName))
       ||std::filesystem::is_symlink(std::filesystem::path(pathName))) return -1;
    for (auto dirEntry: std::filesystem::directory_iterator(std::filesystem::path(pathName), std::filesystem::directory_options::skip_permission_denied))
    {
        os << std::string(depth * 4, ' ') << dirEntry.path().filename().string() << std::endl;
        try
        {
            dfs(dirEntry.path().string().c_str(), os, depth + 1);
        } 
        catch(...)
        {

        }
    }
    return 0;
}

#endif