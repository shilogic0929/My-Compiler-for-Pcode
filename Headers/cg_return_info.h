#ifndef CG_RETURN_INFO
#define CG_RETURN_INFO

#include <bits/stdc++.h>
#include "cg_variable.h"
using namespace std;

class ReturnInfo {
private:
    int eip, stkPtr, paramNum, callArgsNum, curArgsNum;
    map<string, Var*> varTable;

public:
    ReturnInfo(int eip, int stkPtr, int paramNum, int callArgsNum, int curArgsNum, map<string, Var*>& varTable) {
        this->eip = eip;
        this->stkPtr = stkPtr;
        this->paramNum = paramNum;
        this->callArgsNum = callArgsNum;
        this->curArgsNum = curArgsNum;
        this->varTable = varTable;
    }
    
public:
    int getEip() { return this->eip; }

    map<string, Var*> getVarTable() { return this->varTable; }

    int getStackPtr() { return this->stkPtr; }

    int getParamNum() { return this->paramNum; }

    int getCallArgsNum() { return this->callArgsNum; }

    int getCurArgsNum() { return this->curArgsNum; }
};

#endif