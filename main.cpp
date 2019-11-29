#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

std::string replaceAddWhite(std::string text) {
    std::string resText;
    int len = text.size();
    for (int i = 0; i < len; ++i) {
        if (text[i] == '(')
            resText += " ( ";
        else if (text[i] == ')')
            resText += " ) ";
        else
            resText += text[i];
    }
    return resText;
}

inline bool is_whiteBlock(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

std::vector<std::string> getTokens(std::string text) {
    std::vector<std::string> tokens;
    int len = text.size();
    for (int i = 0; i < len; ++i) {
        if(is_whiteBlock(text[i]))
            continue;
        int j = i;
        while (!is_whiteBlock(text[j]) && j < len) {
            j++;
        }
        std::string token = text.substr(i, j - i);
        tokens.push_back(token);
        i = j++;
    }
    return tokens;
}

//从文件读入到string里
std::string readFileIntoString(char *filename) {
    std::ifstream ifile(filename);
    std::ostringstream buf;
    char ch;
    while (buf && ifile.get(ch))
        buf.put(ch);
    return buf.str();
}

int main() {
    char *fn = "C:\\Users\\ttp\\CLionProjects\\ny_pl0\\a.txt";
    std::string text;
    text = readFileIntoString(fn);

    std::string text1 = replaceAddWhite(text);
    auto tokens = getTokens(text1);

    std::cout << tokens.size() << std::endl;
    for (const auto &item:tokens)
        std::cout << item << std::endl;

    return 0;
}
