/*
 * �ο����£� http://zh.lucida.me/blog/how-to-implement-an-interpreter-in-csharp/
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

// ------------------------------------------- �ʷ�����

// ��(,)��ӿո�
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

// ��ȡtokens,ֱ���÷ָ����ָ�
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

// Abstract Syntax Tree��ÿ�����
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
 * ���������ţ�����һ���µĽڵ㵽��ǰ�ڵ㣨 current ����Ȼ�����赱ǰ�ڵ㡣
 * ���������ţ����˵���ǰ�ڵ�ĸ��ڵ㡣
 * �����Ϊ��ǰ���ش����ڵ㣬��ӵ���ǰ�ڵ��С�
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

// ------------------------------------------- ������

/*
 * ���ڱ��涨��ı�����ͨ��parent����ǰ���ҡ�
 */
struct SScope {
    // �������ò���
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

// ------------------------------------------- ����ʵ��
// TODO û���ͷ��ڴ棬û�жԺܶ������м�顣���ڵĴ�����bool��intͳͳΪint. û�и�����
struct SList {
    std::vector<void *> vec;
};

struct SFunction {
    SExpression *body;
    std::vector<std::string> parameters;
    SScope *scope;

    SFunction(SExpression *body, std::vector<std::string> parameters, SScope *scope)
            : body(body), parameters(std::move(parameters)), scope(scope) {}
};

bool try_parse(std::string &val, int *num) {
    return is_digits(val, num);
}

// TODO ���Կ������append �͸߽׺���
void *evaluate(SScope *scope, SExpression *program) {
    SExpression *current = program;
    while (true) {
        // ������������Literals���� 3 ���;�������Named Values���� x
        if (current->child.empty()) {
            int *num = new int(0);
            if (try_parse(current->val, num))
                return num;
            else
                return scope->find(current->val);
        }

        auto str = program->child[0]->val;
        if (str == "def")
            return scope->define(program->child[1]->val,
                                 evaluate(new SScope(scope), program->child[2]));
        else if (str == "if") {
            bool *condition = static_cast<bool *>(evaluate(scope, program->child[1]));
            return (*condition) ? evaluate(scope, program->child[2])
                                : evaluate(scope, program->child[3]);
        } else if (str == "begin") {
            void *res = nullptr;
            for (auto i = program->child.begin() + 1; i != program->child.end(); ++i)
                res = evaluate(scope, *i);
            return res;
        } else if (str == "func") {
            SExpression *body = program->child[2];
            auto *newScope = new SScope(scope);
            SExpression *para = program->child[1];
            std::vector<std::string> parameters;
            for (auto item:para->child)
                parameters.push_back(item->val);
            return new SFunction(body, parameters, newScope);
        }
            // Scheme ���б�������� first �� rest �� empty? �� append
        else if (str == "list") {
            auto *sList = new SList;
            for (auto i = program->child.begin() + 1; i != program->child.end(); ++i) {
                void *res = evaluate(scope, *i);
                sList->vec.push_back(res);
            }
            return sList;
        } else if (str == "first") {
            int *res = new int;
            SList *sList = (SList *) evaluate(scope, *(++program->child.begin()));
            *res = *((int *) sList->vec[0]);
            return res;
        } else if (str == "rest") {
            auto *resSList = new SList();
            SList *sList = (SList *) evaluate(scope, *(++program->child.begin()));
            for (int i = 1; i < sList->vec.size(); i++)
                resSList->vec.push_back(sList->vec[i]);
            return resSList;
        } else if (str == "empty?") {
            int *res = new int;
            SList *sList = (SList *) evaluate(scope, *(++program->child.begin()));
            *res = (int) sList->vec.empty();
            return res;
        }
            // �Ƚϲ��� >= > <= < =
        else if (str == ">=") {
            int *res = new int(*((int *) evaluate(scope, program->child[1]))
                               >= *((int *) evaluate(scope, program->child[2])));
            return (void *) res;
        } else if (str == ">") {
            int *res = new int(*((int *) evaluate(scope, program->child[1]))
                               > *((int *) evaluate(scope, program->child[2])));
            return (void *) res;
        } else if (str == "<=") {
            int *res = new int(*((int *) evaluate(scope, program->child[1]))
                               <= *((int *) evaluate(scope, program->child[2])));
            return (void *) res;
        } else if (str == "<") {
            int *res = new int(*((int *) evaluate(scope, program->child[1]))
                               < *((int *) evaluate(scope, program->child[2])));
            return (void *) res;
        } else if (str == "=") {
            int *res = new int(*((int *) evaluate(scope, program->child[1]))
                               == *((int *) evaluate(scope, program->child[2])));
            return (void *) res;
        }
            // �������� + - * / %
        else if (str == "+") {
            int *res = new int(0);
            for (auto i = program->child.begin() + 1; i != program->child.end(); ++i)
                *res += *((int *) evaluate(scope, *i));
            return (void *) res;
        } else if (str == "-") {
            int *res = new int(0);
            for (auto i = program->child.begin() + 2; i != program->child.end(); ++i)
                *res -= *((int *) evaluate(scope, *i));
            *res += *((int *) evaluate(scope, *(program->child.begin() + 1)));
            return (void *) res;
        } else if (str == "*") {
            int *res = new int(1);
            for (auto i = program->child.begin() + 1; i != program->child.end(); ++i)
                *res *= *((int *) evaluate(scope, *i));
            return (void *) res;
        } else if (str == "/") {
            int *res = new int(1);
            for (auto i = program->child.begin() + 2; i != program->child.end(); ++i)
                *res *= *((int *) evaluate(scope, *i));
            *res = *((int *) evaluate(scope, *(program->child.begin() + 1))) / *res;
            return (void *) res;
        } else if (str == "%") {
            int *res = new int(*((int *) evaluate(scope, program->child[1]))
                               % *((int *) evaluate(scope, program->child[2])));
            return (void *) res;
        }
            // �߼����� and �� or �� not
        else if (str == "and") {
            int *res = new int(0);
            for (auto i = program->child.begin() + 1; i != program->child.end(); ++i)
                if (!*((int *) evaluate(scope, *i)))
                    return (void *) res;
            *res = 1;
            return (void *) res;
        } else if (str == "or") {
            int *res = new int(1);
            for (auto i = program->child.begin() + 1; i != program->child.end(); ++i)
                if (*((int *) evaluate(scope, *i)))
                    return (void *) res;
            *res = 0;
            return (void *) res;
        } else if (str == "not") {
            int *res = new int(!*((int *) evaluate(scope, program->child[1])));
            return (void *) res;
        } else {
            // ����Ϊ�Զ��庯��
            // �Ǿ����������ã�((func (x) (* x x)) 3)
            // �����������ã�(square 3)
            auto *function = (SFunction *) ((str == "(") ? evaluate(scope, program) : scope->find(str));
            std::vector<void *> arguments;
            for (auto i = program->child.begin() + 1; i != program->child.end(); ++i)
                arguments.push_back(evaluate(scope, *i));
            auto *newScope = new SScope(scope);
            for (int i = 0; i < function->parameters.size() && i < arguments.size(); i++)
                newScope->variableTable[function->parameters[i]] = arguments[i];
            return evaluate(newScope, function->body);
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

// TODO ע���޸�����ļ����ļ���ʽΪÿ�ζ�ȡһ�У�һ���ļ��Ĵ��빲��һ��������Ĭ�Ͻ��뽻��ģʽ������n�����ȡ�ļ�ģʽ�� ����ģʽ�£�����һ��������ÿ�ζ��������һ�С�
int main() {
    std::cout << "�Ƿ���ն˽���ģʽ��([y]/n)\n";
    char a;
    std::cin >> a;
    getchar();

    std::string text;
    auto *scope = new SScope(nullptr);
    std::vector<std::string> strLin;
    int index = 0;
    if (a == 'n') {
        char fn[] = R"(C:\Users\ttp\CLionProjects\ny_pl0\a.txt)";
        text = readFileIntoString(fn);
        strLin = split(text, '\n');
    }

    while (true) {
        if (a == 'n') {
            try {
                func(strLin[index++], scope);
            } catch (std::exception &e) {
                std::cout << e.what();
            } catch (...) {
                std::cout << "other error\n";
            }
            if (index == strLin.size())
                return 0;
        } else {
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
}

