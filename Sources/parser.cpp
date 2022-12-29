#include <bits/stdc++.h>
#include "parser.h"
#include "word.h"
#include "error.h"
#include "symbol.h"
#include "symbol_table.h"
#include "cg_pcode.h"
#include "cg_return_info.h"
#include "cg_variable.h"
#include "cg_code_type.h"
#include "cg_func.h"
#include "cg_label_generator.h"
#include "cg_object.h"
#include "cg_pcode_exec.h"
using namespace std;

Parser::Parser(vector<Word*>& words) {
    this->idx = 0;
    this->field = -1;
    this->flagWHILE = 0;
    this->requireReturn = false;
    this->words = words;
    this->grammar = vector<string>();
    this->errors = vector<Error*>();
    this->symbols = map<int, SymbolTable*>();
    this->functions = map<string, Function*>();
    this->labelGenerator = new LabelGenerator();
    analyseCompUnit();
    sort(errors.begin(), errors.end(), [](Error* e1, Error* e2) {
        return e1->getLineNum() < e2->getLineNum();
    });
    for(auto& sbt : symbols) {
        if(sbt.second) delete sbt.second;
    }
    for(auto& func : functions) {
        if(func.second) delete func.second;
    }
    map<int, SymbolTable*>().swap(symbols);
    map<string, Function*>().swap(functions);
    if(labelGenerator != nullptr) delete labelGenerator;
    cout << "grammartical analyse compeleted!" << endl;
}

string Parser::outString(string s) {
    return s + "\n";
}

void Parser::addField() {
    symbols[++field] = new SymbolTable();
    fieldId++;
}

void Parser::popField() {
    symbols.erase(field--);
}

bool Parser::repeatedSymbolInCurrentField(Word* word) {
    // if(field == -1) return false;
    if(symbols[field]->hasSymbol(word)) return true;
    return false;
}

bool Parser::undefinedFunction(Word* word) {
    if(functions.find(word->getContent()) == functions.end()) return true;
    return false;
}

bool Parser::undefinedSymbol(Word* word) {
    for(auto ss : symbols) {
        if(ss.second->hasSymbol(word)) return false;
    }
    return true;
}

void Parser::addSymbol(Word* word, string type, int paramType, int fieldId) {
    symbols[field]->addSymbol(type, paramType, word, fieldId);
}

Symbol* Parser::getSymbol(Word* word) {
    Symbol* sb = nullptr;
    for(auto ss : symbols) {
        if(ss.second->hasSymbol(word)) {
            sb = ss.second->getSymbol(word);
        }
    }
    return sb;
}

Function* Parser::getFunction(Word* word) {
    Function* res = nullptr;
    if(functions.find(word->getContent()) != functions.end()) res = functions[word->getContent()];
    return res;
}

void Parser::getWord() {
    curWord = words[idx++];
    grammar.emplace_back(curWord->outString());
}

void Parser::getWordWithoutAddToGrammar() { curWord = words[idx++]; }

Word* Parser::getNextWord() { return words[idx]; }

Word* Parser::getNext2Word() { return words[idx + 1]; }

Word* Parser::getNext3Word() { return words[idx + 2]; }

// if there is a '[', for example, array[], check the right bracket
void Parser::checkRightBrack() {
    Word* word = getNextWord();
    if(!word->typeEquals("RBRACK")) error("k");
    else getWord(); // ']'
}

// if there is a '(', for example, fundtional call UnaryExp, Function Definition or Stmt, check the right parent
void Parser::checkRightParent() {
    Word* word = getNextWord();
    if(!word->typeEquals("RPARENT")) error("j");
    else getWord(); // ')'
}

void Parser::checkSemicn() {
    Word* word = getNextWord();
    if(!word->typeEquals("SEMICN")) error("i");
    else getWord(); // `;`
}

void Parser::checkParamsMatchRParams(Word* ident, vector<int>& params, vector<int>& rparams) {
    if(params.size() != rparams.size()) error("d", ident->getLineNum());
    else {
        for(int i = 0; i < params.size(); i++) {
            if(params[i] != rparams[i]) {
                error("e", ident->getLineNum());
                break;
            }
        }
    }
}

bool Parser::isConst(Word* word) {
    for(auto sb : symbols) {
        if(sb.second->hasSymbol(word) && sb.second->isConst(word)) return true;
    }
    return false;
}

// CompUnit → {Decl} {FuncDef} MainFuncDef
void Parser::analyseCompUnit() {
    addField();
    
        Word* word = getNextWord();
        while(word->typeEquals("CONSTTK") || 
            (word->typeEquals("INTTK") && getNext2Word()->typeEquals("IDENFR") && !getNext3Word()->typeEquals("LPARENT"))) {
            // like `const ...` or `int variable ...` 
            analyseDecl(); // Decl
            word = getNextWord();
        }
        while(word->typeEquals("VOIDTK") || (word->typeEquals("INTTK") && !getNext2Word()->typeEquals("MAINTK"))) {
            analyseFuncDef(); // FuncDef
            word = getNextWord();
        }
        if(word->typeEquals("INTTK") && getNext2Word()->typeEquals("MAINTK")) 
            analyseMainFuncDef(); // MainFuncDef
        else error();
    
    popField();
    grammar.emplace_back(outString("<CompUnit>"));
}

