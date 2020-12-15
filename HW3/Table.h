//
// Created by adirr on 15/12/2020.
//

#ifndef HW3_TABLE_H
#define HW3_TABLE_H
#include "hw3_output.hpp"
#include "parser.h"
#include <string>
#include <vector>
using namespace std;

class Entry{
public:
    string name;
    string type;
    int offset;
    bool is_function;

    Entry(const string &name, const string &type, int offset, bool is_function=false) : name(name), type(type),
                                                                offset(offset), is_function(is_function) {}
};

class Scope{
public:
    vector<Entry*>* entries;
    int max_offset;
    bool while_scope;

    Scope(int maxOffset, bool whileScope = false) : max_offset(maxOffset), while_scope(whileScope) {

    }

    void addEntry(const string& name, const string& type, bool is_function = false){
        auto* e = new Entry(name, type, max_offset, is_function);
        entries->push_back(e);
        max_offset++;
    }

    void addEntry(const string& name, const string& type, int offset, bool is_function = false){
        auto* e = new Entry(name, type, offset);
        entries->push_back(e);
        //We use this to add function parameters to table, so no need to increase max_offset
    }

    bool isInScope(const string& id){
        for (Entry* e : *(entries)){
            if(e->name == id){
                return true;
            }
        }
        return false;
    }


};


class SymbolTable{
public:
    vector<Scope*>* table;
    int max_offset;
    bool main_found = false;
    SymbolTable() {
        table = new vector<Scope*>();
        max_offset = 0;
        auto* global = new Scope(max_offset);
        //add print to table
        string name = "print";
        string retType = "VOID";
        vector<string> argTypes = {"STRING"};
        string funcType = output::makeFunctionType(retType, argTypes);
        global->addEntry(name, funcType, 0, true);

        //add printi
        name = "printi";
        argTypes = {"INT"};
        funcType = output::makeFunctionType(retType, argTypes);
        global->addEntry(name, funcType, 0, true);

        table->push_back(global);
    }
    void ScopeEnd(){
        if (!main_found){
            output::errorMainMissing();
            exit(0);
        }
        output::endScope();
        Scope* s = table->back();
        for(Entry* e : *(s->entries)){
            output::printID(e->name, e->offset, e->type);
        }
        max_offset-=s->max_offset;
        table->pop_back();
    }
    void addFunction(N* rT, N* n, N* aT){
        string func_name = ((Node*)n)->val;
        if (func_name == "main"){
            main_found = true;
        }
        string retType = ((Node*)rT)->val;
        FormalsList* args = ((FormalsList*)aT);
        args->arg == nullptr ? args = nullptr : args = args;

        vector<string> argTypes = vector<string>();
        vector<string> argNames = vector<string>();

        while(args!= nullptr){
            string var_type = ((Node*)(((Argument*)args)->type))->val;
            argTypes.push_back(var_type);
            string var_name = ((Node*)(((Argument*)args)->name))->val;
            argNames.push_back(var_name);
            args = (FormalsList*)args->next;
        }
        string funcType = output::makeFunctionType(retType, argTypes);
        table->back()->addEntry(func_name, funcType, 0, true);

        auto* new_scope = new Scope(max_offset);
        int i = -1;
        for (int j=0; j<argNames.size(); j++){
            new_scope->addEntry(argNames[j], argTypes[j], i);
            i--;
        }
        table->push_back(new_scope);
    }
    void addVar(N* type, N* name){
        Scope* scope = table->back();
        scope->addEntry(((Node*)name)->val, ((Node*)type)->val, max_offset);
        max_offset++;
    }
    void newScope(bool is_while = false){
        auto* scope = new Scope(max_offset, is_while);
        table->push_back(scope);
    }
    bool isVarDeclared(N* n){
        string name = ((Node*)n)->val;
        for(Scope* scope : *table){
            if(scope->isInScope(name)){
                return true;
            }
        }
        return false;
    }
};
#endif //HW3_TABLE_H
