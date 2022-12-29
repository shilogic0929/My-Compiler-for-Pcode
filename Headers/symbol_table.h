#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <map>
#include "symbol.h"
#include "word.h"
using namespace std;

class SymbolTable {
private: 
    map<string, Symbol*> symbolTable;

public:
    SymbolTable() {
        symbolTable = map<string, Symbol*>();
    }

    bool hasSymbol(Word* word) {
        if(symbolTable.find(word->getContent()) != symbolTable.end()) return true;
        return false; 
    }

    void addSymbol(string type, int intType, Word* word, int fieldId) {
        symbolTable[word->getContent()] = new Symbol(type, intType, word, fieldId);
    }

    bool isConst(Word* word) {
        if(symbolTable.find(word->getContent()) == symbolTable.end()) return false;
        string t = symbolTable[word->getContent()]->getType();
        if(t == "const") return true;
        return false;
    }

    Symbol* getSymbol(Word* word) {
        return symbolTable[word->getContent()];
    }

    string outString() {
        if(!symbolTable.size()) return "{}";
        string res = "";
        int i = 1;
        for(auto it : symbolTable) {
            res += "{key"; res += to_string(i); res += ":"; res += it.first;
            res += ", ";
            res += "value"; res += to_string(i); res += ":"; res += it.second->getContent(); res += "}, ";
            i++;
        }
        return res;
    }
};

#endif