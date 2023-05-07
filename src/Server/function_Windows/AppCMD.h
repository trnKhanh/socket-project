#pragma once

#include <Windows.h>
#include <string>

using std::string;

int listAppHelper(string &res);
int startAppHelper(const string& appName);
int stopAppHelper(const string& appName);

DWORD GetProcessIdByName(const char* processName);