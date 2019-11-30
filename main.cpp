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
#include <map>
#include "tool.h"

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
        i = j;
    }
    return tokens;
}

// ------------------------------------------- Parser

// Abstract Syntax Tree的每个结点
struct SExpression {
    std::string val;
    std::vector<SExpression *> child;
    SExpression *parent;

    SExpression(std::string val, SExpression *parent) : val(std::move(val)), parent(parent) {}

    std::string toString() {
        if (val == "(") {
            std::string res = "(";
            for (auto item:child)
                res += item->toString();
            return res + ")";
        } else {
            return val;
        }
    }
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
    return *program->child.begin();
}

// ------------------------------------------- 作用域

/*
 * 用于保存定义的变量，通过parent，向前查找。
 */
class SScope {
public:
    // 包含内置操作
    std::map<std::string, void *> variableTable;
    SScope *parent;

    explicit SScope(SScope *parent) : parent(parent) {}

    void *find(std::string &name) {
        SScope *current = this;
        while (current != nullptr) {
            if (current->variableTable.find(name) != current->variableTable.end())
                return current->variableTable[name];
            current = current->parent;
        }
        throw name + " is not defined.";
    }

    void *define(std::string &name, void *value) {
        variableTable[name] = value;
        return value;
    }
};


// ------------------------------------------- 类型实现
// TODO 添加其他类型 数值，Bool，列表和函数 ，现在的处理是bool、int统统为int.
bool try_parse(std::string &val, int *num) {
    return is_digits(val, num);
}

void *evaluate(SScope *scope, SExpression *program) {
    SExpression *current = program;
    while (true) {
        // 处理字面量（Literals）： 3 ；和具名量（Named Values）： x
        if (current->child.empty()) {
            int *num = new int(0);
            if (try_parse(current->val, num))
                return num;
            else
                return scope->find(current->val);
        }

        // TODO 处理 def if begin func ...
        auto str = program->child[0]->val;
        if (str == "def")
            return scope->define(program->child[1]->val,
                                 evaluate(new SScope(scope), program->child[2]));
        else if (str == "if") {
            bool *condition = static_cast<bool *>(evaluate(new SScope(scope), program->child[1]));
            return (*condition) ? evaluate(new SScope(scope), program->child[2])
                                : evaluate(new SScope(scope), program->child[3]);
        } else if (str == "begin") {
            void *res = nullptr;
            for (auto i = program->child.begin() + 1; i != program->child.end(); ++i)
                res = evaluate(scope, *i);
            return res;
        }

            // 比较操作 >= > <= < =
        else if (str == ">=") {
            int *res = new int(*((int *) evaluate(new SScope(scope), program->child[1]))
                               >= *((int *) evaluate(new SScope(scope), program->child[2])));
            return (void *) res;
        } else if (str == ">") {
            int *res = new int(*((int *) evaluate(new SScope(scope), program->child[1]))
                               > *((int *) evaluate(new SScope(scope), program->child[2])));
            return (void *) res;
        } else if (str == "<=") {
            int *res = new int(*((int *) evaluate(new SScope(scope), program->child[1]))
                               <= *((int *) evaluate(new SScope(scope), program->child[2])));
            return (void *) res;
        } else if (str == "<") {
            int *res = new int(*((int *) evaluate(new SScope(scope), program->child[1]))
                               < *((int *) evaluate(new SScope(scope), program->child[2])));
            return (void *) res;
        } else if (str == "=") {
            int *res = new int(*((int *) evaluate(new SScope(scope), program->child[1]))
                               == *((int *) evaluate(new SScope(scope), program->child[2])));
            return (void *) res;
        }
            // 算术操作 + - * / %
        else if (str == "+") {
            int *res = new int(0);
            for (auto i = program->child.begin() + 1; i != program->child.end(); ++i)
                *res += *((int *) evaluate(new SScope(scope), *i));
            return (void *) res;
        } else if (str == "-") {
            int *res = new int(0);
            for (auto i = program->child.begin() + 2; i != program->child.end(); ++i)
                *res -= *((int *) evaluate(new SScope(scope), *i));
            *res += *((int *) evaluate(new SScope(scope), *(program->child.begin() + 1)));
            return (void *) res;
        } else if (str == "*") {
            int *res = new int(1);
            for (auto i = program->child.begin() + 1; i != program->child.end(); ++i)
                *res *= *((int *) evaluate(new SScope(scope), *i));
            return (void *) res;
        } else if (str == "/") {
            int *res = new int(1);
            for (auto i = program->child.begin() + 2; i != program->child.end(); ++i)
                *res *= *((int *) evaluate(new SScope(scope), *i));
            *res = *((int *) evaluate(new SScope(scope), *(program->child.begin() + 1))) / *res;
            return (void *) res;
        } else if (str == "%") {
            int *res = new int(*((int *) evaluate(new SScope(scope), program->child[1]))
                               % *((int *) evaluate(new SScope(scope), program->child[2])));
            return (void *) res;
        }
            // 逻辑操作 and ， or 和 not
        else if (str == "and") {
            int *res = new int(0);
            for (auto i = program->child.begin() + 1; i != program->child.end(); ++i)
                if (!*((int *) evaluate(new SScope(scope), *i)))
                    return (void *) res;
            *res = 1;
            return (void *) res;
        } else if (str == "or") {
            int *res = new int(1);
            for (auto i = program->child.begin() + 1; i != program->child.end(); ++i)
                if (*((int *) evaluate(new SScope(scope), *i)))
                    return (void *) res;
            *res = 0;
            return (void *) res;
        } else if (str == "not") {
            int *res = new int(!*((int *) evaluate(new SScope(scope), program->child[1])));
            return (void *) res;
        } else {
            throw "Undefined name:" + str;
        }

    }
}


void func(std::string text, SScope *scope) {
    std::string text1 = replaceAddWhite(std::move(text));
    auto tokens = getTokens(text1);
    auto program = parseAsIScheme(tokens);

    evaluate(scope, program);

    if (scope->variableTable.size() != 1)
        std::cout << "\n";
    for (const auto &item:scope->variableTable) {
        std::cout << item.first << "\t" << *((int *) item.second) << std::endl;
    }
}

int main() {
    std::cout << "是否打开终端交互模式：([y]/n)\n";
    char a;
    std::cin >> a;
    getchar();

    std::string text;
    auto *scope = new SScope(nullptr);

    if (a == 'n') {
        char fn[] = R"(C:\Users\ttp\CLionProjects\ny_pl0\a.txt)";
        text = readFileIntoString(fn);
        try {
            func(text, scope);
        } catch (std::exception &e) {
            std::cout << e.what();
        } catch (...) {
            std::cout << "other error\n";
        }
    } else {
        while (true) {
            printf(">>> ");
            char str[1000];
            std::cin.getline(str, 1000);
            text = str;
            if (text == "exit" || text == "q")
                return 0;

            bool flag = false;
            for (auto item:str)
                if (!is_whiteBlock(item))
                    flag = true;
            if (!flag) continue;

            printf(">>> ");
            try {
                func(text, scope);
            } catch (std::exception &e) {
                std::cout << e.what();
            } catch (...) {
                std::cout << "other error\n";
            }
        }
    }
    return 0;
}
