#pragma once
#include <sys/sysctl.h>
#include <libproc.h>
#include <iostream>
#include <string>

class Process {
    int pid;
    char name[PROC_PIDPATHINFO_MAXSIZE];
public:
    Process(const kinfo_proc &);
    bool operator < (const Process &a) const;

    friend std::ostream& operator << (std::ostream &os, const Process &p);

    std::string toString();
};