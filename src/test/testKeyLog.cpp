#include "../Utils/ConvertUtils.h"
#include <iostream>
#include <Windows.h>
#include <thread>
#include <future>
#include <chrono>

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
    int i = 0;
// Hàm hook xử lý sự kiện nhấn phím
HHOOK hHook;
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* pKeyboardStruct = (KBDLLHOOKSTRUCT*)lParam;
        DWORD vkCode = pKeyboardStruct->vkCode;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
        {
            // std::cerr << i << std::endl;
            // Xử lý sự kiện nhấn phím ở đây
            auto res = changeToKeyPress(vkCode);
            if(res.second == true)
            std::cout << "Key pressed: " << res.first << std::endl;
        }
    }

    // Gọi hook tiếp theo trong chuỗi hook
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}
int foo(std::future<void> future)
{
    // Thiết lập hook
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, NULL, 0);
    if (hHook == NULL)
    {
        std::cerr << "Failed to set keyboard hook." << std::endl;
        return 1;
    }

    // Lặp chờ sự kiện
    MSG msg;
    while(future.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout)
    {
        // std::cout << -1 << std::endl;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    // Hủy hook
    UnhookWindowsHookEx(hHook);

    return 0;
}
int main()
{
    std::promise<void> promise;
    std::thread t(foo, promise.get_future());
    std::cout << 1234 << std::endl;
    //PostThreadMessage(GetThreadId((HANDLE)t.native_handle()), WM_QUIT, (WPARAM)NULL, (LPARAM)NULL);
    std::this_thread::sleep_for(std::chrono::seconds(10));
    promise.set_value();
    t.join();
}

// #include <iostream>
// #include <thread>
// #include <atomic>
// #include <Windows.h>

// std::atomic<bool> terminateFlag(false);

// // Function to run in the separate thread
// void TerminationThread()
// {
//     // Wait for a certain condition to be met
//     // For example, wait for user input or any other termination condition
//     std::string input;
//     std::cout << "Enter 'quit' to terminate: ";
//     std::cin >> input;

//     // Set the terminate flag to true to signal the main thread to exit the loop
//     terminateFlag.store(true);

//     // Alternatively, you can post a WM_QUIT message to the message queue
//     // PostQuitMessage(0);
// }

// int main()
// {
//     // Create the termination thread
//     std::thread terminationThread(TerminationThread);

//     // Main message loop
//     MSG msg;
//     while (!terminateFlag && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
//     {
//         TranslateMessage(&msg);
//         DispatchMessage(&msg);
//     }

//     // Join the termination thread to ensure it has finished
//     terminationThread.join();

//     // Program termination
//     return (int)msg.wParam;
// }