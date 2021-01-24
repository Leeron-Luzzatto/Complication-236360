//
// Created by adirr on 15/12/2020.
//

#ifndef HW3_TABLE_H
#define HW3_TABLE_H
#include "hw3_output.hpp"
#include "parser.hpp"
#include "bp.hpp"
#include "utils.h"
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
    string ret_type;
    vector<string> function_args;

    Entry(const string &name, const string &type, int offset, bool isFunction, const string &retType,
          const vector<string> &functionArgs) : name(name), type(type), offset(offset), is_function(isFunction),
                                                ret_type(retType), function_args(functionArgs) {}

    Entry(const string &name, const string &type, int offset, bool is_function=false, const string &ret_type="None") :
            name(name), type(type), offset(offset), is_function(is_function), ret_type(ret_type) {}
};

class Scope{
public:
    vector<Entry*>* entries;
    int max_offset;
    bool while_scope;

    Scope(int maxOffset, bool whileScope = false) : max_offset(maxOffset), while_scope(whileScope) {
        entries = new vector<Entry*>();
    }

    void addEntry(const string& name, const string& type, bool is_function = false, const string &ret_type = "None"){
        auto* e = new Entry(name, type, max_offset, is_function, ret_type);
        entries->push_back(e);
        max_offset++;

//        printf("Offset increased to %d by %s\n", max_offset, name.c_str());
    }

    void addEntry(const string& name, const string& type, int offset, const vector<string> &functionArgs, bool is_function = false,
                  const string &ret_type = "None"){
        auto* e = new Entry(name, type, offset, is_function, ret_type, functionArgs);
        entries->push_back(e);
        //We use this to add function parameters to table, so no need to increase max_offset
    }

    void addFuncVar(const string& name, const string& type, int offset){
        auto* e = new Entry(name, type, offset, false, "None");
        entries->push_back(e);
    }

    void addVar(const string& name, const string& type, int offset){
        auto* e = new Entry(name, type, offset, false, "None");
        entries->push_back(e);
        max_offset++;

//        printf("Offset increased to %d by %s\n", max_offset, name.c_str());
    }

