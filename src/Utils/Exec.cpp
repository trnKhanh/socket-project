#include "Exec.h"

int exec(const char* cmd, std::string &res) {
    char buffer[128];
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        return -1;
    }
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        res += buffer;
    }
    return 0;
}