#ifndef FUNCTION_H
#define FUNCTION_H

#include <bits/stdc++.h>
#include "word.h"
using namespace std;

class Function {
private: 
    string type;
    string content;
    string returnType;
    vector<int> params; // void: -1, int: 0, int[]: 1, int[][]: 2

public: 
    Function(Word* word, string returnType) {
        this->type = word->getType();
        this->content = word->getContent();
        this->returnType = returnType;
    }

    vector<int> getParams() {
        return this->params;
    }

    string getContent() {
        return this->content;
    }

    void setParams(vector<int>& p) {
        this->params = p;
    }

    string getType() { return this->type; }

    int getParamsNum() {
        return (int)this->params.size();
    }

    string getReturnType() {
        return this->returnType;
    }

};

#endif