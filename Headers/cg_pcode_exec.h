#ifndef CG_PCODE_EXEC_H
#define CG_PCODE_EXEC_H

#include <bits/stdc++.h>
#include "cg_pcode.h"
#include "cg_return_info.h"
#include "cg_variable.h"
#include "cg_code_type.h"
#include "cg_func.h"
#include "cg_label_generator.h"
#include "cg_object.h"
using namespace std;

class PCodeExecutor {
private:
    vector<PCode*> codeList;
    vector<ReturnInfo*> returnInfoList;
    vector<int> stk;
    int eip = 0;  // presents the address of current code
    map<string, Var*> varTable; // memorise the address of the variable in stack
    map<string, Func*> funcTable; // memorises the address of the function in codeList
    map<string, int> labelTable; // memorises the address of the label in codeList
    int mainAddr;
    void release();

public:
    PCodeExecutor(vector<PCode*>& codeList);
    // two operators: push and pop
    void stk_push(int i);
    int stk_pop();
    Var* getVar(string ident);
    int getAddr(Var* var, int intType);

public:
    void executor();
    vector<string> printList;
};

#endif