// Decl → ConstDecl | VarDecl
void Parser::analyseDecl() {
    Word* word = getNextWord();
    if(word->typeEquals("CONSTTK")) analyseConstDecl();
    else if (word->typeEquals("INTTK")) analyseVarDecl();
    else error();
}

// ConstDef → Ident { '[' ConstExp ']' } '=' ConstInitVal
void Parser::analyseConstDef() {
    getWord(); // Ident
    Word* ident = curWord;
    if(repeatedSymbolInCurrentField(ident)) error("b");
    
    codeList.push_back(
        new PCode(CodeType::VAR, new Packing<string>(to_string(fieldId) + "_" + ident->getContent()))
    );
    int paramType = 0;
    Word* word = getNextWord();
    vector<Word*> constExp;
    while(word->typeEquals("LBRACK")) {
        paramType += 1;
        getWord(); // `[`
        getExp(constExp); // ConstExp
        analyseConstExp(constExp);
        checkRightBrack(); // `]`
        word = getNextWord();
    }
    if(paramType > 0) {
        codeList.push_back(
            new PCode(
                CodeType::DIMVAR, 
                new Packing<string>(to_string(fieldId) + "_" + ident->getContent()), 
                new Packing<int>(paramType) 
            )
        );
    } 
    addSymbol(ident, "const", paramType, fieldId); // add to symbolTable[field]
    getWord(); // `=`
    analyseConstInitVal(); // ConstInitVal
    grammar.emplace_back(outString("<ConstDef>"));
    vector<Word*>().swap(constExp);
}

// FuncDef → FuncType Ident '(' [FuncFParams] ')' Block
void Parser::analyseFuncDef() {
    Function* func = nullptr;
    vector<int> params = vector<int>();
    string funcType = analyseFuncType(); // get return type
    getWord(); // Ident
    if(functions.find(curWord->getContent()) != functions.end()) error("b");

    PCode* code = new PCode(
        CodeType::FUNC, 
        new Packing<string>(curWord->getContent())
    );
    codeList.push_back(code);
    func = new Function(curWord, funcType);
    addField(); // now enter a new field
    {
        getWord(); // `(`
        // analyse params
        Word* word = getNextWord();
        if(word->typeEquals("INTTK") || word->typeEquals("VOIDTK")) analyseFuncFParams(params);
        checkRightParent(); // `)`
        func->setParams(params);
        functions[func->getContent()] = func;
        if(func->getReturnType() == "int") {
            requireReturn = true;
        } else requireReturn = false;

        bool isReturn = analyseBlock(1);
        if(requireReturn && !isReturn) error("g");
    }
    popField();
    code->setValue2(new Packing<int>(params.size()));
    codeList.push_back(
        new PCode(CodeType::RET, new Packing<int>(0))
    );
    codeList.push_back(
        new PCode(CodeType::ENDFUNC)
    );
    grammar.emplace_back(outString("<FuncDef>"));
}

// MainFuncDef → 'int' 'main' '(' ')' Block
void Parser::analyseMainFuncDef() {
    for(int i = 0; i < 2; i++) getWord(); // `int`, `main`
    if(functions.find(curWord->getContent()) != functions.end()) error("b");
    else {
        Function* func = new Function(curWord, "int");
        vector<int> params = vector<int>();
        func->setParams(params);
        functions["main"] = func;
        vector<int>().swap(params);
    }
    codeList.push_back(
        new PCode(CodeType::MAIN, new Packing<string>(curWord->getContent()))
    );
    getWord(); // `(`
    checkRightParent(); // `)`
    requireReturn = true;
    bool isReturn = analyseBlock(false); // note that main function' field is global
    if(requireReturn && !isReturn) error("g");
    codeList.push_back(
        new PCode(CodeType::EXIT)
    );
    grammar.emplace_back(outString("<MainFuncDef>"));
}

// Block → '{' { BlockItem } '}'
bool Parser::analyseBlock(bool fromFunc) {
    getWord(); // `{`
    if(!fromFunc) addField();
    
        bool isReturn = false;
        Word* word = getNextWord();
        while(word->typeEquals("CONSTTK") || word->typeEquals("INTTK") || word->typeForStmt()) {
            isReturn = analyseBlockItem(); // BlockItem
            word = getNextWord();
        }
        getWord(); // `}`
    
    if(!fromFunc) popField();
    grammar.emplace_back(outString("<Block>"));
    return isReturn;
}

// BlockItem → Decl | Stmt
bool Parser::analyseBlockItem() {
    Word* word = getNextWord();
    bool isReturn = false;
    if(word->typeEquals("INTTK") || word->typeEquals("CONSTTK")) analyseDecl();
    else isReturn = analyseStmt();
    return isReturn;
}

/**
 * recursively analyse
 * Stmt → LVal '=' Exp ';'
 * | [Exp] ';' // Exp exists ot not
 * | Block 
 * | 'if' '(' Cond ')' Stmt [ 'else' Stmt ] // `else` exists or not
 * | 'while' '(' Cond ')' Stmt 
 * | 'break' ';' 
 * | 'continue' ';' 
 * | 'return' [Exp] ';' // Exp exists or not
 * | LVal '=' 'getint''('')'';' 
 * | 'printf''('FormatString{','Exp}')'';' // Exp exists or not
 */
