//
// Created by adirr on 12/01/2021.
//

#ifndef HW3_UTILS_H
#define HW3_UTILS_H

#include "bp.hpp"
#include "parser.hpp"

#include <string>
using namespace std;

int REG_COUNTER = 0;
    int FUNC_COUNTER = 0;


int DIV_LABEL = 0;


string freshDivLabel(){
    return "div_" +to_string(DIV_LABEL++);
}

string freshReg(){
    return "%r" + to_string(REG_COUNTER++);
}

string freshStr(){
    return "@.str" + to_string(REG_COUNTER++);
}

int emit(const string& dataLine){
    return CodeBuffer::instance().emit(dataLine);
}

void emitGlobal(const string& dataLine){
    CodeBuffer::instance().emitGlobal(dataLine);
}

void printGlobalBuffer(){
    CodeBuffer::instance().printGlobalBuffer();
}

void printCodeBuffer(){
    CodeBuffer::instance().printCodeBuffer();
}

string genLabel(){
    return CodeBuffer::instance().genLabel();
}

vector<pair<int,BranchLabelIndex>> makeList(pair<int,BranchLabelIndex> item){
    return CodeBuffer::instance().makelist(item);
}

void backPatch(const vector<pair<int,BranchLabelIndex>>& address_list, const std::string &label){
    CodeBuffer::instance().bpatch(address_list, label);
}

vector<pair<int,BranchLabelIndex>> merge(const vector<pair<int,BranchLabelIndex>> &l1,const vector<pair<int,BranchLabelIndex>> &l2){
    return CodeBuffer::instance().merge(l1, l2);
}


