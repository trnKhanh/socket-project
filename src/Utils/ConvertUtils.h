#pragma once

#include <stdint.h>
#include <string>

using std::string;

uint64_t my_htonll(uint64_t value);
uint64_t my_ntohll(uint64_t value);

void my_itos(char* res, int n);
int my_stoi(char* res);
int my_stoi_rev(char* res);

void string_to_listchar(char*& res, string s);