#ifndef KEYWORDS_H
#define _KEYWORDS_H

#include <vector>
#include <string>
#include <algorithm>
#include <map>
using namespace std;

static const map<string, string> keywords = {
    {"main", "MAINTK"},
    {"const", "CONSTTK"},
    {"int", "INTTK"},
    {"break", "BREAKTK"},
    {"continue", "CONTINUETK"},
    {"if", "IFTK"},
    {"else", "ELSETK"},
    {"!", "NOT"},
    {"&&", "AND"},
    {"||", "OR"},
    {"while", "WHILETK"},
    {"getint", "GETINTTK"},
    {"printf", "PRINTFTK"},
    {"return", "RETURNTK"},
    {"+", "PLUS"},
    {"-", "MINU"},
    {"void", "VOIDTK"},
    {"*", "MULT"},
    {"/", "DIV"},
    {"%", "MOD"},
    {"<", "LSS"},
    {"<=", "LEQ"},
    {">", "GRE"},
    {">=", "GEQ"},
    {"==", "EQL"},
    {"!=", "NEQ"},
    {"=", "ASSIGN"},
    {";", "SEMICN"},
    {",", "COMMA"},
    {"(", "LPARENT"},
    {")", "RPARENT"},
    {"[", "LBRACK"},
    {"]", "RBRACK"},
    {"{", "LBRACE"},
    {"}", "RBRACE"},
};

static const string getType(string key) {
    auto it = keywords.find(key);
    if(it != keywords.end()) {
        return keywords.find(key)->second;
    }
    return "UNKNOWN";
}

#endif