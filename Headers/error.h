#ifndef ERROR_H
#define ERROR_H

#include <bits/stdc++.h>
using namespace std;

class Error {
private:
    int lineNum;
    string type;

public: 
    Error(int lineNum, string type) {
        this->lineNum = lineNum;
        this->type = type;
    }

    int getLineNum() { return this->lineNum; }

    string getType() { return this->type; };

    string outString() {
        return to_string(lineNum) + " " + this->type + "\n";
    }
};

#endif