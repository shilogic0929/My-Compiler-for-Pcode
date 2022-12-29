#ifndef CG_PCODE_H
#define CG_PCODE_H

#include <bits/stdc++.h>
#include "cg_code_type.h"
#include "cg_object.h"
using namespace std;

class PCode {
private:
    // contains a CodeType and two operating values
    // CodeType is an enum, value1 and value2 may be integer or string or null, which depend on specific code type
    // Caculation Type contains binocular and singular operators:
    // Binocular operator: int b = stk_pop(), a = stk_pop(), c = cal(a, b), stk_push(c);
    // Singular operator: int a = stk.pop(), b = cal(a), stk.push(b);
    CodeType type;
    Object* value1 = nullptr;
    Object* value2 = nullptr;

public:
    PCode(CodeType type) {
        this->type = type;
    }
    PCode(CodeType type, Object* value1) {
        this->type = type;
        this->value1 = value1;
    }
    PCode(CodeType type, Object* value1, Object* value2) {
        this->type = type;
        this->value1 = value1;
        this->value2 = value2;
    }
    void setValue1(Object* value1) {
        this->value1 = value1;
    }
    void setValue2(Object* value2) { 
        this->value2 = value2; 
    }
    CodeType getType() { 
        return this->type; 
    }
    Object* getValue1() {
        return this->value1;
    }
    Object* getValue2() {
        return this->value2;
    }

public:
    string outString() {
        string res = "";
        if(this->type == CodeType::LABEL) {
            auto tmp = value1->to_son_class<string>();
            if(tmp != nullptr) {
                res = tmp->c_str();
                res += ": ";
            }
        }
        else if(type == CodeType::CALL) {
            auto tmp = value1->to_son_class<string>();
            if(tmp != nullptr) {
                res = "$";
                res += tmp->c_str();
            }
        }
        else if(type == CodeType::PRINT) {
            auto tmp = value1->to_son_class<string>();
            if(tmp != nullptr) {
                res = "PRINT ";
                res += tmp->c_str();
            }
        }
        else if(type == CodeType::FUNC) {
            auto tmp = value1->to_son_class<string>();
            if(tmp != nullptr) {
                res = "FUNC @";
                res += tmp->c_str();
                res += ":";
            }
        }
        else {
            string a = "", b = "";
            auto t1 = value1->to_son_class<string>();
            if(t1 != nullptr) a = t1->c_str();
            auto t2 = value2->to_son_class<string>();
            if(t2 != nullptr) b = t2->c_str();
            return Types[static_cast<int>(type)] + " " + a + b;
        }
    }
};

#endif