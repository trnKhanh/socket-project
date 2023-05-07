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
    std::filesystem::path mypath(pathName);
    
    for (auto pathIt = std::filesystem::recursive_directory_iterator(mypath); pathIt != std::filesystem::recursive_directory_iterator(); ++pathIt)
    {
        os << std::string(pathIt.depth() * 4, ' ') << (*pathIt).path().filename().string() << "\n";
    }
    res = os.str();
    return 0;
}