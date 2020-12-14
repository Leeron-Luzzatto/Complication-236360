//
// Created by adirr on 14/12/2020.
//

#ifndef HW3_PARSER_H
#define HW3_PARSER_H

#include <string>
using namespace std;

class Node{
public:
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
        } else
            val = s;
    }

};

#endif //HW3_PARSER_H
