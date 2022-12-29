#include <bits/stdc++.h>
#include "cg_pcode_exec.h"
#include "cg_pcode.h"
#include "cg_return_info.h"
#include "cg_variable.h"
#include "cg_code_type.h"
#include "cg_func.h"
#include "cg_label_generator.h"
#include "cg_object.h"
using namespace std;

PCodeExecutor::PCodeExecutor(vector<PCode*>& codeList) {
    this->codeList = codeList;
    int i = 0;
    for(auto code : this->codeList) {
        CodeType x = code->getType();
        switch (x) {
            case CodeType::MAIN : { // get main function's address
                this->mainAddr = i;
            } break;

            case CodeType::FUNC : { // get all function
                string key = code->getValue1()->to_son_class<string>()->c_str();
                funcTable[key] = new Func(i, *code->getValue2()->to_son_class<int>());
            } break;

            case CodeType::LABEL : { // get all label
                string key = code->getValue1()->to_son_class<string>()->c_str();
                labelTable[key] = i;
            } break;
        }
        i += 1;
    }
    executor();
}
void PCodeExecutor::stk_push(int i) {
    stk.push_back(i);
} 

int PCodeExecutor::stk_pop() {
    int res = stk.back();
    stk.pop_back();
    return res;
}

Var* PCodeExecutor::getVar(string ident) {
    if(varTable.count(ident)) return varTable[ident];
    else return returnInfoList[0]->getVarTable()[ident];
}

int PCodeExecutor::getAddr(Var* var, int intType) {
    // note that dim is the actual number, for example, for array: int a[3][2], a[0][0] is 0, a[0] is 1, a is 2
    // so when search for address, the real [] num is defined as dim - n
    int addr = 0, dim = var->getDimension() - intType;
    if(!dim) addr = var->getIdx();
    else if(dim == 1) {
        int pp = stk_pop();
        if(var->getDimension() == 1) addr = var->getIdx() + pp;
        else addr = var->getIdx() + var->getDim2() * pp;
    }
    else if(dim == 2) {
        int pp_b = stk_pop(), pp_a = stk_pop();
        addr = var->getIdx() + var->getDim2() * pp_a + pp_b;
    }
    return addr;
}

