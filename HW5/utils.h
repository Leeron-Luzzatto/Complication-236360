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

int FUNC_NUMBER = 0;

int DIV_LABEL = 0;


string freshDivLabel(){
    return "div_" +to_string(DIV_LABEL++);
}

string freshFuncPointerReg(){
    return "%funcArgs" + std::to_string(FUNC_COUNTER++);
}

string freshReg(){
    return "%r" + std::to_string(REG_COUNTER++);
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


void init_llvm(){
    emitGlobal("@.div_by_zero = constant [23 x i8] c\"Error division by zero\\00\"");
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
    CodeBuffer::instance().emit("}");
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

void handle_muldiv_exp(const string& resType, const string& binop, const string& resReg, const string& reg1, const string& reg2){
    if(resType == "INT"){
        if(binop == "*"){
            emit(resReg + " = mul i32 " + reg1 + "," + reg2);
        }
        else{
            //DIV
            check_zero_div(reg2);
            emit(resReg + " = sdiv i32 " + reg1 + "," + reg2);
        }
    }
    else{
        //BYTE type
        string tmp1 = freshReg();
        string tmp2 = freshReg();
        if(binop == "*"){
            emit(tmp1 + " = mul i32 " + reg1 + "," + reg2);
        }
        else{
            //DIV
            check_zero_div(reg2);
            emit(tmp2 + " = sdiv i32 " + reg1 + "," + reg2);
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

#endif //HW3_UTILS_H