bool Parser::analyseStmt() {
    bool isReturn = false;
    Word* word = getNextWord();
    vector<Word*> exp;
    if(word->typeEquals("IDENFR")) {
        getExp(exp);
        // note that `IDENFR` could also symbolise an `Exp`, so the exp fetched by getExp() could be like `Exp;`, not exp for LVal
    	// so we need to differentiate the two situations here
        if(getNextWord()->typeEquals("ASSIGN")) {
            Word* ident = exp[0];
            int paramType = analyseLVal(exp); // LVal
            codeList.push_back(
                new PCode(
                    CodeType::ADDRESS, 
                    new Packing<string>(to_string(getSymbol(ident)->getFieldId()) + "_" + ident->getContent()),
                    new Packing<int>(paramType)
                )
            );
            if(isConst(word)) error("h", word->getLineNum());
            getWord(); // `=`
            if(!getNextWord()->typeEquals("GETINTTK")) {
                getExp(exp); // Exp
                analyseExp(exp); 
                checkSemicn(); // `;`
            }
            else {
                for(int i = 0; i < 2; i++) getWord(); // `getint`, `(`
                checkRightParent(); // `)`
                checkSemicn(); // `;`
                codeList.push_back(
                    new PCode(CodeType::GETINT)
                );
            }
            codeList.push_back(
                new PCode(
                    CodeType::POP, new Packing<string>(to_string(getSymbol(ident)->getFieldId()) + "_" + ident->getContent())
                )
            );
        } 
        else {
            analyseExp(exp); // Exp
            checkSemicn(); // `;`
        }
    } 
    else if(word->typeForBeginOfExp()) {
        getExp(exp); // Exp
        analyseExp(exp);
        checkSemicn(); // `;`
    } 
    else if(word->typeEquals("LBRACE")) analyseBlock(false);
    else if(word->typeEquals("IFTK")) {
        map<string, string> mp = {
            {"if", labelGenerator->getLabel("if")},
            {"else", labelGenerator->getLabel("else")},
            {"if_end", labelGenerator->getLabel("if_end")},
            {"if_block", labelGenerator->getLabel("if_block")}
        };
        ifLabels.push_back(mp);
        codeList.push_back(
            new PCode(CodeType::LABEL, new Packing<string>(mp["if"]))
        );
        for(int i = 0; i < 2; i++) getWord(); // `if`, `(`
        analyseCond("IFTK"); // Cond
        checkRightParent(); // `)`
        codeList.push_back(
            new PCode(CodeType::JZ, new Packing<string>(mp["else"]))
        );
        codeList.push_back(
            new PCode(CodeType::LABEL, new Packing<string>(mp["if_block"]))
        );
        analyseStmt();
        word = getNextWord();
        codeList.push_back(
            new PCode(CodeType::JMP, new Packing<string>(mp["if_end"]))
        );
        codeList.push_back(
            new PCode(CodeType::LABEL, new Packing<string>(mp["else"]))
        );
        if(word->typeEquals("ELSETK")) {
            getWord(); // `else`
            analyseStmt();
        }
        codeList.push_back(
            new PCode(CodeType::LABEL, new Packing<string>(mp["if_end"]))
        );
        ifLabels.pop_back();
    } 
    else if(word->typeEquals("WHILETK")) {
        map<string, string> mp = {
            {"while", labelGenerator->getLabel("while")},
            {"while_end", labelGenerator->getLabel("while_end")},
            {"while_block", labelGenerator->getLabel("while_block")}
        };
        whileLabels.push_back(mp);
        codeList.push_back(
            new PCode(CodeType::LABEL, new Packing<string>(mp["while"]))
        );
        getWord(); // `while`
        flagWHILE += 1;
            getWord(); // `(`
            analyseCond("WHILETK"); // Cond
            checkRightParent(); // `)`
            codeList.push_back(
                new PCode(CodeType::JZ, new Packing<string>(mp["while_end"]))
            );
            codeList.push_back(
                new PCode(CodeType::LABEL, new Packing<string>(mp["while_block"]))
            );
            analyseStmt();
        flagWHILE -= 1;
        codeList.push_back(
            new PCode(CodeType::JMP, new Packing<string>(mp["while"]))
        );
        codeList.push_back(
            new PCode(CodeType::LABEL, new Packing<string>(mp["while_end"]))
        );
        whileLabels.pop_back();
    } 
    else if(word->typeEquals("BREAKTK") || word->typeEquals("CONTINUETK")) {
        getWord(); // `break` or `continue`
        if(word->typeEquals("CONTINUETK")) {
            codeList.push_back(
                new PCode(CodeType::JMP, new Packing<string>(whileLabels.back()["while"]))
            );
        }
        else {
            codeList.push_back(
                new PCode(CodeType::JMP, new Packing<string>(whileLabels.back()["while_end"]))
            );
        }
        if(!flagWHILE) error("m");
        checkSemicn(); // `;`
    }
    else if(word->typeEquals("RETURNTK")) {
        int flagRETURN = false;
        getWord(); // `return`
        isReturn = true;
        Word* nxtWord = getNextWord();
        if(nxtWord->typeForBeginOfExp()) {
            if(!requireReturn && isReturn) error("f");
            getExp(exp);
            analyseExp(exp);
            flagRETURN = true;
        }
        codeList.push_back(
            new PCode(CodeType::RET, new Packing<int>(flagRETURN))
        );
        checkSemicn(); // `;`
    }
    else if(word->typeEquals("PRINTFTK")) {
        Word* printftk = nullptr;
        Word* strcon = nullptr;
        getWord(); // `printf`
        printftk = curWord;
        getWord(); // `(`
        getWord(); // `STRCON`
        strcon = curWord;
        word = getNextWord();
        int paramCnt = 0;
        while(word->typeEquals("COMMA")) {
            getWord(); // `,`
            getExp(exp);
            analyseExp(exp);
            paramCnt += 1;
            word = getNextWord();
        }
        if(strcon->illegalFormatString()) error("a", strcon->getLineNum());
        if(paramCnt != strcon->getFormatNum()) error("l", printftk->getLineNum());
        checkRightParent(); // `)`
        checkSemicn(); // `;`
        codeList.push_back(
            new PCode(
                CodeType::PRINT, 
                new Packing<string>(strcon->getContent()),
                new Packing<int>(paramCnt)
            )
        );
    } 
    else if(word->typeEquals("SEMICN")) checkSemicn(); // `;`
    grammar.emplace_back(outString("<Stmt>"));
    vector<Word*>().swap(exp);
    return isReturn;
}

