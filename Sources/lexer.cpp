#include <bits/stdc++.h>
#include "lexer.h"
using namespace std;
#define ED '\0'

int len;

Lexer::Lexer(string& in) {
    this->idx = 0;
    this->lineNum = 1;
    this->words = vector<Word*>();
    this->code = in;
    len = code.size();
    this->analyse();
    cout << "lexical analyse completed!" << endl;
}

char Lexer::getChar() {
    if(idx < len) {
        char c = code[idx];
        if(c == '\n') lineNum++;
        idx++;
        return c;
    }
    else return ED;
}

void Lexer::unGetChar() {
    char c = code[--idx];
    if(c == '\n') lineNum--;
}

void Lexer::analyse() {
    try {
        char c = ED;
        while((c = getChar()) != ED) {
            if(c == ' ' || c == '\r' || c == '\t') continue;
            if(c == '+' || c == '-' || c == '*' || c == '%') words.emplace_back(new Word(c, lineNum));
            else if(c == '/') analyseSlash();
            else if (c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}') {
                words.emplace_back(new Word(c, lineNum));
            }
            else if (c == '>' || c == '<' || c == '=' || c == '!') {
                analyseRelation(c);
            } 
            else if (c == ',' || c == ';') {
                words.emplace_back(new Word(c, lineNum));
            } 
            else if (c == '"') {
                analyseCitation();
            } 
            else if (c == '&' || c == '|') {
                analyseLogic(c);
            } 
            else if (isdigit(c)) {
                analyseDigit(c);
            } 
            else if (isalpha(c) || c == '_') {
                analyseLetter(c);
            }
        }
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

void Lexer::analyseSlash() {
    char c = getChar();
    if(c == '/') {
        do {
            c = getChar();
            if (c == ED || c == '\n') {
                return;
                // 判断为//注释，结束分析
            }
        } while(1);
    } 
    else if(c == '*') {
        do {
            c = getChar();
            if (c == ED) return;
            if (c == '*') {
                c = getChar();
                if (c == '/') {
                    return;
                    // 判断为/* */注释，直接结束分析
                } 
                else unGetChar();
            }
        } while(1);
    } 
    else {
        words.emplace_back(new Word("/", lineNum));
        unGetChar();
    }
}

void Lexer::analyseRelation(char c) {
    if(c == '=') {
        c = getChar();
        if(c == '=') words.emplace_back(new Word("==", lineNum));
        else {
            unGetChar();
            words.emplace_back(new Word("=", lineNum));
            return;
        }
    } 
    else if(c == '<') {
        c = getChar();
        if (c == '=') words.emplace_back(new Word("<=", lineNum));
        else {
            unGetChar();
            words.emplace_back(new Word("<", lineNum));
        }
    } 
    else if(c == '>') {
        c = getChar();
        if (c == '=') words.emplace_back(new Word(">=", lineNum));
        else {
            unGetChar();
            words.emplace_back(new Word(">", lineNum));
        }
    } 
    else {
        c = getChar();
        if(c == '=') words.emplace_back(new Word("!=", lineNum));
        else {
            unGetChar();
            words.emplace_back(new Word("!", lineNum));
        }
    }
}

void Lexer::analyseCitation() {
    char c = ED;
    string buffer = "";
    while((c = getChar()) != ED) {
        if(c == '"') {
            words.emplace_back(new Word("STRCON", "\"" + buffer + "\"", lineNum));
            return;
        } 
        buffer += c;
    }
}

void Lexer::analyseLogic(char pre) {
    char c = ED;
    if((c = getChar()) != ED) {
        if(pre == '&') {
            if(c == '&') words.emplace_back(new Word("&&", lineNum));
            else {
                unGetChar();
                words.emplace_back(new Word("&", lineNum));
            }
        } 
        else {
            if(c == '|') words.emplace_back(new Word("||", lineNum));
            else {
                unGetChar();
                words.emplace_back(new Word("|", lineNum));
            }
        }
    }
}

void Lexer::analyseDigit(char pre) {
    string builder = "";
    builder += pre;
    char c = ED;
    while((c = getChar()) != ED) {
        if(isdigit(c)) builder += c;
        else {
            unGetChar();
            words.emplace_back(new Word("INTCON", builder, lineNum));
            return;
        }
    }
}

void Lexer::analyseLetter(char pre) {
    string builder = "";
    builder += pre;
    char c = ED;
    while((c = getChar()) != ED) {
        if(isalpha(c) || c == '_' || isdigit(c)) builder += c;
        else {
            unGetChar();
            if(keywords.count(builder)) words.emplace_back(new Word(builder, lineNum));
            else words.emplace_back(new Word("IDENFR", builder, lineNum));
            return;
        }
    }
}