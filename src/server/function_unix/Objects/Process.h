#ifdef __APPLE__

#pragma once
#include <sys/sysctl.h>
#include <libproc.h>
#include <iostream>
#include <string>

class Process {
    int _pid;
    char _name[PROC_PIDPATHINFO_MAXSIZE];
public:
    Process();
    Process(const kinfo_proc &);
    bool operator < (const Process &a) const;

    friend std::ostream& operator << (std::ostream &os, const Process &p);

    std::string toString();
};

#endif