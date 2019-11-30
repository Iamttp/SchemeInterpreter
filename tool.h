//
// Created by ttp on 2019/11/30.
//

#ifndef NY_PL0_TOOL_H
#define NY_PL0_TOOL_H

#include <string>

// 判断是否为分隔符
inline bool is_whiteBlock(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

bool is_digits(std::string val, int *num) {
    int temp_num = 0;
    for (auto item:val) {
        temp_num *= 10;
        if ('0' <= item && item <= '9')
            temp_num += item - '0';
        else
            return false;
    }
    *num = temp_num;
    return true;
}

#endif //NY_PL0_TOOL_H
