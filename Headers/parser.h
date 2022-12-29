#ifndef PARSER_H
#define PARSER_H

#include <bits/stdc++.h>
#include "word.h"
#include "exps.h"
#include "symbol.h"
#include "symbol_table.h"
#include "function.h"
#include "error.h"
#include "cg_pcode.h"
#include "cg_return_info.h"
#include "cg_variable.h"
#include "cg_code_type.h"
#include "cg_func.h"
#include "cg_label_generator.h"
#include "cg_object.h"
#include "cg_pcode_exec.h"
using namespace std;

/**
 * strategy:
 * normal rule: keep getting words and analyse
 * expression rule: scan the whole expression first, which is implemented by function getExp.
 * Then divide the expression and wield recursive descent method to analyse them.
 */
class Parser {
private:
    int idx = 0;
    int field = -1;
    int flagWHILE = 0; // if the current block is in a while circle
    bool requireReturn = false; // if the current function is required to return

    Word* curWord; // to dispaly current word when reading vector<Word*> words from lexer one by one
    vector<Word*> words;
    map<int, SymbolTable*> symbols;
    map<string, Function*> functions;

private:
    int fieldId = -1;
    LabelGenerator* labelGenerator = nullptr;
    vector<map<string, string> > ifLabels;
    vector<map<string, string> > whileLabels;
    vector<map<int, string> > condLabels;

private:
    string outString(string s);
    void addField();
    void popField();

    bool undefinedSymbol(Word* word); // undefined symbol in all fields
    bool repeatedSymbolInCurrentField(Word* word); // there's repeat symbol in current field
    bool undefinedFunction(Word* word); // undefined function name in all fields
    bool isConst(Word* word);

    void addSymbol(Word* word, string type, int intType, int fieldId);
    Symbol* getSymbol(Word* word);
    Function* getFunction(Word* word);

    void getWord();
    void getWordWithoutAddToGrammar();
    Word* getNextWord();
    Word* getNext2Word();
    Word* getNext3Word();

    void checkRightBrack();
    void checkRightParent();
    void checkSemicn();
    void checkParamsMatchRParams(Word* ident, vector<int>& params, vector<int>& rparams);

    void analyseCompUnit(); 
    void analyseDecl(); 
    void analyseConstDef(); 
    void analyseFuncDef(); 
    void analyseMainFuncDef(); 
    bool analyseBlock(bool fromFunc); 
    bool analyseBlockItem();
    bool analyseStmt();
    void analyseFuncFParams(vector<int>& params);
    int analyseFuncFParam();
    string analyseFuncType();
    void analyseConstInitVal();
    void analyseBType();
    void analyseConstDecl();
    void analyseVarDecl();
    void analyseVarDef();
    void analyseInitVal();
    void analyseCond(string from);

    int analyseExp(vector<Word*>& exp);
    void analyseFuncRParams(Word* ident, vector<Word*>& exp, vector<int>& params);
    void analyseRelExp(vector<Word*>& exp);
    void analyseEqExp(vector<Word*>& exp);
    void analyseLAndExp(vector<Word*>& exp, string from, string label);
    void analyseLOrExp(vector<Word*>& exp, string from);
    int analyseLVal(vector<Word*>& exp);
    void analyseNumber(Word* word);
    int analysePrimaryExp(vector<Word*>& exp);
    int analyseUnaryExp(vector<Word*>& exp);
    void analyseUnaryOp(Word* word);

    Exps* divideExp(vector<Word*>& words, vector<string>& symbol);
    int analyseMulExp(vector<Word*>& exp);
    int analyseAddExp(vector<Word*>& exp);
    void analyseConstExp(vector<Word*>& exp);
    
    void getExp(vector<Word*>& exp);

    void error(); // To do
    void error(string type);
    void error(string type, int lineNum);

public:
    explicit Parser(vector<Word*>& words);
    vector<string> grammar;
    vector<Error*> errors;
    vector<PCode*> codeList;
};

#endif