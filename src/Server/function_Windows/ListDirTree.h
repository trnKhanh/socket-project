#pragma once

#include <string>

using std::string;

int listDirTreeHelper(const char* path, string& buffer);
string listDirectoryTree(const char* path, int indent);