#ifdef __APPLE__

#pragma once
#include <string>
#include <sstream>

int listDirTreeHelper(const char *pathName, std::string &res);

int dfs(const char *pathName, std::ostringstream &os, int depth);

#endif