    bool isInScope(const string& id, bool is_function = false){
//        for (Entry* e : *(entries)){
//        printf("%d\n",entries->size());
//        for (auto e = *(entries)->rbegin(); e!=*(entries)->rend();++e){

//        if(entries == nullptr){
//            printf("Entris is null...\n");
//        }

        for(int i=entries->size() - 1; i>=0; i--){
            Entry* e = (*entries)[i];
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

    string getScopeFuncRet(){
//        for (auto e = *(entries)->rbegin(); e!=*(entries)->rend();++e){
        for(int i = entries->size()-1; i>=0; i--){
            Entry* e = (*entries)[i];
            if(e->is_function){
                return e->ret_type;
            }
        }
        return "None";
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
        global->addEntry(name, funcType, 0, argTypes, true, retType);

        //add printi
        name = "printi";
        argTypes = {"INT"};
        funcType = output::makeFunctionType(retType, argTypes);
        global->addEntry(name, funcType, 0, argTypes, true, retType);

        table->push_back(global);
    }
    void FinalScopeEnd(){
        if (!main_found){
            output::errorMainMissing();
            exit(0);
        }
        //output::endScope();
        Scope* s = table->back();
//        for(Entry* e : *(s->entries)){
        for(int i=0; i<s->entries->size(); i++){
            Entry* e = (*(s->entries))[i];
//            output::printID(e->name, e->offset, e->type);
        }
        max_offset-=s->max_offset;
        table->pop_back();
    }
    void ScopeEnd(){
//        output::endScope();
        Scope* s = table->back();
        for(int i=0; i<s->entries->size(); i++){
            Entry* e = (*(s->entries))[i];
            //output::printID(e->name, e->offset, e->type);
            if(e->offset >= 0){
                max_offset--;
            }

        }
//        max_offset-=s->max_offset;

//        printf("Global offset deacreased to %d\n", max_offset);

        table->pop_back();
    }
    void addFunction(N* rT, N* n, N* aT){
        if(isVarDeclared(n) || isFuncDeclared(n)){
            output::errorDef(yylineno, ((Node*)n)->val);
            exit(0);
        }
        string func_name = ((Node*)n)->val;
        string retType = ((Type_var*)rT)->type;
        FormalsList* args = ((FormalsList*)aT);

        vector<string> argTypes = args->types;
        vector<string> argNames = args->names;

        if (func_name == "main" && retType == "VOID" && argTypes.empty()){
            main_found = true;
        }

        string funcType = output::makeFunctionType(retType, argTypes);
        table->back()->addEntry(func_name, funcType, 0, argTypes, true, retType);

        auto* new_scope = new Scope(max_offset);
        int i = -1;
        for (int j=0; j<argNames.size(); j++){
            if(isVarDeclared(argNames[j]) || isFuncDeclared(argNames[j])){
                output::errorDef(yylineno, argNames[j]);
                exit(0);
            }
            new_scope->addFuncVar(argNames[j], argTypes[j], i);
            i--;
        }
        table->push_back(new_scope);

        //LLVM part
        FUNC_COUNTER++;
        string llvm_ret_type = retType == "VOID" ? "void" : "i32";
        string args_list = "";
        for(int j = 0; j<argTypes.size(); j++){
            args_list += "i32 %func" + to_string(FUNC_COUNTER) + "arg" + to_string(j + 1);
            if(j < argTypes.size() - 1){
                emit(", ");
            }
        }

        emit("define " + llvm_ret_type + " @" + func_name + "(" + args_list + ") {");
        string stack = "%func" + to_string(FUNC_COUNTER) + "args";
        emit(stack + " = alloca i32, i32 50");
        for(int j = 0; j<argTypes.size(); j++){
            string reg = freshReg();
            emit(reg + "= getelementptr inbounds i32, i32* "   + stack + ", i32 " + to_string(j));
            emit("store i32 %func" + to_string(FUNC_COUNTER) + "arg" + to_string(j + 1) + ", i32* " + reg);
        }

    }
    int addVar(N* type, N* name){
        Scope* scope = table->back();
        if(isVarDeclared(name) || isFuncDeclared(name)){
            output::errorDef(yylineno, ((Node*)name)->val);
            exit(0);
        }
        scope->addVar(((Node*)name)->val, ((Type_var*)type)->type, max_offset);
        return max_offset++;

//        printf("Global offset increased to %d by %s\n", max_offset, ((Node*)name)->val.c_str());
    }
    void newScope(bool is_while = false){
        Scope* scope = new Scope(max_offset, is_while);
        table->push_back(scope);
    }
    bool isVarDeclared(N* n){
        string name = ((Node*)n)->val;
        //for(Scope* scope : *table){
//        for(auto scope = *(table)->rbegin(); scope != *(table)->rend(); ++scope){
        for(int i=table->size() - 1; i>=0; i--){
            Scope* scope = (*table)[i];
            if(scope->isInScope(name)){
                return true;
            }
        }
        return false;
    }
    bool isFuncDeclared(N* n){
        string name = ((Node*)n)->val;
        //for(Scope* scope : *table){
        for(int i=table->size() - 1; i>=0; i--){
            Scope* scope = (*table)[i];
            if(scope->isInScope(name, true)){
                return true;
            }
        }
        return false;
    }
    bool isVarDeclared(const string& name){
        //for(Scope* scope : *table){
//        for(auto scope = *(table)->rbegin(); scope != *(table)->rend(); ++scope){
        for(int i=table->size() - 1; i>=0; i--){
            Scope* scope = (*table)[i];
            if(scope->isInScope(name)){
                return true;
            }
        }
        return false;
    }
    bool isFuncDeclared(const string& name){
        //for(Scope* scope : *table){
        for(int i=table->size() - 1; i>=0; i--){
            Scope* scope = (*table)[i];
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
        for(int i=table->size() - 1; i>=0; i--){
            Scope* scope = (*table)[i];
            for(int j=0; j<scope->entries->size(); j++){
                Entry* e = (*(scope->entries))[j];
                if(e->name == name){
                    //Found it, return type
                    return e->type;
                }
            }
        }
    }
    int getVarOffset(N* n){
        int offset = 0;
        string name = ((Node*)n)->val;
        if(!isVarDeclared(n)){
            output::errorUndef(yylineno, name);
            exit(0);
        }
        //We know variable exits
        for(int i=table->size() - 1; i>=0; i--){
            Scope* scope = (*table)[i];
            for(int j=0; j<scope->entries->size(); j++){
                Entry* e = (*(scope->entries))[j];
                if(e->name == name){
                    //Found it, return offset
                    return offset;
                }
                if(e->is_function){
                    continue;
                }
                offset++;
            }
        }
        return -10;
    }
    string checkValidAssign(N* id, N* exp){
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
        int exp_value = ((Expression*)exp)->number;
        if(var_type == "BYTE" && exp_type == "BYTE" && exp_value > 255){
            output::errorByteTooLarge(yylineno, to_string(((Expression*)exp)->number));
            exit(0);
        }
        return var_type;
    }
    void checkCurrFuncVoid(){
        for(int i=table->size() - 1; i>=0; i--){
            Scope* scope = (*table)[i];
            string ret = scope->getScopeFuncRet();
            if(ret != "None"){
                if(ret == "VOID"){
                    return;
                }
                else{
                    output::errorMismatch(yylineno);
                    exit(0);
                }
            }
        }
    }
    void checkRetType(N* exp){
        string exp_type = ((Expression*)exp)->type;
        string ret;
        for(int i=table->size() - 1; i>=0; i--){
            Scope* scope = (*table)[i];
            ret = scope->getScopeFuncRet();
            if(ret != "None"){
                break;
            }
        }
        if(ret == exp_type){
            return;
        }
        else if(ret == "INT" && exp_type == "BYTE"){
            return;
        }
        output::errorMismatch(yylineno);
        exit(0);
    }
    void checkInWhile(bool is_break = false){
        for(int i=table->size() - 1; i>=0; i--){
            Scope* scope = (*table)[i];
            if(scope->while_scope){
                return;
            }
        }
        if(is_break){
            output::errorUnexpectedBreak(yylineno);
        }
        else{
            output::errorUnexpectedContinue(yylineno);
        }
        exit(0);
    }
    bool checkValidArgs(N* fName, const vector<string>& args){
        string func_name = ((Node*)fName)->val;
        int i = 0;
        for(; i<this->table->size(); i++){
            Scope* s = (*table)[i];
            for(int j=0; j<s->entries->size(); j++){
                Entry* e = (*(s->entries))[j];
                if(e->is_function && e->name == func_name){
                    vector<string> function_args_types = e->function_args;
                    if(function_args_types.size() != args.size()){
                        output::errorPrototypeMismatch(yylineno, func_name, function_args_types);
                        exit(0);
                    }
                    for(int l=0; l<function_args_types.size(); l++){

//                        printf("%s %s\n", function_args_types[l].c_str(), args[args.size()-l-1].c_str());

                        if(!checkValidAssign(function_args_types[l], args[args.size()-l-1])){
                            output::errorPrototypeMismatch(yylineno, func_name, function_args_types);
                            exit(0);
                        }
                    }
                    return true;
                }
            }
        }
    }
    bool checkFuncArgsEmpty(N* fName){
        string func_name = ((Node*)fName)->val;
        int i = 0;
        for(; i<this->table->size(); i++){
            Scope* s = (*table)[i];
            for(int j=0; j<(*(s->entries)).size(); j++){
                Entry* e = (*(s->entries))[j];
                if(e->is_function && e->name == func_name){
                    vector<string> function_args_types = e->function_args;
                    if(!function_args_types.empty()) {
                        output::errorPrototypeMismatch(yylineno, func_name, function_args_types);
                        exit(0);
                    }
                    return true;
                }
            }
        }
    }
    bool checkValidAssign(const string& type1, const string& type2){
        if(type1 == type2){
            return true;
        }
        else if(type1 == "INT" && type2 == "BYTE"){
            return true;
        }
        return false;
    }
    string getFuncRetType(string func_name){
        for(int i = 0; i<this->table->size(); i++){
            Scope* s = (*table)[i];
            for(int j=0; j<(*(s->entries)).size(); j++){
                Entry* e = (*(s->entries))[j];
                if(e->is_function && e->name == func_name){
                    return e->ret_type;
                }
            }
        }
    }
};
#endif //HW3_TABLE_H
