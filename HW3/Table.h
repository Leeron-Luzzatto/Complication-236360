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
extern int yylineno;


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

    bool isInScope(const string& id, bool is_function = false){
        for (Entry* e : *(entries)){
            if(e->name == id){
                if (is_function){
                    if(!e->is_function) {
                        continue;
                    }
                }
                else{
                    if(e->is_function){
                        continue;
                    }
                }
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
    void FinalScopeEnd(){
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
    void ScopeEnd(){
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
        string retType = ((Type_var*)rT)->type;
        FormalsList* args = ((FormalsList*)aT);
        args->arg == nullptr ? args = nullptr : args = args;

        if (func_name == "main" && retType == "VOID" && args == nullptr){
            main_found = true;
        }

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
        if(isVarDeclared(name) || isFuncDeclared(name)){
            output::errorDef(yylineno, ((Node*)name)->val);
            exit(0);
        }
        scope->addEntry(((Node*)name)->val, ((Type_var*)type)->type, max_offset);
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
    bool isFuncDeclared(N* n){
        string name = ((Node*)n)->val;
        for(Scope* scope : *table){
            if(scope->isInScope(name, true)){
                return true;
            }
        }
        return false;
    }
    string getVarType(N* n){
        string name = ((Node*)n)->val;
        if(!isVarDeclared(n)){
            output::errorUndef(yylineno, name);
            exit(0);
        }
        //We know variable exits
        for(Scope* scope : *table){
            for(Entry* e : *(scope->entries)){
                if(e->name == name){
                    //Found it, return type
                    return e->type;
                }
            }
        }
    }
    void checkValidAssign(N* id, N* exp){
        string var_type = getVarType(id);
        // Var exists, check types
        string exp_type = ((Expression*)exp)->type;
        //Check types are correct
        if(var_type == exp_type){}
        else if(var_type == "INT" && exp_type == "BYTE"){}
        else{
            output::errorMismatch(yylineno);
            exit(0);
        }
        //TO DO: need to add check that byte or set are in valid range
        //check if byte or
    }

};
#endif //HW3_TABLE_H
