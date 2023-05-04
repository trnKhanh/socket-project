#include "Process.h"
#include <sstream>
Process::Process(const kinfo_proc &k)
{
    this->pid = k.kp_proc.p_pid;
    proc_pidpath(this->pid, this->name, sizeof(this->name));
}
bool Process::operator<(const Process &a) const
{
    return this->pid < a.pid;
}
std::ostream& operator << (std::ostream &os, const Process &p)
{
    os << p.pid << ": " << p.name;
    return os;
}
std::string Process::toString()
{
    std::ostringstream os;
    os << this->pid << ": " << this->name;
    return os.str();   
}