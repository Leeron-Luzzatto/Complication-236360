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
int IN_LABEL = 0;


string freshINLabel(){
    return "div_" +to_string(IN_LABEL++);
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
    emitGlobal("@.div_by_zero = constant [23 x i8] c\"Error division by zero\\00\"");
    emitGlobal("@.set_range_plus = constant [30 x i8] c\"Error out of set range. Op: +\\00\"");
    emitGlobal("@.set_range_minus = constant [30 x i8] c\"Error out of set range. Op: -\\00\"");
    emitGlobal("@.set_range_in = constant [31 x i8] c\"Error out of set range. Op: in\\00\"");
    emitGlobal("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
    emitGlobal("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");

    emit("declare i32 @printf(i8*, ...)");
    emit("declare void @exit(i32)");
    emit("declare i8* @malloc(i32)");
    emit("declare void @free(i8*)");
    emit("declare void @llvm.memcpy.p0i8.p0i8.i32(i8*, i8*, i32, i1)");

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
    string branchReg = freshReg();
    emit(branchReg + "= icmp eq i32 " + reg + ", 0");
    string trueLabel = freshINLabel();
    string falseLabel = freshINLabel();
    emit("br i1 " + branchReg + ", label %" + trueLabel + ", label %" + falseLabel);
    emit(trueLabel + ":");
    emit("call void @print(i8* getelementptr ([23 x i8], [23 x i8]* @.div_by_zero, i32 0, i32 0))");
    emit("call void @exit(i32 1)");
    emit("br label %" + falseLabel);
    emit(falseLabel + ":");
}

void operand_handler_no_set(const string& resType, const string& binop, const string& resReg, const string& reg1, const string& reg2){
    if(resType == "INT"){
        if(binop == "*"){
            emit(resReg + " = mul i32 " + reg1 + ", " + reg2);
        }
        else if(binop == "/"){
            //DIV
            check_zero_div(reg2);
            emit(resReg + " = sdiv i32 " + reg1 + ", " + reg2);
        }
    }
    else{
        //BYTE type
        string tmp1 = freshReg();
        string tmp2 = freshReg();
        if(binop == "*"){
            emit(tmp1 + " = mul i32 " + reg1 + ", " + reg2);
        }
        else if(binop == "/"){
            //DIV
            check_zero_div(reg2);
            emit(tmp1 + " = sdiv i32 " + reg1 + ", " + reg2);
        }
        emit(tmp2 + " = trunc i32 " + tmp1 + " to i8");
        emit(resReg + "= zext i8 " + tmp2 + " to i32");
    }
}

void checkRange(const string& setPointer, const string& num, const string& op){
    //Set size
    string setSizeP = freshReg();
    emit(setSizeP + " = getelementptr inbounds i32, i32* " + setPointer + ", i32 0");
    string setSizeB = freshReg();
    emit(setSizeB + " = load i32, i32* " + setSizeP);
    string setSize = freshReg();
    emit(setSize + " = sdiv i32 " + setSizeB + ", 4");
    //First element
    string fromP = freshReg();
    emit(fromP + " = getelementptr inbounds i32, i32* " + setPointer + ", i32 1");
    string from = freshReg();
    emit(from + " = load i32, i32* " + fromP + " ; SET from value");
    //Last element @@@@Remember we added 4!
    string temp = freshReg();
    emit(temp + " = add i32 " + from + ", " + setSize);
    string To = freshReg();
    emit(To + " = sub i32 " + temp + ", 4");
    //Check exp value is bigger then from
    string isBiggerFrom = freshReg();
    emit(isBiggerFrom + " = icmp sgt i32 " + num + ", " + To);
    string errorLabel = freshINLabel();
    string passOne = freshINLabel();
    emit("br i1 " + isBiggerFrom + ", label %" + errorLabel + ", label %" + passOne);
    emit(passOne + ":");
    string isSmallerTo = freshReg();
    string PassAll = freshINLabel();
    emit(isSmallerTo + " = icmp slt i32 " + num + ", " + from);
    emit("br i1 " + isSmallerTo + ", label %" + errorLabel + ", label %" + PassAll);
    emit(errorLabel + ":");
    if(op == "in"){
        emit("call void @print(i8* getelementptr ([31 x i8], [31 x i8]* @.set_range_in, i32 0, i32 0))");
    }
    else if(op == "+"){
        emit("call void @print(i8* getelementptr ([30 x i8], [30 x i8]* @.set_range_plus, i32 0, i32 0))");
    }
    else{
        emit("call void @print(i8* getelementptr ([30 x i8], [30 x i8]* @.set_range_minus, i32 0, i32 0))");
    }

    //DEBUG
//    string err = freshStr();
//    string str_len = "9";
//    emitGlobal(err + " = constant [" + str_len + " x i8] c\"GOT HERE\\00\"");
//    string to_print = freshReg();
//    emit(to_print + " = getelementptr [" + str_len + " x i8], [" + str_len + " x i8]* " + err + ", i32 0, i32 0");
//    emit("call void @print(i8* " + to_print + ")");

    emit("call void @exit(i32 1)");
    emit("ret void");
    emit(PassAll + ":");
}

void operand_handler_with_set(const string& resType, const string& binop, const string& resReg, N* e1, N* e2, N* resExp){
    if(resType == "INT"){
        if(binop == "-"){
            emit(resReg + " = sub i32 " + e1->regName + "," + e2->regName);
        }
        else if(binop == "+"){
            emit(resReg + " = add i32 " + e1->regName + "," + e2->regName + " ; + operand");
        }
    }
    else if(resType == "BYTE"){
        //BYTE type
        string tmp1 = freshReg();
        string tmp2 = freshReg();
        if(binop == "-"){
            emit(tmp1 + " = sub i32 " + e1->regName + "," + e2->regName);
        }
        else if(binop == "+"){
            emit(tmp1 + " = add i32 " + e1->regName + "," + e1->regName);
        }
        emit(tmp2 + " = trunc i32 " + tmp1 + " to i8");
        emit(resReg + "= zext i8 " + tmp2 + " to i32");
    }
    else if(resType == "SET"){
        Expression* set = nullptr;
        Expression* num = nullptr;
        if(((Expression*)e1)->type=="SET"){
            set = (Expression*)e1;
            num = (Expression*)e2;
        }
        else{
            set = (Expression*)e2;
            num = (Expression*)e1;
        }

        if(binop == "+"){
            string setP = freshReg();
            emit(setP + " = inttoptr i32 " + set->regName + " to i32*");
            checkRange(setP, num->regName, binop);
            //Set size
            string setSizeP = freshReg();
            emit(setSizeP + " = getelementptr inbounds i32, i32* " + setP + ", i32 0");
            string setSize = freshReg();
            emit(setSize + " = load i32, i32* " + setSizeP);
            //Get first index
            string fromP = freshReg();
            emit(fromP + " = getelementptr inbounds i32, i32* " + setP + ", i32 1");
            string from = freshReg();
            emit(from + " = load i32, i32* " + fromP);
            //Allocate new SET
            string seti8 = freshReg();
            emit(seti8 + " = call i8* @malloc(i32 " + setSize + ")");
            string seti32 = freshReg();
            emit(seti32 + " = bitcast i8* " + seti8 + " to i32*");
            string setToSave = freshReg();
            emit(setToSave + " = ptrtoint i32* " + seti32 + " to i32");
            //Copy old to new
            string srcSeti8 = freshReg();
            emit(srcSeti8 + " = bitcast i32* " + setP + " to i8*");
            emit("call void @llvm.memcpy.p0i8.p0i8.i32(i8* " + seti8 + ", i8* " + srcSeti8 + ", i32 " + setSize + ", i1 1)");
            //Find SET array index to light up
            string temp = freshReg();
            emit(temp + " = sub i32 " + num->regName + ", " + from);
            string location = freshReg();
            emit(location + " = add i32 3, " + temp);
            string locationP = freshReg();
            emit(locationP + " = getelementptr inbounds i32, i32* " + seti32 + ", i32 " + location);
            string curr = freshReg();
            emit(curr + " = load i32, i32* " + locationP);

            string branchReg = freshReg();
            emit(branchReg + "= icmp eq i32 " + curr + ", 0");
            string trueLabel = freshINLabel();
            string falseLabel = freshINLabel();
            emit("br i1 " + branchReg + ", label %" + trueLabel + ", label %" + falseLabel);
            emit(trueLabel + ":");

            emit("store i32 1, i32* " + locationP);
            string numElementsP = freshReg();
            emit(numElementsP + " = getelementptr inbounds i32, i32* " + seti32 + ", i32 2");
            string numElements = freshReg();
            emit(numElements + " = load i32, i32* " + numElementsP);
            string newSize = freshReg();
            emit(newSize + " = add i32 1, " + numElements);
            emit("store i32 " + newSize + ", i32* " + numElementsP);
            emit("br label %" + falseLabel);
            emit(falseLabel + ":");

            resExp->regName = setToSave;

        }
        else{
            string setP = freshReg();
            emit(setP + " = inttoptr i32 " + set->regName + " to i32*");
            checkRange(setP, num->regName, binop);
            //Set size
            string setSizeP = freshReg();
            emit(setSizeP + " = getelementptr inbounds i32, i32* " + setP + ", i32 0");
            string setSize = freshReg();
            emit(setSize + " = load i32, i32* " + setSizeP);
            //Get first index
            string fromP = freshReg();
            emit(fromP + " = getelementptr inbounds i32, i32* " + setP + ", i32 1");
            string from = freshReg();
            emit(from + " = load i32, i32* " + fromP);
            //Allocate new SET
            string seti8 = freshReg();
            emit(seti8 + " = call i8* @malloc(i32 " + setSize + ")");
            string seti32 = freshReg();
            emit(seti32 + " = bitcast i8* " + seti8 + " to i32*");
            string setToSave = freshReg();
            emit(setToSave + " = ptrtoint i32* " + seti32 + " to i32");
            //Copy old to new
            string srcSeti8 = freshReg();
            emit(srcSeti8 + " = bitcast i32* " + setP + " to i8*");
            emit("call void @llvm.memcpy.p0i8.p0i8.i32(i8* " + seti8 + ", i8* " + srcSeti8 + ", i32 " + setSize + ", i1 1)");
            //Find SET array index to light up
            string temp = freshReg();
            emit(temp + " = sub i32 " + num->regName + ", " + from);
            string location = freshReg();
            emit(location + " = add i32 3, " + temp);
            string locationP = freshReg();
            emit(locationP + " = getelementptr inbounds i32, i32* " + seti32 + ", i32 " + location);
            string curr = freshReg();
            emit(curr + " = load i32, i32* " + locationP);

            string branchReg = freshReg();
            emit(branchReg + "= icmp eq i32 " + curr + ", 1");
            string trueLabel = freshINLabel();
            string falseLabel = freshINLabel();
            emit("br i1 " + branchReg + ", label %" + trueLabel + ", label %" + falseLabel);
            emit(trueLabel + ":");

            emit("store i32 0, i32* " + locationP);
            string numElementsP = freshReg();
            emit(numElementsP + " = getelementptr inbounds i32, i32* " + seti32 + ", i32 2");
            string numElements = freshReg();
            emit(numElements + " = load i32, i32* " + numElementsP);
            string newSize = freshReg();
            emit(newSize + " = sub i32 "+ numElements + ", 1");
            emit("store i32 " + newSize + ", i32* " + numElementsP);
            emit("br label %" + falseLabel);
            emit(falseLabel + ":");

            resExp->regName = setToSave;
        }
    }

}

void string_handler(N* n){
    Expression* exp = ((Expression*)exp);
    string strReg1 = freshStr();
    string numB = to_string(exp->name.length()+1 - 2);
    emitGlobal(strReg1 + " = constant [" + numB + " x i8] c\"" + exp->name.substr(1, exp->name.length()-2) + "\\00\"");
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
    else if (operand == "!=")
        llvm_op = "ne";
    string condReg = freshReg();
    emit(condReg + " = icmp " + llvm_op + " i32 " + exp1->regName + ", " + exp2->regName);
    int address = emit("br i1 " + condReg + ", label @, label @");
    res->truelist = makeList({address, FIRST});
    res->falselist = makeList({address, SECOND});
}

string bool_handler(Expression* exp){
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
    return toLoad;
}

void exp_handler(N* e, N* t, int offset, N* statement){
    Expression* exp = ((Expression*)e);
    Type_var* type = (Type_var*)t;
    string stackP = freshReg();
    if(exp->type == "BOOL"){
        string  toLoad = bool_handler(exp);
        emit(stackP + "= getelementptr inbounds i32, i32* %func" + to_string(FUNC_COUNTER) + "args, i32 " + to_string(offset));
        emit("store i32 " + toLoad + ", i32* " + stackP);
    }
    if(exp->type == "SET"){
        string setP = freshReg();
        emit(setP + " = inttoptr i32 " + exp->regName + " to i32* ; Cast set from i32 to i32* again");

        string sizeP = freshReg();
        emit(sizeP + " = getelementptr inbounds i32, i32* " + setP + ", i32 0");
        string size = freshReg();
        emit(size + " = load i32, i32* " + sizeP);
        string mallocSize = freshReg();
        emit(mallocSize + " = add i32 0, " + size + "; Get total SET size in bytes");

        string seti8 = freshReg();
        emit(seti8 + " = call i8* @malloc(i32 " + mallocSize + ")");

        string srci32 = freshReg();
        emit(srci32 + " = inttoptr i32 " + exp->regName +" to i32*");
        string srci8 = freshReg();
        emit(srci8 + " = bitcast i32* " + srci32 + " to i8*");
        emit("call void @llvm.memcpy.p0i8.p0i8.i32(i8*" + seti8 + ", i8* " + srci8 + ", i32 " + mallocSize + ", i1 1)");

        string seti32 = freshReg();
        emit(seti32 + " = bitcast i8* " + seti8 + " to i32*");
        stackP = freshReg();
        emit(stackP + " = getelementptr inbounds i32, i32* %func" + to_string(FUNC_COUNTER) + "args, i32 "+ to_string(offset));
        string setToSave = freshReg();
        emit(setToSave + " = ptrtoint i32* " + seti32 + " to i32");
        emit("store i32 " + setToSave + ", i32* " + stackP  + " ; store new SET on stack");

        statement->regName = setToSave;
    }
    else{
        emit(stackP + "= getelementptr inbounds i32, i32* %func" + to_string(FUNC_COUNTER) + "args, i32 " + to_string(offset));
        emit("store i32 " + exp->regName + ", i32* " + stackP);
    }
}

void id_handler(N* n, int offset){
    Expression* exp = ((Expression*)n);
    string stackP = freshReg();
    emit(stackP + "= getelementptr inbounds i32, i32* %func" + to_string(FUNC_COUNTER) + "args, i32 " + to_string(offset));
    string tmp = freshReg();
    emit(tmp + "= load i32, i32* " + stackP);
    if(exp->type == "BOOL"){
        string cond = freshReg();
        emit(cond + " = icmp eq i32 0, " + tmp);
        int address = emit("br i1 " + cond + ", label @, label @");
        exp->falselist = makeList({address, FIRST});
        exp->truelist = makeList({address, SECOND});
    }
    else{
        exp->regName = tmp;
    }


}

string func_call(N* got_id, N* got_expList, const string& retType, N* call){
//    int address = emit("br label @");
//    got_expList->nextlist = makeList({address, FIRST});
    Node* ID = ((Node*)got_id);
    Exp_list* expList = ((Exp_list*)got_expList);
    string llvmRetType;
    string retReg, callRes;
    if(retType == "VOID"){
        llvmRetType = "void";
        retReg = "";
        callRes = "";
    }
    else{
        llvmRetType = "i32";
        retReg = freshReg();
        callRes = retReg + " = ";
    }
    string param_list = "";
    bool is_print = ID->val == "print";
    for(int i=0; i<expList->types.size(); i++){
        string currentType;
        string currentReg;
        if(is_print){
            currentType = "i8*";
            currentReg = expList->regNames[expList->types.size() -i -1];
            }

        else{
            if(expList->types[expList->types.size() -i -1] == "BOOL"){
                currentReg = bool_handler(expList->expressions[expList->types.size() -i -1]);
                currentType = "i32";

            }
            else{
                currentType = "i32";
                currentReg = expList->regNames[expList->types.size() -i -1];
            }
        }
        if(i<expList->types.size() - 1){
            param_list += ", ";
        }
        param_list += currentType + " " + currentReg;
    }
//    address = emit("br label @");
//    string label = genLabel();
    emit(callRes + "call " + llvmRetType + " @" + ID->val + "(" + param_list + ")");
//    backPatch(merge(makeList({address, FIRST}), got_expList->nextlist), label);
    if(retType == "BOOL"){
        string branch = freshReg();
        emit(branch + " = icmp eq i32 0, " + retReg);
        int address = emit("br i1 " + branch +", label @, label @");
        call->falselist = makeList({address,FIRST});
        call->truelist = makeList({address,SECOND});
        return "";
    }
    return retReg;
}

string func_call_noParam(N* got_id, const string& retType, N* call){
    Node* ID = ((Node*)got_id);
    string llvmRetType;
    string retReg, callRes;
    if(retType == "VOID"){
        llvmRetType = "void";
        retReg = "";
        callRes = "";
    }
    else if(retType == "SET"){ //SET_
        llvmRetType = "i32*";
        retReg = freshReg();
        callRes = retReg + " = ";
    }
    else{
        llvmRetType = "i32";
        retReg = freshReg();
        callRes = retReg + " = ";
    }

//    int address = emit("br label @");
//    string label = genLabel();
    emit(callRes + "call " + llvmRetType + " @" + ID->val + "()");
//    backPatch(makeList({address, FIRST}), label);
    if(retType == "BOOL"){
        string branch = freshReg();
        emit(branch + " = icmp eq i32 0, " + retReg);
        int address = emit("br i1 " + branch +", label @, label @");
        call->falselist = makeList({address,FIRST});
        call->truelist = makeList({address,SECOND});
        return "";
    }
    return retReg;
}

void new_var_handler(N* t, int offset, N* statement){
    Type_var* type = ((Type_var*)t);
    string stackP = freshReg();
    //Handle SET_ init to empty group
    if(type->type != "SET") {
        emit(stackP + " = getelementptr inbounds i32, i32* %func" + to_string(FUNC_COUNTER) + "args, i32 " +
             to_string(offset));
        emit("store i32 0, i32* " + stackP);
    }
    else{
        string malloci8 = freshReg();
        string sizeToAlloc = to_string(4 * (type->end - type->start + 4)); //Size of int(4B) *(SET SIZE + 4)
        emit( malloci8 + " = call i8* @malloc(i32 " + sizeToAlloc + ")");
        string malloci32 = freshReg();
        emit(malloci32 + " = bitcast i8* " + malloci8 + " to i32*");

        for(int i = 2; i < type->end - type->start + 4; i++){
            string temp = freshReg();
            emit(temp + " = getelementptr inbounds i32, i32* " + malloci32 + ", i32 " + to_string(i));
            emit("store i32 0, i32* " + temp);
        }
//        emit("store i32 zeroinitializer, i32* " + malloci32);
//        //TETS
//        emit("%test1 = getelementptr inbounds i32, i32* " + malloci32 + ", i32 0");
//        emit("%t1 = load i32, i32* %test1");
//        emit("%test2 = getelementptr inbounds i32, i32* " + malloci32 + ", i32 50");
//        emit("%t2 = load i32, i32* %test2");
//        emit("%test3 = getelementptr inbounds i32, i32* " + malloci32 + ", i32 100");
//        emit("%t3 = load i32, i32* %test3");
//        emit("call void @printi(i32 %t1)");
//        emit("call void @printi(i32 %t2)");
//        emit("call void @printi(i32 %t3)");
//        emit("call void @exit(i32 1)");
//        string start = freshReg();
//        string end = freshReg();
//        emit(start + " = getelementptr inbounds i32, i32* " + malloci32 + ", i32 0");
//        emit(end + " = getelementptr inbounds i32, i32* " + malloci32 + ", i32 1");
//        emit("store i32 " + to_string(type->start) + ", i32* " + start);
//        emit("store i32 " + to_string(type->end) + ", i32* " + end);


        emit(stackP + " = getelementptr inbounds i32, i32* %func" + to_string(FUNC_COUNTER) + "args, i32 " +
             to_string(offset));

        string toSave = freshReg();
        emit(toSave + " = ptrtoint  i32* " + malloci32 + " to i32");
        emit("store i32 " + toSave + ", i32* " + stackP);

        string sizeP = freshReg();
        string startP = freshReg();
        string size = freshReg();
        string start = freshReg();
        emit(sizeP + " = getelementptr inbounds i32, i32* " + malloci32 + ", i32 0");
        emit(size + " = add i32 0, " + sizeToAlloc);
        emit("store i32 " + size + ", i32* " + sizeP + " ; Store SET size in block #0");

        emit(startP + " = getelementptr inbounds i32, i32* " + malloci32 + ", i32 1");
        emit(start + " = add i32 0, " + to_string(type->start));
        emit("store i32 " + start + ", i32* " + startP + " ; store SET start index in block #1");

        statement->regName = toSave;
    }
}

void assign_to_exist(N* e, int offset, N* statement, const string& type){
    Expression* exp = ((Expression*)e);
    string stackP = freshReg();
    emit(stackP + "= getelementptr inbounds i32, i32* %func" + to_string(FUNC_COUNTER) + "args, i32 " + to_string(offset));
    if(exp->type == "BOOL"){
        string  toLoad = bool_handler(exp);
        emit("store i32 " + toLoad + ", i32* " + stackP);
    }

    if(exp->type == "SET"){
        //free old SET
        string oldSeti32 = freshReg();
        emit(oldSeti32 + " = load i32, i32* " + stackP);
        string oldSetP32 = freshReg();
        emit(oldSetP32 + " = inttoptr i32 " + oldSeti32 + " to i32*");
        string oldSetP = freshReg();
        emit(oldSetP + " = bitcast i32* " + oldSetP32 + " to i8*");
        emit("call void @free(i8* " + oldSetP + ") ; free old set before allocate new one");

        //Handle src SET
        string srcSeti32 = freshReg();
        emit(srcSeti32 + " = inttoptr i32 " + exp->regName + " to i32* ; Cast set from i32 to i32* again before copy to old set");
        string srcSeti8 = freshReg();
        emit(srcSeti8 + " = bitcast i32* " + srcSeti32 + " to i8*");

        string sizeP = freshReg();
        emit(sizeP + " = getelementptr inbounds i32, i32* " + srcSeti32 + ", i32 0");
        string size = freshReg();
        emit(size + " = load i32, i32* " + sizeP);
        string mallocSize = freshReg();
        emit(mallocSize + " = add i32 0, " + size + "; Get total SET size in bytes");

        //Handle dest SET(malloc and store)
        string destSeti8 = freshReg();
        emit(destSeti8 + " = call i8* @malloc(i32 " + mallocSize + ")");
        string destSeti32 = freshReg();
        emit(destSeti32 + " = bitcast i8* " + destSeti8 + " to i32*");
        string setToSave = freshReg();
        emit(setToSave + " = ptrtoint i32* " + destSeti32 + " to i32");
        emit("store i32 " + setToSave + ", i32* " + stackP);
        emit("call void @llvm.memcpy.p0i8.p0i8.i32(i8* " + destSeti8 + ", i8* " + srcSeti8 + ", i32 " + mallocSize + ", i1 1)");

        statement->regName = setToSave;
    }
    else{
        emit("store i32 " + exp->regName + ", i32* " + stackP);
    }
}

void return_exp_handler(N* e){
    Expression* exp = (Expression*)e;
    string res;
    if(exp->type == "BOOL"){
        res = bool_handler(exp);
    }
    else{
        res = exp->regName;
    }
    emit("ret i32 " + res);
}

void set_in_handler(N* n, N* s, N* res){
    Expression* num = (Expression*)n;
    Expression* set = (Expression*)s;
    string seti32 = freshReg();
    emit(seti32 + " = inttoptr i32 " + set->regName + " to i32*");
    checkRange(seti32, num->regName, "in");
    
    string fromP = freshReg();
    emit(fromP + " = getelementptr inbounds i32, i32* " + seti32 + ",i32 1");
    string from = freshReg();
    emit(from + " = load i32, i32* " + fromP);

    string temp = freshReg();
    emit(temp + " = sub i32 " + num->regName + "," + from);
    string location = freshReg();
    emit(location + " = add i32 3, " + temp);
    string numP = freshReg();
    emit(numP + " = getelementptr inbounds i32, i32* " + seti32 + ",i32 " + location);
    string SetNumCell = freshReg();
    emit(SetNumCell + " = load i32,i32* " + numP);

    string cmp = freshReg();
    emit(cmp + " = icmp eq i32 1, " + SetNumCell);
    int address = emit("br i1 " + cmp + ", label @, label @");
    res->truelist = makeList({address, FIRST});
    res->falselist = makeList({address, SECOND});
}

void getSetSize(N* s, N* res){
    Expression* set = (Expression*)s;
    string setP = freshReg();
    emit(setP + " = inttoptr i32 " + set->regName + " to i32*");
    string sizeP = freshReg();
    emit(sizeP + " = getelementptr inbounds i32, i32* " + setP + ", i32 2");
    string size = freshReg();
    emit(size + " = load i32, i32* " + sizeP);
    res->regName = size;
}

#endif //HW3_UTILS_H
