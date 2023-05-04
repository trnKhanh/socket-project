#include "ListProcesses.h"
#include <sys/sysctl.h>
#include <stdio.h>
#include <cstdlib>
#include <algorithm>
#include <sstream>

int listProcesses(std::vector<Process> &Processes)
{
    int name[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    kinfo_proc *p;
    size_t len;
    // get total size of all processes 
    if (sysctl(name, sizeof(name)/sizeof(*name), NULL, &len, NULL, 0) == -1)
    {
        perror("sysctl");
        return -1;
    } 
    p = (kinfo_proc *)malloc(len);
    // get all process
    if (sysctl(name, sizeof(name)/sizeof(*name), p, &len, NULL, 0) == -1)
    {
        perror("sysctl");
        return -1;
    }
    int processNumber = len / sizeof(kinfo_proc);
    Processes.clear();
    for (int i = 0; i < processNumber; ++i)
    {
        Processes.push_back(Process(p[i]));
    }
    std::sort(Processes.begin(), Processes.end());
    free(p);
    return 0;
}

int listProcessesStr(std::string &res)
{
    std::vector<Process> processes;
    if (listProcesses(processes) == -1)
    {
        perror("listProcesses");
        return -1;
    }
    std::ostringstream os;
    for (auto process: processes)
    {
        os << process.toString() << "\n";
    }
    res = os.str();
    return 0;
}