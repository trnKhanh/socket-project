#include "String.h"
#include <sstream>

int String::split(const std::string &str, const std::string &c, std::vector<std::string> &res)
{
    res.clear();
    int i = 0;
    while (i < str.size())
    {
        int found = str.find(c, i);
        if (found == -1) found = (int)str.size();
        res.push_back(str.substr(i, found - i));
        i = found + c.size();
    }
    return 0;
}
int String::join(const std::vector<std::string> &strs, const std::string &c, std::string &res)
{
    std::ostringstream os;
    for (int i = 0; i < strs.size(); ++i)
    {
        os << strs[i];
        if (i + 1 < strs.size())
            os << c;
    }
    res = os.str();
    return 0;
}