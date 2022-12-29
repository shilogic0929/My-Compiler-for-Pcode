#ifndef EXPS_H
#define EXPS_H

#include <vector>
#include "word.h"
using namespace std;

class Exps {
private:
    vector<vector<Word*> > words;
    vector<Word*> symbols;

public:
    Exps(vector<vector<Word*> >& words, vector<Word*>& symbols) {
        this->words = words;
        this->symbols = symbols;
    }
    vector<vector<Word*> > getWords() {
        return this->words;
    }
    vector<Word*> getSymbols() {
        return this->symbols;
    }
};

#endif