// FuncFParams → FuncFParam { ',' FuncFParam }
void Parser::analyseFuncFParams(vector<int>& params) {
    vector<int>().swap(params);
    int paramType = analyseFuncFParam();
    params.emplace_back(paramType);
    Word* word = getNextWord();
    while(word->typeEquals("COMMA")) {
        getWord(); // `,`
        paramType = analyseFuncFParam();
        params.emplace_back(paramType);
        word = getNextWord();
    }
    grammar.emplace_back(outString("<FuncFParams>"));
}

// FuncFParam → BType Ident ['[' ']' { '[' ConstExp ']' }]
int Parser::analyseFuncFParam() {
    int paramType = 0;
    for(int i = 0; i < 2; i++) getWord(); // `void`|`int`, Ident
    Word* ident = curWord;
    if(repeatedSymbolInCurrentField(ident)) error("b");
    Word* word = getNextWord();
    if(word->typeEquals("LBRACK")) {
        paramType += 1;
        getWord(); // `[`
        checkRightBrack(); // `]`
        word = getNextWord();
        vector<Word*> exp;
        while(word->typeEquals("LBRACK")) {
            paramType += 1;
            getWord(); // `[`
            getExp(exp);
            analyseConstExp(exp);
            checkRightBrack(); // `]`
            word = getNextWord();
        }
    }
    codeList.push_back(
        new PCode(
            CodeType::PARA, 
            new Packing<string>(to_string(fieldId) + "_" + ident->getContent()),
            new Packing<int>(paramType)
        )
    );
    addSymbol(ident, "para", paramType, fieldId);
    grammar.emplace_back(outString("<FuncFParam>"));
    return paramType;
}

// FuncType → 'void' | 'int'
string Parser::analyseFuncType() {
    getWord(); // `void` | `int`
    grammar.emplace_back(outString("<FuncType>"));
    return curWord->getContent();
}

// ConstInitVal → ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
void Parser::analyseConstInitVal() {
    Word* word = getNextWord();
    if(word->typeEquals("LBRACE")) {
        getWord(); // `{`
        word = getNextWord();
        if(!word->typeEquals("RBRACE")) {
            analyseConstInitVal();
            Word* tmp = getNextWord();
            while(tmp->typeEquals("COMMA")) {
                getWord(); // `,`
                analyseConstInitVal();
                tmp = getNextWord();
            }
        }
        getWord(); // `}`
    } 
    else {
        vector<Word*> exp;
        getExp(exp);
        analyseConstExp(exp);
        vector<Word*>().swap(exp);
    }
    grammar.emplace_back(outString("<ConstInitVal>"));
}

// BType → 'int'
void Parser::analyseBType() { /*NOTHING TODO*/ }

// ConstDecl → 'const' BType ConstDef { ',' ConstDef } ';'
void Parser::analyseConstDecl() {
    for(int i = 0; i < 2; i++) getWord(); // `const`, `int`
    if(curWord->typeEquals("INTTK")) analyseBType();
    else error();
    
    analyseConstDef();
    Word* word = getNextWord();
    while(word->typeEquals("COMMA")) {
        getWord(); // `,`
        analyseConstDef();
        word = getNextWord();
    }
    checkSemicn(); // `;`
    grammar.emplace_back(outString("<ConstDecl>"));
}

// VarDecl → BType VarDef { ',' VarDef } ';'
void Parser::analyseVarDecl() {
    getWord(); // `int`
    analyseVarDef();
    Word* word = getNextWord();
    while(word->typeEquals("COMMA")) {
        getWord(); // `,`
        analyseVarDef();
        word = getNextWord();
    }
    checkSemicn(); // `;`
    grammar.emplace_back(outString("<VarDecl>"));
}

