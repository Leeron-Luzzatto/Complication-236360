//
// Created by adirr on 14/12/2020.
//

#ifndef HW3_PARSER_H
#define HW3_PARSER_H

#include <string>
using namespace std;

struct N{
};

struct Node : public N{
    string val;
    Node(const string& s){
        if (s == "void") {
            val = "VOID";
        } else if (s == "bool") {
            val = "BOOL";
        } else if (s == "int") {
            val = "INT";
        } else if (s == "byte") {
            val = "BYTE";
        } else if(s == "set"){
            val = "SET";
        }
        else
            val = s;
    }
};

struct Funcs : public N{
    Funcs() {}
};

struct FormalsList : public N{
    N* arg;
    N* next;

    FormalsList(N* arg = nullptr, N* next = nullptr) : arg(arg), next(next) {}
};

struct Argument : public N{
    N* type;
    N* name;
    N* value;

    Argument(N* type, N* name, N* value = nullptr) : type(type), name(name), value(value) {}
};

struct Statements : public N{
    N* statement;
    N* next;

    Statements(N* statement = nullptr, N* next = nullptr) : statement(statement), next(next) {}
};


#endif //HW3_PARSER_H
