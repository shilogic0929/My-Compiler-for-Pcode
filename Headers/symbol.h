#ifndef SYMBOL_H
#define SYMBOL_H

#include <bits/stdc++.h>
#include "word.h"
using namespace std;

// symbol table item
class Symbol {
private:
    string type;
    int paramType; // paramType: 0:int, 1:int[], 2:int[][], ...
    string content;
    int fieldId = 0;

public:
    Symbol(string type, int paramType, Word* word, int fieldId) {
        this->type = type;
        this->paramType = paramType;
        this->content = word->getContent();
        this->fieldId = fieldId;
    }

public:
    string getType() { return this->type; }

    string getContent() { return this->content; }

    int getParamType() { return this->paramType; }

    int getFieldId() { return this->fieldId; }

    string outString() { return this->content; }

};

#endif