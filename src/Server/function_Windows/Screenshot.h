#pragma once

#include <Windows.h>
#include <vector>

int screenshotHelper(std::vector<char> &buffer);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);