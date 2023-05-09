#pragma once

#include <Windows.h>
#include <string>
#include <set>

int listAppHelper(std::string &res);
int startAppHelper(const std::string& appName);
int stopAppHelper(const std::string& appName);

DWORD GetProcessIdByName(const char* processName);
void EnumerateInstalledApplications(std::set <std::string>& s, HKEY key, const char* path);