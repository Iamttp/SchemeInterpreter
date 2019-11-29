/*
 * 参考文章： http://zh.lucida.me/blog/how-to-implement-an-interpreter-in-csharp/
 */

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <fstream>
#include <sstream>
#include <list>

// ------------------------------------------- 词法分析

// 给(,)添加空格
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

// 判断是否为分隔符
inline bool is_whiteBlock(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

// 获取tokens,直接用分隔符分割
std::vector<std::string> getTokens(std::string text) {
    std::vector<std::string> tokens;
    int len = text.size();
    for (int i = 0; i < len; ++i) {
        if (is_whiteBlock(text[i]))
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

// ------------------------------------------- Parser

// Abstract Syntax Tree的每个结点
struct SExpression {
    std::string val;
    std::list<SExpression *> child;
    SExpression *parent;

    SExpression(std::string val, SExpression *parent) : val(std::move(val)), parent(parent) {}
};

/*
 * 碰到左括号，创建一个新的节点到当前节点（ current ），然后重设当前节点。
 * 碰到右括号，回退到当前节点的父节点。
 * 否则把为当前词素创建节点，添加到当前节点中。
 */
SExpression *parseAsIScheme(const std::vector<std::string> &token) {
    auto *program = new SExpression("", nullptr);
    auto *current = program;
    for (const auto &lex:token) {
        if (lex == "(") {
            auto *newNode = new SExpression("(", current);
            current->child.push_back(newNode);
            current = newNode;
        } else if (lex == ")") {
            current = current->parent;
        } else {
            current->child.push_back(new SExpression(lex, current));
        }
    }
    return program;
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

    parseAsIScheme(tokens);

    return 0;
}
