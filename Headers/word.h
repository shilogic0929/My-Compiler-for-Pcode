#ifndef WORD_H
#define WORD_H

#include <bits/stdc++.h>
#include "keywords.h"
using namespace std;

class Word {
private:
    string identification;
    string content;
    string type;
    int lineNum;

public:
    Word(string identification, int lineNum) {
        this->identification = identification;
        this->type = keywords.find(this->identification)->second;
        this->content = this->identification;
        this->lineNum = lineNum;
    }
    Word(char identification, int lineNum) {
        this->identification = ""; this->identification += identification;
        this->type = keywords.find(this->identification)->second;
        this->content = this->identification;
        this->lineNum = lineNum;
    }
    Word(string type, string content, int lineNum) {
        this->type = type;
        this->content = content;
        this->lineNum = lineNum;
    }
    // <type content>
    string outString() {
        return this->type + " " + this->content + "\n";
    }

    bool typeEquals(string type) {
        return this->type == type;
    } 

    string getType() {
        return this->type;
    }

    int getLineNum() {
        return this->lineNum;
    }

    string getContent() {
        return this->content;
    }

    int getFormatNum() {
        int res = 0;
        for(int i = 0; i < content.size() - 1; i++) {
            if(content[i] == '%' && content[i + 1] == 'd') res += 1;
        }
        return res;
    }

    bool isValid(char c) {
        if(c == 32 || c == 33 || (c >= 40 && c <= 126)) return true;
        return false;
    }

    bool illegalFormatString() {
        for(int i = 1; i < content.size() - 1; i++) {
            if(isValid(content[i]) && content[i] == '\\' && content[i + 1] != 'n') return true;
            else if(!isValid(content[i])) {
                if(content[i] == '%' && content[i + 1] == 'd') continue;
                return true;
            }
        }
        return false;
    }

    bool typeForBeginOfExp() {
        if(type == "LPARENT" || type == "IDENFR" || type == "INTCON" || type == "NOT" || type == "PLUS" || type == "MINU")
            return true;
        return false;
    }

    bool typeForStmt() {
        if(type == "IDENFR" || type == "LBRACE" || type == "IFTK" || type == "ELSETK"
            || type == "WHILETK" || type == "BREAKTK" || type == "CONTINUETK" || type == "RETURNTK"
            || type == "PRINTFTK" || type == "SEMICN" || typeForBeginOfExp()) return true;
        return false;
    }

    bool typeForValidateStmt() {
        if(type == "IFTK" || type == "ELSETK" || type == "WHILETK" || type == "CONTINUETK" 
        ||type == "BREAKTK" || type == "PRINTFTK" || type == "SEMICN" || type == "RETURNTK") return true;
        return false;
    }

    bool typeForNotInExp() {
        if(type == "CONSTTK" || type == "INTTK" || type == "BREAKTK" || type == "CONTINUETK" || type == "IFTK" 
        || type == "ELSETK" || type == "WHILETK" || type == "GETINTTK" || type == "PRINTFTK" || type == "RETURNTK")
            return true;
        return false;
    }

    bool typeOfUnary() {
        if(type == "PLUS" || type == "MINU" || type == "NOT") return true;
        return false;
    }
};

#endif