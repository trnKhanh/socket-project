#include "ConvertUtils.h"

#include <cstring>

// host to network (uint64_t)
uint64_t my_htonll(uint64_t value){
    uint64_t result;
    uint8_t* ptr_value = (uint8_t*)&value;
    uint8_t* ptr_result = (uint8_t*)&result;
    for(int i = 0; i < 8; ++i)
        ptr_result[i] = ptr_value[7 - i];
    return result;
}

// network to host (uint64_t)
uint64_t my_ntohll(uint64_t value){
    uint64_t result;
    uint8_t* ptr_value = (uint8_t*)&value;
    uint8_t* ptr_result = (uint8_t*)&result;
    for(int i = 0; i < 8; ++i)
        ptr_result[i] = ptr_value[7 - i];
    return result;
}

// convert integer to char*
void my_itos(char* res, int n){
    int i = 0;
    while(n){
        res[i++] = char(n % 10 + '0');
        n /= 10;
    }
    res[i] = '\0';
}

// convert char* to integer
int my_stoi(char* res){
    int num = 0;
    int sz = strlen(res);
    for(int i = 0; i < sz; ++i)
        if(res[i] >= '0' && res[i] <= '9')
            num = num * 10 + int(res[i] - '0');
    return num;
}

// conver char* to integer but reverse digit
int my_stoi_rev(char* res){
    int num = 0;
    int sz = strlen(res);
    for(int i = sz; i >= 0; --i)
        if(res[i] >= '0' && res[i] <= '9')
            num = num * 10 + int(res[i] - '0');
    return num;
}

// convert string to char*
void string_to_listchar(char*& res, string s){
    int sz = s.size();
    res = (char*)malloc(sz + 1);
    for(int i = 0; i < sz; ++i)
        res[i] = s[i];
    res[sz] = '\0';
}