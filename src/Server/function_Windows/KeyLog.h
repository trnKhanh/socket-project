#pragma once

#include <Windows.h>
#include <string>

using std::string;
using std::pair;

pair <string, bool> changeToKeyPress(DWORD dwKeyCode); // get key press from DWORD
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);