void PCodeExecutor::executor() {
    int callArgsNum = 0, curArsgNum = 0, flagMAIN = 0;
    vector<int> realParams = vector<int>();
    for(; eip < codeList.size(); eip++) {
        auto code = codeList[eip];
        CodeType x = code->getType();
        switch (x) {
            case CodeType::LABEL : {
                /*Nothing to do*/
            } break;

            case CodeType::VAR : {
                Var* var = new Var(stk.size());
                auto string_ptr = code->getValue1()->to_son_class<string>();
                if(string_ptr != nullptr) {
                    string key = string_ptr->c_str();
                    varTable[key] = var;
                }
            } break;

            case CodeType::PUSH : {
                auto int_ptr = code->getValue1()->to_son_class<int>();
                if(int_ptr != nullptr) {
                    stk_push(*int_ptr);
                }
            } break;

            case CodeType::POP : {
                int value = stk_pop(), addr = stk_pop();
                stk[addr] = value;
            } break;

            case CodeType::ADD : {
                int b = stk_pop(), a = stk_pop(), c = a + b;
                stk_push(c);
            } break;
            
            case CodeType::SUB : {
                int b = stk_pop(), a = stk_pop(), c = a - b;
                stk_push(c);
            } break;

            case CodeType::MUL : {
                int b = stk_pop(), a = stk_pop(), c = a * b;
                stk_push(c);
            } break;
        

            case CodeType::DIV : {
                int b = stk_pop(), a = stk_pop(), c = a / b;
                stk_push(c);
            } break;

            case CodeType::MOD : {
                int b = stk_pop(), a = stk_pop(), c = a % b;
                stk_push(c);
            } break;

            case CodeType::CMPEQ : {
                int b = stk_pop(), a = stk_pop(), c = a == b;
                stk_push(c);
            } break;

            case CodeType::CMPNE : {
                int b = stk_pop(), a = stk_pop(), c = a != b;
                stk_push(c);
            } break;

            case CodeType::CMPLT : {
                int b = stk_pop(), a = stk_pop(), c = a < b;
                stk_push(c);
            } break;

            case CodeType::CMPLE : {
                int b = stk_pop(), a = stk_pop(), c = a <= b;
                stk_push(c);
            } break;

            case CodeType::CMPGT : {
                int b = stk_pop(), a = stk_pop(), c = a > b;
                stk_push(c);
            } break;

            case CodeType::CMPGE : {
                int b = stk_pop(), a = stk_pop(), c = a >= b;
                stk_push(c);
            } break;

            case CodeType::AND : {
                int b = stk_pop(), a = stk_pop(), c = a && b;
                stk_push(c);
            } break;

            case CodeType::OR : {
                int b = stk_pop(), a = stk_pop(), c = a || b;
                stk_push(c);
            } break;

            case CodeType::NOT : {
                int a = stk_pop(), b = !a;
                stk_push(b);
            } break;

            case CodeType::POS : {
                stk_push(stk_pop());
            } break;

            case CodeType::NEG : {
                stk_push(-stk_pop());
            } break;

            case CodeType::JMP : {
                auto string_ptr = code->getValue1()->to_son_class<string>();
                if(string_ptr != nullptr) {
                    string key = string_ptr->c_str();
                    eip = labelTable[key];
                }
                else {
                    cout << "JMP Error" << endl;
                    break;
                }
            } break;

            case CodeType::JZ : {
                if(stk.back() == 0) {
                    auto string_ptr = code->getValue1()->to_son_class<string>();
                    if(string_ptr != nullptr) {
                        string key = string_ptr->c_str();
                        eip = labelTable[key];
                    }
                    else {
                        cout << "JZ Error" << endl;
                        break;
                    }
                }
            } break;

            case CodeType::JNZ : {
                if(stk.back() != 0) {
                    auto string_ptr = code->getValue1()->to_son_class<string>();
                    if(string_ptr != nullptr) {
                        string key = string_ptr->c_str();
                        eip = labelTable[key];
                    }
                    else {
                        cout << "JNZ Error" << endl;
                        break;
                    }
                }
            } break;

            case CodeType::FUNC : {
                if(!flagMAIN) eip = mainAddr - 1;
            } break;

            case CodeType::MAIN : {
                flagMAIN = 1;
                returnInfoList.push_back(new ReturnInfo(codeList.size(), stk.size() - 1, 0, 0, 0, varTable));
                varTable.clear();
            } break;

            case CodeType::ENDFUNC : {
                /*Nothing to do*/
            } break;

            case CodeType::PARA : {
                Var* param = new Var(realParams[realParams.size() - callArgsNum + curArsgNum]);
                auto int_ptr = code->getValue2()->to_son_class<int>();
                auto string_ptr = code->getValue1()->to_son_class<string>();
                if(int_ptr != nullptr && string_ptr != nullptr) {
                    int n = *int_ptr;
                    param->setDimension(n);
                    if(n == 2) param->setDim2(stk_pop());
                    string key = string_ptr->c_str();
                    varTable[key] = param;
                    curArsgNum += 1;
                    if(curArsgNum == callArgsNum) realParams.erase(realParams.end() - callArgsNum, realParams.end());
                }
                else {
                    cout << "PARA Error" << endl;
                }
            } break;

            case CodeType::RET : {
                auto int_ptr = code->getValue1()->to_son_class<int>();
                if(int_ptr != nullptr) {
                    int n = *int_ptr;
                    ReturnInfo* info = returnInfoList.back();
                    returnInfoList.pop_back();
                    eip = info->getEip();
                    varTable = info->getVarTable();
                    callArgsNum = info->getCallArgsNum();
                    curArsgNum = info->getCurArgsNum();
                    auto left = stk.begin() + (info->getStackPtr() + 1 - info->getParamNum());
                    if(n == 1) stk.erase(left, stk.end() - 1);
                    else stk.erase(left, stk.end());
                }
                else {
                    cout << "RET Error" << endl;
                    break;
                }
            } break;

            case CodeType::CALL : {
                auto string_ptr = code->getValue1()->to_son_class<string>();
                if(string_ptr != nullptr) {
                    string key = string_ptr->c_str();
                    Func* func = funcTable[key];
                    returnInfoList.push_back(new ReturnInfo(eip, stk.size() - 1, func->getArgs(), func->getArgs(), curArsgNum, varTable));
                    eip = func->getIdx();
                    varTable.clear();
                    callArgsNum = func->getArgs();
                    curArsgNum = 0;
                }
                else {
                    cout << "CALL Error" << endl;
                    break;
                }
            } break;

            case CodeType::RPARA : {
                auto int_ptr = code->getValue1()->to_son_class<int>();
                if(int_ptr != nullptr) {
                    int n = *int_ptr;
                    if(!n) realParams.push_back(stk.size() - 1);
                    else realParams.push_back(stk.back());
                }
                else {
                    cout << "RPARA Error" << endl;
                    break;
                }
            } break;

            case CodeType::GETINT : {
                int a;
                cin >> a;
                stk_push(a);
            } break;

            case CodeType::PRINT : {
                auto string_ptr = code->getValue1()->to_son_class<string>();
                auto int_ptr = code->getValue2()->to_son_class<int>();
                if(string_ptr != nullptr && int_ptr != nullptr) {
                    string fmt_str = string_ptr->c_str();
                    int n = *int_ptr;
                    string tmp = "";
                    vector<int> param = vector<int>();
                    for(int i = 0; i < n; i++) param.push_back(stk_pop());
                    for(int i = 0; i < fmt_str.size(); i++) {
                        if(i + 1 < fmt_str.size()) {
                            if(fmt_str[i] == '%' && fmt_str[i + 1] == 'd') {
                                tmp += to_string(param.back());
                                param.pop_back();
                                i++;
                                continue;
                            }
                        }
                        tmp += fmt_str[i];
                    }
                    tmp = tmp.substr(1, tmp.size() - 2);
                    string res = "";
                    for(int i = 0; i < tmp.size(); ) {
                        if(tmp[i] == '\\') {
                            res += '\n';
                            i++;
                        }
                        else res += tmp[i];
                        i++;
                    }
                    printList.push_back(res);
                }
                else {
                    cout << "PRINT Error" << endl;
                    break;
                }
            } break;

            case CodeType::VALUE : {
                auto string_ptr = code->getValue1()->to_son_class<string>();
                auto int_ptr = code->getValue2()->to_son_class<int>();
                if(string_ptr != nullptr && int_ptr != nullptr) {
                    string ident = string_ptr->c_str();
                    Var* var = getVar(ident);
                    int n = *int_ptr;
                    int addr = getAddr(var, n);
                    stk_push(stk[addr]);
                }
                else {
                    cout << "VALUE Error" << endl;
                    break;
                }
            } break;

            case CodeType::ADDRESS : {
                auto string_ptr = code->getValue1()->to_son_class<string>();
                auto int_ptr = code->getValue2()->to_son_class<int>();
                if(string_ptr != nullptr && int_ptr != nullptr) {
                    string ident = string_ptr->c_str();
                    Var* var = getVar(ident);
                    int n = *int_ptr;
                    int addr = getAddr(var, n);
                    stk_push(addr);
                }
                else {
                    cout << "ADDRESS Error" << endl;
                    break;
                }
            } break;

            case CodeType::DIMVAR : {
                auto string_ptr = code->getValue1()->to_son_class<string>();
                auto int_ptr = code->getValue2()->to_son_class<int>();
                if(string_ptr != nullptr && int_ptr != nullptr) {
                    string ident = string_ptr->c_str();
                    Var* var = getVar(ident);
                    int n = *int_ptr;
                    var->setDimension(n);
                    if(n == 1) {
                        int a = stk_pop();
                        var->setDim1(a);
                    }
                    else if(n == 2) {
                        int b = stk_pop(), a = stk_pop();
                        var->setDim1(a);
                        var->setDim2(b);
                    }
                }
                else {
                    cout << "DIMVAR Error" << endl;
                    break;
                }
            } break;

            case CodeType::PLACEHOLDER : {
                auto string_ptr = code->getValue1()->to_son_class<string>();
                auto int_ptr = code->getValue2()->to_son_class<int>();
                if(string_ptr != nullptr && int_ptr != nullptr) {
                    string ident = string_ptr->c_str();
                    Var* var = getVar(ident);
                    int n = *int_ptr;
                    if(n == 0) stk_push(0);
                    else if(n == 1) {
                        for(int i = 0; i < var->getDim1(); i++) 
                            stk_push(0);
                    }
                    else if(n == 2) {
                        for(int i = 0; i < var->getDim1() * var->getDim2(); i++) 
                            stk_push(0);
                    }
                }
                else {
                    cout << "PLACEHOLDER Error" << endl;
                    break;
                }
            } break;

            case CodeType::EXIT : {
                break;
            } break;
        }
    }
}

void PCodeExecutor::release() {
    for(auto& rt : returnInfoList) {
        if(rt) delete rt;
    }
    vector<ReturnInfo*>().swap(returnInfoList);

    for(auto& vt : varTable) {
        if(vt.second) delete vt.second;
    }
    map<string, Var*>().swap(varTable);

    for(auto& ft : funcTable) {
        if(ft.second) delete ft.second;
    }
    map<string, Func*>().swap(funcTable);
}