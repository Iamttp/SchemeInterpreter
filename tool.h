//
// Created by ttp on 2019/11/30.
//

#ifndef NY_PL0_TOOL_H
#define NY_PL0_TOOL_H

#include <string>

//从文件读入到string里
std::string readFileIntoString(char *filename) {
    std::ifstream ifile(filename);
    std::ostringstream buf;
    char ch;
    while (buf && ifile.get(ch))
        buf.put(ch);
    return buf.str();
}

// 判断是否为分隔符
inline bool is_whiteBlock(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

std::vector<std::string> split(std::string &str, char c) {
    std::vector<std::string> res;
    int j = 0;
    int i = 0;
    for (; i < str.size(); i++)
        if (str[i] == c) {
            res.push_back(str.substr(j, i - j));
            j = i + 1;
            i = j;
        }
    auto subStr = str.substr(j, i - j);
    if (!subStr.empty())
        res.push_back(subStr);
    return res;
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