// VarDef → Ident { '[' ConstExp ']' } | Ident { '[' ConstExp ']' } '=' InitVal
void Parser::analyseVarDef() {
    getWord(); // Ident
    Word* ident = curWord;
    if(repeatedSymbolInCurrentField(ident)) error("b");

    codeList.push_back(
        new PCode(CodeType::VAR, new Packing<string>(to_string(fieldId) + "_" + ident->getContent()))
    );
    int paramType = 0;
    Word* word = getNextWord();
    vector<Word*> constExp;
    while(word->typeEquals("LBRACK")) {
        paramType += 1;
        getWord(); // `[`
        getExp(constExp);
        analyseConstExp(constExp);
        checkRightBrack(); // `]`
        word = getNextWord();
    }
    if(paramType > 0) {
        codeList.push_back(
            new PCode(
                CodeType::DIMVAR, 
                new Packing<string>(to_string(fieldId) + "_" + ident->getContent()),
                new Packing<int>(paramType)
            )
        );
    }
    addSymbol(ident, "var", paramType, fieldId);
    if(word->typeEquals("ASSIGN")) {
        getWord(); // `=`
        analyseInitVal();
    }
    else {
        codeList.push_back(
            new PCode(
                CodeType::PLACEHOLDER,
                new Packing<string>(to_string(fieldId) + "_" + ident->getContent()),
                new Packing<int>(paramType)
            )
        );
    }
    grammar.emplace_back(outString("<VarDef>"));
    vector<Word*>().swap(constExp);
}

// InitVal → Exp | '{' [ InitVal { ',' InitVal } ] '}'
void Parser::analyseInitVal() {
    Word* word = getNextWord();
    if(!word->typeEquals("LBRACE")) { // anaylse Exp
        vector<Word*> exp;
        getExp(exp);
        analyseExp(exp);
        vector<Word*>().swap(exp);
    } 
    else {
        getWord(); // `{`
        word = getNextWord();
        if(!word->typeEquals("RBRACE")) {
            analyseInitVal();
            Word* nxtWord = getNextWord();
            while(nxtWord->typeEquals("COMMA")) {
                getWord(); // `,`
                analyseInitVal();
                nxtWord = getNextWord();
            }
        }
        getWord(); // `}`
    }
    grammar.emplace_back(outString("<InitVal>"));
}

// Exp → AddExp
int Parser::analyseExp(vector<Word*>& exp) {
    int paramType = analyseAddExp(exp);
    grammar.emplace_back(outString("<Exp>"));
    return paramType;
}

// Cond → LOrExp
void Parser::analyseCond(string from) {
    vector<Word*> exp;
    getExp(exp);
    analyseLOrExp(exp, from);
    grammar.emplace_back(outString("<Cond>"));
    vector<Word*>().swap(exp);
}

// FuncRParams → Exp { ',' Exp }
void Parser::analyseFuncRParams(Word* ident, vector<Word*>& exp, vector<int>& params) {
    vector<string> symbol = {"COMMA"}; 
    vector<int> rparams = vector<int>();
    Exps* exps = divideExp(exp, symbol);
    int i = 0;
    for(auto exp : exps->getWords()) {
        int paramType = analyseExp(exp);
        rparams.emplace_back(paramType);
        codeList.push_back(
            new PCode(CodeType::RPARA, new Packing<int>(paramType))
        );
        if(i < exps->getSymbols().size()) {
            grammar.emplace_back(exps->getSymbols()[i]->outString());
            i += 1;
        }
    }
    checkParamsMatchRParams(ident, params, rparams);
    vector<int>().swap(rparams);
    grammar.emplace_back(outString("<FuncRParams>"));
}

/**
 * RelExp → AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
 * left recursion must be avoided, so I rewrite it like: 
 * RelExp → AddExp ('<' | '>' | '<=' | '>=') AddExp ('<' | '>' | '<=' | '>=') AddExp ... 
 */
void Parser::analyseRelExp(vector<Word*>& exp) {
    vector<string> symbol = {"LSS", "LEQ", "GRE", "GEQ"};
    Exps* exps = divideExp(exp, symbol);
    int i = 0;
    for(auto e : exps->getWords()) {
        analyseAddExp(e);
        if(i) {
            CodeType tp;
            if(exps->getSymbols()[i - 1]->typeEquals("LSS")) tp = CodeType::CMPLT;
            else if(exps->getSymbols()[i - 1]->typeEquals("LEQ")) tp = CodeType::CMPLE;
            else if(exps->getSymbols()[i - 1]->typeEquals("GRE")) tp = CodeType::CMPGT;
            else tp = CodeType::CMPGE;
            codeList.push_back(new PCode(tp));
        }
        grammar.emplace_back(outString("<RelExp>"));
        if(i < exps->getSymbols().size()) {
            grammar.emplace_back(exps->getSymbols()[i]->outString());
            i += 1;
        }
    }
}

/**
 * EqExp → RelExp | EqExp ('==' | '!=') RelExp
 * Rewrite: EqExp → RelExp ('==' | '!=') RelExp ('==' | '!=') RelExp ... 
 */
void Parser::analyseEqExp(vector<Word*>& exp) {
    vector<string> symbol = {"EQL", "NEQ"};
    Exps* exps = divideExp(exp, symbol);
    int i = 0;
    for(auto e : exps->getWords()) {
        analyseRelExp(e);
        if(i) {
            CodeType tp;
            if(exps->getSymbols()[i - 1]->typeEquals("EQL")) tp = CodeType::CMPEQ;
            else tp = CodeType::CMPNE;
            codeList.push_back(new PCode(tp));
        }
        grammar.emplace_back(outString("<EqExp>"));
        if(i < exps->getSymbols().size()) {
            grammar.emplace_back(exps->getSymbols()[i]->outString());
            i += 1;
        }
    }
}

/**
 * LAndExp → EqExp | LAndExp '&&' EqExp
 * Rewrite: LAndExp → EqExp '&&' EqExp '&&' ...
 */
