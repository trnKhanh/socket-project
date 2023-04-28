#include "function_unix/ListProcesses.h"
#include <algorithm>

int main(int argc, char *argv[])
{
    vector<Process> processes;
    listProcesses(processes);
    sort(processes.begin(), processes.end());
    for (Process p: processes)
        cout << p << "\n";
}