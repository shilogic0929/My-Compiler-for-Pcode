#ifndef CG_CODE_TYPE_H
#define CG_CODE_TYPE_H

#include <bits/stdc++.h>
using namespace std;

enum struct CodeType {
    LABEL,
    VAR,
    PUSH,
    POP,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    CMPEQ,
    CMPNE,
    CMPGT,
    CMPLT,
    CMPGE,
    CMPLE,
    AND,
    OR,
    NOT,
    NEG,
    POS,
    JZ,
    JNZ,
    JMP,
    MAIN,
    FUNC,
    ENDFUNC,
    PARA,
    RET,
    CALL,
    RPARA,
    GETINT,
    PRINT,
    DIMVAR,
    VALUE,
    ADDRESS,
    PLACEHOLDER,
    EXIT
};

const string Types[] = {
    "LABEL",
    "VAR",
    "PUSH",
    "POP",
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "MOD",
    "CMPEQ",
    "CMPNE",
    "CMPGT",
    "CMPLT",
    "CMPGE",
    "CMPLE",
    "AND",
    "OR",
    "NOT",
    "NEG",
    "POS",
    "JZ",
    "JNZ",
    "JMP",
    "MAIN",
    "FUNC",
    "ENDFUNC",
    "PARA",
    "RET",
    "CALL",
    "RPARA",
    "GETINT",
    "PRINT",
    "DIMVAR",
    "VALUE",
    "ADDRESS",
    "PLACEHOLDER",
    "EXIT"
};

#endif 