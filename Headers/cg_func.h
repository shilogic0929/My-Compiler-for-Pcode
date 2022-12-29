#ifndef CG_FUNC_H
#define CG_FUNC_H

class Func {
private:
    int idx;
    int args;

public:
    Func(int idx, int args) {
        this->idx = idx;
        this->args = args;
    }
    int getIdx() {
        return this->idx;
    }
    int getArgs() {
        return this->args;
    }
};

#endif