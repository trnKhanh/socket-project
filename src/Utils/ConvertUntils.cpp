#include "ConvertUtils.h"

uint64_t my_htonll(uint64_t value){
    uint64_t result;
    uint8_t* ptr_value = (uint8_t*)&value;
    uint8_t* ptr_result = (uint8_t*)&result;
    for(int i = 0; i < 8; ++i)
        ptr_result[i] = ptr_value[7 - i];
    return result;
}

uint64_t my_ntohll(uint64_t value){
    uint64_t result;
    uint8_t* ptr_value = (uint8_t*)&value;
    uint8_t* ptr_result = (uint8_t*)&result;
    for(int i = 0; i < 8; ++i)
        ptr_result[i] = ptr_value[7 - i];
    return result;
}