void init_llvm(){
    emit("@.div_by_zero = constant [23 x i8] c\"Error division by zero\\00\"");
    emitGlobal("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
    emitGlobal("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");

    emit("declare i32 @printf(i8*, ...)");
    emit("declare void @exit(i32)");

    emit("define void @printi(i32) {");
    emit("call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0), i32 %0)");
    emit("ret void");
    emit("}");

    emit("define void @print(i8*) {");
    emit("call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0), i8* %0)");
    emit("ret void");
    emit("}");

}

void func_end(const string& retType){
    (retType == "VOID") ? emit("ret void") : emit("ret i32 0");
    emit("}");
}

void check_zero_div(const string& reg){
    string brunchReg = freshReg();
    emit(brunchReg + "= icmp eq i32 " + reg + ", 0");
    string trueLabel = freshDivLabel();
    string falseLabel = freshDivLabel();
    emit("br i1 " + brunchReg + ", label %" + trueLabel + ", label %" + falseLabel);
    emit(trueLabel + ":");
    string zeroDivReg = freshReg();
    emit(zeroDivReg + " = getelementptr [23 x i8], [23 x i8]* @.div_by_zero, i32 0, i32 0");
    emit("call void @print(i8* " + zeroDivReg + ")");
    emit("call void @exit(i32 1)");
    emit("br label %" + falseLabel);
    emit(falseLabel + ":");
}

void operand_handler(const string& resType, const string& binop, const string& resReg, const string& reg1, const string& reg2){
    if(resType == "INT"){
        if(binop == "*"){
            emit(resReg + " = mul i32 " + reg1 + "," + reg2);
        }
        else if(binop == "/"){
            //DIV
            check_zero_div(reg2);
            emit(resReg + " = sdiv i32 " + reg1 + "," + reg2);
        }
        else if(binop == "-"){
            emit(resReg + " = sub i32 " + reg1 + "," + reg2);
        }
        else if(binop == "+"){
            emit(resReg + " = add i32 " + reg1 + "," + reg2);
        }
    }
    else{
        //BYTE type
        string tmp1 = freshReg();
        string tmp2 = freshReg();
        if(binop == "*"){
            emit(tmp1 + " = mul i32 " + reg1 + "," + reg2);
        }
        else if(binop == "/"){
            //DIV
            check_zero_div(reg2);
            emit(tmp1 + " = sdiv i32 " + reg1 + "," + reg2);
        }
        else if(binop == "-"){
            emit(tmp1 + " = sub i32 " + reg1 + "," + reg2);
        }
        else if(binop == "+"){
            emit(tmp1 + " = add i32 " + reg1 + "," + reg2);
        }
        emit(tmp2 + " = trunc i32 " + tmp1 + " to i8");
        emit(resReg + "= zext i8 " + tmp2 + " to i32");
    }
}

void llvm_bool_handle(N* n, const string& reg){
    Expression* exp = ((Expression*)exp);
    if(exp->type=="BOOL") {
        string brunch = freshReg();
        emit(brunch + " = icmp eq i32 0, " + reg);
        int address = emit("br i1 " + brunch + ", label @, label @");
        exp->falselist = makeList({address,SECOND});
        exp->truelist = makeList({address,FIRST});
    }
    else{
        exp->regName=reg;
    }
}

void string_handler(N* n){
    Expression* exp = ((Expression*)exp);
    string strReg1 = freshStr();
    string numB = to_string(exp->str.length()+1);
    emitGlobal(strReg1 + " = constant [" + numB + " x i8] c\"" + exp->str + "\\00\"");
    string strReg2 = freshReg();
    emit(strReg2 + " = getelementptr [" + numB + " x i8] , ["+ numB + " x i8]* " + strReg1 + ", i32 0, i32 0");
    exp->regName = strReg2;
}

void relop_handler(N* n, N* res, N* exp1, N* exp2){
    string operand = ((Node*)n)->val;
    string llvm_op;
    if (operand == "==")
        llvm_op = "eq";
    else if (operand == "<")
        llvm_op = "slt";
    else if (operand == ">")
        llvm_op = "sgt";
    else if (operand == "<=")
        llvm_op = "sle";
    else if (operand == ">=")
        llvm_op = "sge";
    else if (operand == "!+")
        llvm_op = "ne";
    string condReg = freshReg();
    emit(condReg + " = icmp " + llvm_op + " i32 " + exp1->regName + ", " + exp2->regName);
    int address = emit("br i1 " + condReg + ", label @, label @");
    res->truelist = makeList({address, FIRST});
    res->falselist = makeList({address, SECOND});
}

void exp_handler(N* n, int offset){
    Expression* exp = ((Expression*)n);
    string stackP = freshReg();
    if(exp->type != "BOOL"){
        emit(stackP + "= getelementptr inbounds i32, i32* %func" + to_string(FUNC_COUNTER) + "args, i32 " + to_string(offset));
        emit("store i32 " + exp->regName + ", i32* " + stackP);
    }
    else{
        string phiResult = freshReg();
        string regResult = freshReg();
        string fLabel = genLabel();
        int falseAddress = emit("br label @");
        string tLabel = genLabel();
        int trueAddress = emit("br label @");
        backPatch(exp->truelist, tLabel);
        backPatch(exp->falselist, fLabel);
        string phiLabel = genLabel();
        backPatch(makeList({trueAddress,FIRST}), phiLabel);
        backPatch(makeList({falseAddress,FIRST}), phiLabel);

        string toLoad = freshReg();
        emit(phiResult + " = phi i1 [0, %"+fLabel+"], [1, %"+tLabel+"]");
        emit(toLoad + "= zext i1 "+ phiResult+" to i32");
        if(!exp->label.empty()){
            emit("br label %" + exp->label);
        }
        emit(stackP + "= getelementptr inbounds i32, i32* %func" + to_string(FUNC_COUNTER) + "args, i32 " + to_string(offset));
        emit("store i32 " + toLoad + ", i32* " + stackP);
    }
}

void id_handler(N* n, int offset){
    Expression* exp = ((Expression*)n);
    string stackP = freshReg();
    emit(stackP + "= getelementptr inbounds i32, i32* %func" + to_string(FUNC_COUNTER) + "args, i32 " + to_string(offset));
    string tmp = freshReg();
    emit(tmp + "= load i32, i32* " + stackP);
    if(exp->type != "BOOL"){
        exp->regName = tmp;
    }
    else{
        string cond = freshReg();
        emit(cond + " = icmp eq i32 0, " + tmp);
        int address = emit("br i1 " + cond + ", label @, label @");
        exp->falselist = makeList({address, FIRST});
        exp->truelist = makeList({address, SECOND});
    }
}

#endif //HW3_UTILS_H
