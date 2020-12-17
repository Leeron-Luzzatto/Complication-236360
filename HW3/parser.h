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
        else if(s == "string"){
            val = "STRING";
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

struct Type_var : public N{
    string type;
    // For set
    int start;
    int end;

    Type_var(const string &type = "", const string &start = "0", const string &End= "0") : type(type) {
       int s = stoi(start);
       int e = stoi(End);
       this->start = s;
       this->end = e;
    }
};


struct Expression : public N{
    string name;
    string type;
    bool bool_value;
    int number;
    string str;
    string op;
    Expression(const string& name = "", const string& type = "", bool bool_value = false, int number = 0, const string& str = "", const string& op = "")
            : name(name), type(type), bool_value(bool_value), number(number), str(str), op(op) {}
};
#endif //HW3_PARSER_H