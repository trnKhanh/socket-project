#include "ListDirTree.h"
#include <filesystem>
#include <sstream>

int listDirTree(const char *pathName, std::string &res)
{
    res.clear();
    if (!std::filesystem::exists(pathName))
    {
        return -1;
    }
    std::ostringstream os;
    std::filesystem::path mypath(pathName);
    for (const auto &dirEntry: std::filesystem::recursive_directory_iterator(mypath))
    {
        os << std::filesystem::relative(dirEntry.path(), mypath)<< "\n";
    }
    res = os.str();
    return 0;
}