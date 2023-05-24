#ifdef __APPLE__

#include "Process.h"
#include <sstream>
Process::Process()
{
    this->_pid = 0;
    this->_name[0] = 0;
}
Process::Process(const kinfo_proc &k)
{
    this->_pid = k.kp_proc.p_pid;
    proc_pidpath(this->_pid, this->_name, sizeof(this->_name));
}
bool Process::operator<(const Process &a) const
{
    return this->_pid < a._pid;
}
std::ostream& operator << (std::ostream &os, const Process &p)
{
    os << p._pid << ": " << p._name;
    return os;
}
std::string Process::toString()
{
    std::ostringstream os;
    os << this->_pid << ": " << this->_name;
    return os.str();   
}

#endif