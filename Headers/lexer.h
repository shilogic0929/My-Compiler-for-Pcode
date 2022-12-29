#ifndef LEXER_H
#define LEXER_H

#include <bits/stdc++.h>
#include "word.h"
using namespace std;

class Lexer {
private:
    string code;
    int lineNum = 1;
    int idx = 0;

    void analyse();
    char getChar();
    void analyseSlash();
    void unGetChar();
    void analyseRelation(char c);
    void analyseCitation();
    void analyseLogic(char pre);
    void analyseDigit(char pre);
    void analyseLetter(char pre);

public: 
    explicit Lexer(string& in);
    vector<Word*> words;
};


#endif