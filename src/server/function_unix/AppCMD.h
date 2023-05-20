#ifdef __APPLE__

#pragma once
#include <string>

int listAppHelper(std::string &res);
int startAppHelper(const std::string &appName);
int stopAppHelper(const std::string &appName);

#endif