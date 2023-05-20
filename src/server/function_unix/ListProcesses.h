#ifdef __APPLE__

#pragma once
#include "Objects/Process.h"
#include <vector>
#include <string>

int listProcessesHelper(std::vector<Process> &Processes);
int listProcessesStrHelper(std::string &res);
int getProcessByPIDHelper(int PID, Process &res);

#endif