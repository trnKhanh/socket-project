#include "ListProcesses.h"
#include <sys/sysctl.h>
#include <stdio.h>
#include <cstdlib>

void listProcesses(vector<Process> &Processes)
{
    int name[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    kinfo_proc *p;
    size_t len;
    // get total size of all processes 
    if (sysctl(name, sizeof(name)/sizeof(*name), NULL, &len, NULL, 0) == -1)
    {
        perror("sysctl");
        return;
    } 
    p = (kinfo_proc *)malloc(len);
    // get all process
    if (sysctl(name, sizeof(name)/sizeof(*name), p, &len, NULL, 0) == -1)
    {
        perror("sysctl");
        return;
    }
    int processNumber = len / sizeof(kinfo_proc);
    Processes.clear();
    for (int i = 0; i < processNumber; ++i)
    {
        Processes.push_back(Process(p[i]));
    }
    free(p);
}