void Parser::analyseLAndExp(vector<Word*>& exp, string from, string label) {
    vector<string> symbol = {"AND"};
    Exps* exps = divideExp(exp, symbol);
    int i = 0, j = 0;
    for(auto e : exps->getWords()) {
        analyseEqExp(e);
        if(i) codeList.push_back(new PCode(CodeType::AND));
        if(exps->getWords().size() > 1 && j < exps->getWords().size() - 1) {
            // if(from == "IKTK") {
            //     codeList.push_back(
            //         new PCode(CodeType::JZ, new Packing<string>(label))
            //     );
            // }
            // else {
            //     codeList.push_back(
            //         new PCode(CodeType::JZ, new Packing<string>(label))
            //     );
            // }
            codeList.push_back(
                new PCode(CodeType::JZ, new Packing<string>(label))
            );
        }
        grammar.emplace_back(outString("<LAndExp>"));
        if(i < exps->getSymbols().size()) {
            grammar.emplace_back(exps->getSymbols()[i]->outString());
            i += 1;
        }
        j += 1;
    }
}

/**
 * LOrExp → LAndExp | LOrExp '||' LAndExp
 * Rewrite: LOrExp → LAndExp '||' LAndExp '||' LAndExp ...
 */
void Parser::analyseLOrExp(vector<Word*>& exp, string from) {
    vector<string> symbol = {"OR"};
    Exps* exps = divideExp(exp, symbol);
    int i = 0, j = 0;
    for(auto e : exps->getWords()) {
        string label = labelGenerator->getLabel("cond_" + to_string(j));
        analyseLAndExp(e, from, label);
        codeList.push_back(
            new PCode(CodeType::LABEL, new Packing<string>(label))
        );
        if(i > 0) {
            codeList.push_back(
                new PCode(CodeType::OR)
            );
        }
        if(exps->getWords().size() > 1 && j != exps->getWords().size() - 1) {
            if(from == "IFTK") {
                codeList.push_back(
                    new PCode(CodeType::JNZ, new Packing<string>(ifLabels.back()["if_block"]))
                );
            }
            else {
                codeList.push_back(
                    new PCode(CodeType::JNZ, new Packing<string>(whileLabels.back()["while_block"]))
                );
            }
        }
        grammar.emplace_back(outString("<LOrExp>"));
        if(i < exps->getSymbols().size()) {
            grammar.emplace_back(exps->getSymbols()[i]->outString());
            i += 1;
        }
        j += 1;
    }
}

// [l, r)
inline vector<Word*> splitVector(vector<Word*>& exp, int l, int r) {
    vector<Word*> spiltExp = vector<Word*>();
    for(int i = l; i < r; i++) spiltExp.emplace_back(exp[i]);
    return spiltExp;
}

// LVal → Ident {'[' Exp ']'}
int Parser::analyseLVal(vector<Word*>& exp) {
    Word* ident = exp[0];
    if(undefinedSymbol(ident)) error("c", ident->getLineNum());

    codeList.push_back(
        new PCode(
            CodeType::PUSH, 
            new Packing<string>(to_string(getSymbol(ident)->getFieldId()) + "_" + ident->getContent())
        )
    );
    grammar.emplace_back(exp[0]->outString()); // Ident
    int paramType = 0;
    if(exp.size() >= 2) {
        vector<Word*> e = vector<Word*>();
        int flagBRACK = 0;
        for(auto word : exp) {
            if(word->typeEquals("LBRACK")) {
                flagBRACK += 1;
                if(flagBRACK == 1) {
                    paramType += 1;
                    grammar.emplace_back(word->outString());
                    vector<Word*>().swap(e);
                } 
                else e.emplace_back(word);
            } 
            else if(word->typeEquals("RBRACK")) {
                flagBRACK--;
                if(!flagBRACK) {
                    analyseExp(e);
                    grammar.emplace_back(word->outString());
                } 
                else e.emplace_back(word);
            } 
            else e.emplace_back(word);
        }
        // fault
            if(0) {
                // note that Exp might be [Exp][Exp], so you can't write like this:
                if(exp[1]->typeEquals("LBRACK")) {
                    grammar.emplace_back(exp[1]->outString());
                    vector<Word*> splitExp = splitVector(exp, 2, (int)exp.size() - 1);
                    analyseExp(splitExp);
                    grammar.emplace_back(exp.back()->outString());
                }
                else {
                    vector<Word*> splitExp = splitVector(exp, 1, (int)exp.size());
                    analyseExp(splitExp);
                }
            }
        //
        if(flagBRACK) {
            analyseExp(e);
            error("k", exp.back()->getLineNum());
        }
        vector<Word*>().swap(e);
    }
    grammar.emplace_back(outString("<LVal>"));
    // note that, for example, there is a function like `int func(int a)`, where its parameter type is 0
    // now, arr[2][2] are defined, so parameter type of arr[0] is 1, and parameter type of arr[0][0] is 0
    // so the actual type of parameter is ident's original type - paramType
    if(!undefinedSymbol(ident)) return getSymbol(ident)->getParamType() - paramType;
    return 0;
}

// Number → IntConst
void Parser::analyseNumber(Word* word) {
    codeList.push_back(
        new PCode(
            CodeType::PUSH, 
            new Packing<int>(stoi(word->getContent())))
    );
    grammar.emplace_back(word->outString());
    grammar.emplace_back(outString("<Number>"));
}

