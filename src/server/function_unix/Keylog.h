#pragma once
#include <vector>
#include <iostream>
#include <ApplicationServices/ApplicationServices.h>

extern CFRunLoopRef event_loop;
extern std::vector<int> keylogfds;

void startKeylogHelper();
void stopKeylogHelper();