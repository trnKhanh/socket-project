#include <iostream>
#include <Windows.h>

int main() {
    CHAR executablePath[MAX_PATH];

    HINSTANCE hInstance = FindExecutable("calc.exe", NULL, executablePath);
    if (hInstance <= HINSTANCE(32)) {
        std::cerr << "Failed to find the executable." << std::endl;
        return 1;
    }

    // Start the application using the retrieved executable path
    HINSTANCE hInstance2 = ShellExecute(NULL, "open", executablePath, NULL, NULL, SW_SHOW);
    if (hInstance2 <= HINSTANCE(32)) {
        std::cerr << "Failed to start the application." << std::endl;
        return 1;
    }

    std::cout << "Application started successfully!" << std::endl;

    return 0;
}