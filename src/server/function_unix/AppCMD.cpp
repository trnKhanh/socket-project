#ifdef __APPLE__

#include "AppCMD.h"
#include "../../Utils/Exec.h"
#include "../../Utils/String.h"
#include <vector>
#include <libproc.h>
#include <sstream>
#include "ListProcesses.h"

int listAppHelper(std::string &res)
{
    std::string allApp;
    if (exec("mdfind 'kMDItemKind=Application'", allApp) == -1)
    {
        return -1;
    }
    std::vector<std::string> tmp;
    std::string runningApp;
    if (exec("osascript -e 'tell application \"System Events\" to get unix id of (processes where background only is false)'", runningApp) == -1)
    {
        return -1;
    }
    std::vector<int> runningPids;
    String::split(runningApp, ", ", tmp);
    for (auto s: tmp)
        runningPids.push_back(std::stoi(s));

    std::ostringstream os;
    os << "All applications:\n" << allApp << "\nRunning applications:\n";
    for (auto pid: runningPids)
    {
        Process buffer;
        if (getProcessByPIDHelper(pid, buffer) == -1)
            return -1;
        os << buffer << "\n";
    }
    res = os.str();

    return 0;
}
int startAppHelper(const std::string &appName)
{
    if (system(("open -a \"" + appName + "\"").c_str())) return -1;
    return 0;
}
int stopAppHelper(const std::string &appName)
{
    if (system(("killall -9 \"" + appName + "\"").c_str())) return -1;
    return 0;
}

#endif