// PrimaryExp → '(' Exp ')' | LVal | Number
int Parser::analysePrimaryExp(vector<Word*>& exp) {
    int paramType = 0;
    if(exp[0]->typeEquals("LPARENT")) { // `(` Exp `)`
        // remove `(` `)`
        grammar.emplace_back(exp[0]->outString());
        if(!exp.back()->typeEquals("RPARENT")) {
            error("j", exp.back()->getLineNum());
            exp.emplace_back(new Word(")", exp.back()->getLineNum()));
        }
        vector<Word*> splitExp = splitVector(exp, 1, (int)exp.size() - 1);
        analyseExp(splitExp);
        grammar.emplace_back(exp.back()->outString());
        vector<Word*>().swap(splitExp);
    } 
    else if(exp[0]->typeEquals("IDENFR")) {
        paramType = analyseLVal(exp); // LVal
        Word* ident = exp[0];
        if(paramType == 0) {
            // if(codeList.size() == 82) {
            //     cout << "debugger" << endl;
            // }
            codeList.push_back(
                new PCode(
                    CodeType::VALUE, 
                    new Packing<string>(to_string(getSymbol(ident)->getFieldId()) + "_" + ident->getContent()),
                    new Packing<int>(paramType)
                )
            );
        }
        else {
            // if(codeList.size() == 82) {
            //     cout << "debugger" << endl;
            // }
            codeList.push_back(
                new PCode(
                    CodeType::ADDRESS,
                    new Packing<string>(to_string(getSymbol(ident)->getFieldId()) + "_" + ident->getContent()),
                    new Packing<int>(paramType)
                )
            );
        }
    }
    else if(exp[0]->typeEquals("INTCON")) analyseNumber(exp[0]); // Number
    else error();
    grammar.emplace_back(outString("<PrimaryExp>"));
    return paramType;
}

// ConstExp → AddExp
void Parser::analyseConstExp(vector<Word*>& exp) {
    analyseAddExp(exp);
    grammar.emplace_back(outString("<ConstExp>"));
}

/**
 * AddExp → MulExp | AddExp ('+' | '−') MulExp
 * left recursion must be avoided
 * Rewrite it like: AddExp → MulExp ('+' | '−') MulExp  ('+' | '−') MulExp ...
 */
int Parser::analyseAddExp(vector<Word*>& exp) {
    vector<string> symbol = {"PLUS", "MINU"};
    Exps* exps = divideExp(exp, symbol);
    int i = 0, paramType = 0;
    for(auto e : exps->getWords()) {
        paramType = analyseMulExp(e);
        if(i) {
            CodeType tp;
            if(exps->getSymbols()[i - 1]->typeEquals("PLUS")) tp = CodeType::ADD;
            else tp = CodeType::SUB;
            codeList.push_back(new PCode(tp));
        }
        grammar.emplace_back(outString("<AddExp>"));
        if(i < exps->getSymbols().size()) {
            grammar.emplace_back(exps->getSymbols()[i]->outString());
            i += 1;
        }
    }
    return paramType;
}

/**
 * MulExp → UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
 * Rewrite it like: MulExp → UnaryExp ('*' | '/' | '%') UnaryExp  ('*' | '/' | '%') UnaryExp ...
 */
int Parser::analyseMulExp(vector<Word*>& exp) {
    vector<string> symbol = {"MULT", "DIV", "MOD"};
    Exps* exps = divideExp(exp, symbol);
    int i = 0, paramType = 0;
    for(auto e : exps->getWords()) {
        paramType = analyseUnaryExp(e);
        if(i) {
            CodeType tp;
            if(exps->getSymbols()[i - 1]->typeEquals("MULT")) tp = CodeType::MUL;
            else if(exps->getSymbols()[i - 1]->typeEquals("DIV")) tp = CodeType::DIV;
            else tp = CodeType::MOD;
            codeList.push_back(new PCode(tp));
        }
        grammar.emplace_back(outString("<MulExp>"));
        if(i < exps->getSymbols().size()) {
            grammar.emplace_back(exps->getSymbols()[i]->outString());
            i += 1;
        }
    }
    return paramType;
}

// UnaryOp → '+' | '−' | '!'
void Parser::analyseUnaryOp(Word* word) {
    grammar.emplace_back(word->outString());
    grammar.emplace_back(outString("<UnaryOp>"));
}

