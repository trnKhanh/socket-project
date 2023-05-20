#pragma once

#include <iostream>

using std::cout;
using std::cin;

int menu(){
    cout << "[[MENU]]\n";
    cout << "1. List app.\n";
    cout << "2. List process\n";
    cout << "3. Screen Capture.\n";
    cout << "4. Keyboard Capture.\n";
    cout << "5. DFS.\n";
    cout << "6. Exit.\n";
    int type;
    cin >> type;
    return type;
}