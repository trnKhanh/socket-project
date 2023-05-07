#pragma once

#include <stdint.h>
#include <string>

using std::string;
using std::pair;

uint64_t my_htonll(uint64_t value); // host to network (uint64_t)
uint64_t my_ntohll(uint64_t value); // network to host (uint64_t)

void my_itos(char* res, int n); // convert integer to char*
int my_stoi(char* res); // convert char* to integer
int my_stoi_rev(char* res); // conver char* to integer but reverse digit

void string_to_listchar(char*& res, string s); // convert string to char*