// UnaryExp → PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
int Parser::analyseUnaryExp(vector<Word*>& exp) {
    int paramType = 0;
    if(!exp.size()) return paramType;
    if(exp[0]->typeEquals("PLUS") || exp[0]->typeEquals("NOT") || exp[0]->typeEquals("MINU")) { // UnaryOp UnaryExp
        // remove UnaryOp
        analyseUnaryOp(exp[0]); // UnaryOp
        vector<Word*> unaryExp = splitVector(exp, 1, (int)exp.size());
        analyseUnaryExp(unaryExp);
        vector<Word*>().swap(unaryExp);
        CodeType tp;
        if(exp[0]->typeEquals("PLUS")) tp = CodeType::POS;
        else if(exp[0]->typeEquals("MINU")) tp = CodeType::NEG;
        else tp = CodeType::NOT;
        codeList.push_back(new PCode(tp));
    } 
    else if(exp.size() == 1) paramType = analysePrimaryExp(exp); // PrimaryExp
    else {
        if(exp[0]->typeEquals("IDENFR") && exp[1]->typeEquals("LPARENT")) {
            Word* ident = exp[0]; // Ident
            vector<int> params = vector<int>();
            if(undefinedFunction(ident)) error("c", ident->getLineNum());
            else params = getFunction(ident)->getParams();
            if(!exp.back()->typeEquals("RPARENT")) {
                error("j");
                exp.emplace_back(new Word(")", curWord->getLineNum()));
            }

            // remove Ident `(` `)`
            grammar.emplace_back(exp[0]->outString()); // ident
            grammar.emplace_back(exp[1]->outString()); // `(`
            if(exp.size() >= 4) {
                vector<Word*> splitExp = splitVector(exp, 2, (int)exp.size() - 1); // remove `(`, `)`
                analyseFuncRParams(ident, splitExp, params); // FuncRParams
                vector<Word*>().swap(splitExp);
            }
            else if(params.size()) error("d", ident->getLineNum()); 

            grammar.emplace_back(exp.back()->outString()); // `)`
            codeList.push_back(
                new PCode(CodeType::CALL, new Packing<string>(ident->getContent()))
            );
            if(!undefinedFunction(ident) && getFunction(ident)->getReturnType() == "void") paramType = -1; 
        }
        else paramType = analysePrimaryExp(exp); // PrimaryExp
    }
    grammar.emplace_back(outString("<UnaryExp>"));
    return paramType;
}

/**
 * @brief 
 * function divideExp is used for divide the whole exp passed by getExp or the pre functions
 * @param exp 
 * @param symbol 
 * @return Exps* 
 */
Exps* Parser::divideExp(vector<Word*>& exp, vector<string>& symbol) {
    vector<vector<Word*> > exps = vector<vector<Word*> >();
    vector<Word*> expEle = vector<Word*>();
    vector<Word*> symbols = vector<Word*>();
    set<string> ss;
    for(auto str : symbol) ss.insert(str);

    int flagUNARY = 0, flagPARENT = 0, flagBRACK = 0;
    for(auto word : exp) {
        if(word->typeEquals("LPARENT")) flagPARENT++;
        if(word->typeEquals("RPARENT")) flagPARENT--;
        if(word->typeEquals("LBRACK")) flagBRACK++;
        if(word->typeEquals("RBRACK")) flagBRACK--;
        if(ss.count(word->getType()) && flagPARENT == 0 && flagBRACK == 0) {
            // UnaryOp
            if(word->typeOfUnary()) {
                if(!flagUNARY) {
                    expEle.emplace_back(word);
                    continue;
                }
            }
            exps.emplace_back(expEle);
            symbols.emplace_back(word);
            expEle = vector<Word*>();
        } 
        else expEle.emplace_back(word);

        if(word->typeEquals("IDENFR") || word->typeEquals("RPARENT") || 
            word->typeEquals("INTCON") || word->typeEquals("RBRACK")) 
            flagUNARY = true;
        else flagUNARY = false;

        // flagUNARY = (
        //     word->typeEquals("IDENFR") || word->typeEquals("RPARENT") || 
        //     word->typeEquals("INTCON") || word->typeEquals("RBRACK")
        // );
    }
    if(expEle.size()) exps.emplace_back(expEle);
    return new Exps(exps, symbols);
}

void Parser::getExp(vector<Word*>& exp) {
    exp = vector<Word*>();
    int inFunc = 0, flagFUNC = 0, flagPARENT = 0, flagBRACK = 0;
    Word* word = getNextWord(), *preWord = nullptr;
    while(1) {
        if(word->typeEquals("SEMICN") || word->typeEquals("ASSIGN") || word->typeEquals("RBRACE") 
            || word->typeForValidateStmt() || (word->typeEquals("COMMA") && !inFunc) || word->typeForNotInExp()) break;

        if(preWord != nullptr) {
            if((word->typeEquals("INTCON") || word->typeEquals("IDENFR"))) {
                if(preWord->typeEquals("IDENFR") || preWord->typeEquals("INTCON") || preWord->typeEquals("RPARENT") || preWord->typeEquals("RBRACK"))
                    break;
            }
            if(!flagPARENT && !flagBRACK) {
                if(preWord->typeEquals("INTCON") && (word->typeEquals("LBRACK") || word->typeEquals("LBRACE"))) 
                    break;
            }
        }

        if(word->typeEquals("IDENFR") && getNext2Word()->typeEquals("LPARENT")) inFunc = 1;
        else if(word->typeEquals("LPARENT")) {
            flagPARENT += 1;
            if(inFunc) flagFUNC += 1;
        }
        else if(word->typeEquals("RPARENT")) {
            flagPARENT -= 1;
            if(inFunc) {
                flagFUNC -= 1;
                if(flagFUNC == 0) inFunc = 0;  
            }
        }
        else if(word->typeEquals("LBRACK")) flagBRACK += 1;
        else if(word->typeEquals("RBRACK")) flagBRACK -= 1;
        if(flagPARENT < 0 || flagBRACK < 0) break;
        curWord = words[idx++]; //getWordWithoutAddToGrammar();
        exp.emplace_back(curWord);
        preWord = word;
        word = getNextWord();
    }
}

void Parser::error() { /*TODO*/ }

void Parser::error(string type) {
    errors.emplace_back(new Error(curWord->getLineNum(), type));
    //cout << to_string(curWord->getLineNum()) << " " << type;
}

void Parser::error(string type, int lineNum) {
    errors.emplace_back(new Error(lineNum, type));
    //cout << to_string(lineNum) << " " << type;
}