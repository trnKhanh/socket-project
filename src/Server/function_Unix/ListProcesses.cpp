#ifdef __APPLE__

#include "ListProcesses.h"
#include <sys/sysctl.h>
#include <stdio.h>
#include <cstdlib>
#include <algorithm>
#include <sstream>

int listProcessesHelper(std::vector<Process> &Processes)
{
    // name to get all processes
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
        free(p);
        return -1;
    }
    // p contains the information about all process
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

int listProcessesStrHelper(std::string &res)
{
    std::vector<Process> processes;
    if (listProcessesHelper(processes) == -1)
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

int getProcessByPIDHelper(int PID, Process &res)
{
    int name[] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, PID};
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
        free(p);
        return -1;
    }
    res = Process(*p);
    free(p);
    return 0;
}

#endif