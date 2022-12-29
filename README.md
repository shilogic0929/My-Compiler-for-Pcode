## 词法分析

### 需求

读取' testfile.txt '，将每个字符解析为单词并打印出来。同时，记住每个单词的类型、内容和行数。

#### 读文件

逐行读取，扫描每个字符串的每个字符并进行分析。

```c++
// lexer
        char *strbuf;
        int len = in.pubseekoff(0, ios::end, ios::in);
        in.pubseekpos(0, ios::in);
        strbuf = new char[len];
        in.sgetn(strbuf, len);
        string inStr(strbuf);

        //std::cerr << inStr;

        Lexer lexer(inStr);
        auto tokenLists = lexer.words;
```

#### 词法分析

当我得到关键字，进入下一个分析，类似有穷自动机。

```c++
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
```

##### 普通单词

例如，当我得到“+”时，我直接新建一个Word类型化“+”。

##### 函数

例如

当我得到'&lt;'时，输入函数' analyseRelation '再读一个字符。如果是' = '，分析' LEQ '…

```c++
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
```

`analyseLogic` 类似.

##### 数字

数字: 当我得到一个数字时，这意味着我将扫描一些数字的序列，并将它们转换为Word类型化“INTCON”。

字母: 当我得到一个字母时，这意味着我将扫描一个关于字母或数字的字符串。它可能是“IDENFR”或“STRCON”，这取决于它是否在关键字哈希表中。

#### Word类

```c++
class Word {
private:
    string identification;
    string content;
    string type;
    int lineNum;
```

封装初始函数，我只需要在主函数中' new Word(…)'，它将创建相应的单词。

```c++
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
```

至于关键字哈希表，它是一个map，映射单词的字符串及其类型。

```c++
static const map<string, string> keywords = {
    {"main", "MAINTK"},
    {"const", "CONSTTK"},
    {"int", "INTTK"},
    {"break", "BREAKTK"},
    {"continue", "CONTINUETK"},
    {"if", "IFTK"},...
```

### 后续工作

#### 读文件

逐行读取文件不方便预读和撤销，所以我首先将文件读入一个字符串inStr

```c++
Lexer lexer(inStr);
auto tokenLists = lexer.words;
```

#### 词法分析

逐个分析单词，所以我添加了全局变量' idx'来记住指针在哪里。

此外，我遇到了需要多读一次或者撤销的情况，所以我封装了' ungetChar '和' getChar '函数，方便我分析。

```c++
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
```

##### 注释

1. `//` : 单行注释：遇到 `\n` 停止.

```c++
if(c == '/') {
        do {
            c = getChar();
            if (c == ED || c == '\n') {
                return;
                // 判断为//注释，结束分析
            }
        } while(1);
    } 
```

2. `/* */`: 多行注释 ，遇到 `*/` 停止

```c++
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
```

## 语法分析

要求:根据词法分析程序识别出的单词，根据语法规则识别出各种语法元素。采用递归下降法对语法中定义的语法成分进行分析。

### 分析

#### 读文件

像词法分析器一样，我准备了函数“getWord”、“getNextWord”等。同时，有一个全局指针变量' (Word) curWord '来显示当我读取' vector&lt;Word&gt;中的单词。

正常的规则是:我不断地得到单词并分析它们

表达式规则:我首先扫描整个表达式，这是由函数' getExp '实现的。**然后对表达式进行分解，采用递归下降法对其进行分析。

`getExp` 如下

```c++
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
```

#### 递归下降

根据语法规则，规则的每一项都有编码功能。

大致思路:读一个单词，检查它的符号，然后进入下一个分析函数。

**例如:**

```c
CompUnit → {Decl} {FuncDef} MainFuncDef // 1.是否存在Decl 2.是否存在 FuncDef
```

```c++
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
```

语法是用来记忆输出的词汇分析和语法分析列表。

#### 消除左递归

```java
加减表达式 AddExp → MulExp | AddExp ('+' | '−') MulExp // 1.MulExp 2.+ 需覆盖 3.- 需覆盖
```

检查exp是否有'+'或'-'。如果有，分离exp到addxp和MulExp。然后分别分析它们。

例如：

```c++
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
```

### 后续工作

函数 `splitVector` 和 ` divideExp`用来分割  ` getExp`以及之前的函数传过来的表达式

`splitVector`:

```c++
inline vector<Word*> splitVector(vector<Word*>& exp, int l, int r) {
    vector<Word*> spiltExp = vector<Word*>();
    for(int i = l; i < r; i++) spiltExp.emplace_back(exp[i]);
    return spiltExp;
}
```

`divideExp`:

```c++
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
```

`Exps`

```c++
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
```

## 错误处理

### 分析

#### 建立符号表

```c++
// symbol table item
class Symbol {
private:
    string type;
    int paramType; // paramType: 0:int, 1:int[], 2:int[][], ...
    string content;
    int fieldId = 0;

public:
    Symbol(string type, int paramType, Word* word, int fieldId) {
        this->type = type;
        this->paramType = paramType;
        this->content = word->getContent();
        this->fieldId = fieldId;
    }

public:
    string getType() { return this->type; }

    string getContent() { return this->content; }

    int getParamType() { return this->paramType; }

    int getFieldId() { return this->fieldId; }

    string outString() { return this->content; }

};
```

```c++
class SymbolTable {
private: 
    map<string, Symbol*> symbolTable;

public:
    SymbolTable() {
        symbolTable = map<string, Symbol*>();
    }

    bool hasSymbol(Word* word) {
        if(symbolTable.find(word->getContent()) != symbolTable.end()) return true;
        return false; 
    }

    void addSymbol(string type, int intType, Word* word, int fieldId) {
        symbolTable[word->getContent()] = new Symbol(type, intType, word, fieldId);
    }

    bool isConst(Word* word) {
        if(symbolTable.find(word->getContent()) == symbolTable.end()) return false;
        string t = symbolTable[word->getContent()]->getType();
        if(t == "const") return true;
        return false;
    }

    Symbol* getSymbol(Word* word) {
        return symbolTable[word->getContent()];
    }

    string outString() {
        if(!symbolTable.size()) return "{}";
        string res = "";
        int i = 1;
        for(auto it : symbolTable) {
            res += "{key"; res += to_string(i); res += ":"; res += it.first;
            res += ", ";
            res += "value"; res += to_string(i); res += ":"; res += it.second->getContent(); res += "}, ";
            i++;
        }
        return res;
    }
};
```

Type表示符号的类型。IntType是一个整数。**如果是0，符号就是int。**如果是1，则符号为int[]，如果是2，则符号为int[][]…

content 就是它的内容。field就是它的作用域。我创建了一个符号Map，记住在每个区域创建的符号。当进入一个新的域，fieldId++。离开一个域，fieldId--，与之对应的符号都被销毁了。

```c++
map<int, SymbolTable*>().swap(symbols);
map<string, Function*>().swap(functions);
```

`needReturn` : 当前函数有返回值

`whileFlag ` 当前代码块在while循环体中

#### 错误处理

##### **a**

检查格式

```c++
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
```

##### **b c**

B:每次我得到一个标识，检查是否有相同的符号被定义在这个区域。

```java
bool Parser::repeatedSymbolInCurrentField(Word* word) {
    // if(field == -1) return false;
    if(symbols[field]->hasSymbol(word)) return true;
    return false;
}
```

C:检查所有区域。**如果符号已定义。函数是一样的。

```java
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
```

##### **d e**

为了检查函数参数是否匹配，我记住了每个函数的参数，当我遇到函数调用时，我会扫描函数调用的参数并进行匹配。我准备了一个函数来做这个。最后，我发现我需要再次使用递归下降，所以我在语法分析器的递归下降中添加了检查过程。

##### f g

有一个全局变量' needReturn '用于显示当前函数是否需要返回。如果它有，但在代码块的末尾没有返回，或者如果它没有，但有返回，错误将被记住。

##### **h**

检查它是否为const。

```c++
if (isConst(word)) {
  error("h", word->getLineNum());
}
```

##### **i j k**

封装功能，检查缺失的符号

**例如:**

```c++
void Parser::checkRightParent() {
    Word* word = getNextWord();
    if(!word->typeEquals("RPARENT")) error("j");
    else getWord(); // ')'
}
```

##### **l**

分别计算string和printf的参数数量，并检查它们是否相等。

##### **m**

如果代码块在while循环中，则有一个全局的' whileFlag '符号。如果不是，任何continue和break都会产生错误。

### 后续工作

#### 作用域

当我得到一个块或者一个函数的时候，我标记了字段fieldId++，但是这会导致在输入一个函数的代码块的时候，函数的参数不能被记忆在与函数块不同的地方。所以我把规则改为只有当块不是函数内部块时，fieldId++。

```c++
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
```

#### 错误d和e

为了检查函数参数是否匹配，我为每个函数设置了一个数组。

```c++
class Function {
private: 
    string type;
    string content;
    string returnType;
    vector<int> params; // void: -1, int: 0, int[]: 1, int[][]: 2
```

当我得到一个函数时，我记住它的返回类型和参数。

对于 `vector<int> params`, 对应如下:

| 类型     | 元素    | 值 |
| -------- | ------- | -- |
| Void     |         | -1 |
| Int      | a       | 0  |
| Int[]    | a[]     | 1  |
| Int[] [] | a[] [3] | 2  |

所以当我得到一个函数调用时，我会用我之前保存下来的来检查它的参数

```c++
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
```

为了获得实类型参数，我在语法分析器的递归下降过程中添加了分析程序。就像:

```c++
int Parser::analyseExp(vector<Word*>& exp) {
    int paramType = analyseAddExp(exp);
    grammar.emplace_back(outString("<Exp>"));
    return paramType;
}
```

每次递归都会返回一个' intType '，它表示表达式的最终类型。因为一个表达式的项必须是同一类型，所以我只返回其中的一个。这是递归的出口。**它将返回函数顶部表达式的正确类型。

```c++
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
```

## 代码生成

在这一部分中，我选择了生成Pcode。设计了一种基于逆波兰表达式栈和符号表的虚拟代码。

同时，我设计了虚拟机来执行它们。

Pcode虚拟机是用来运行Pcode命令的虚拟机。**它由代码区(code)、指令指针(EIP)、堆栈、var_table、func_table和label_table组成。

在接下来的文章中，我将首先介绍Pcode如何执行，然后介绍如何生成Pcode。

### 分析

#### 虚拟机

首先，我们需要一个“codes”列表和一个“stack”(int)。“eip”:表示当前运行代码的地址。

' varTable ':存储堆栈中变量的地址。

' funcTable ':在代码列表中存储函数的地址。

' labelTable ':在代码列表中记忆标签的地址。

然后，依次运行代码并管理堆栈。

#### 如何区分不同变量

在生成代码之前，通过不同作用域的唯一作用域号来区分不同作用域的品种，例如:' areaID + "_" + curWord.getContent() '。在这种情况下，变量不会在代码中出现超过一次，除了递归函数调用，这将通过将' varTable '推入堆栈来解决(稍后显示)。

#### 特定代码定义

PCode类:

```c++
class PCode {
private:
    // contains a CodeType and two operating values
    // CodeType is an enum, value1 and value2 may be integer or string or null, which depend on specific code type
    // Caculation Type contains binocular and singular operators:
    // Binocular operator: int b = stk_pop(), a = stk_pop(), c = cal(a, b), stk_push(c);
    // Singular operator: int a = stk.pop(), b = cal(a), stk.push(b);
    CodeType type;
    Object* value1 = nullptr;
    Object* value2 = nullptr;

```

它表示一个代码对象，该对象具有CodeType和两个操作值。**CodeType是一个枚举。**Value1和value2可能是Integer或String或null，这取决于特定的代码类型。**

#### 计算类型

**双目运算**

```c++
int b = pop();
int a = pop();
push(cal(a,b));
```

单目运算

```java
push(cal(pop()));
```

##### VAR

**VAR**命令声明一个变量，将变量名和分配给它的地址保存在变量表中。

```c++
case CodeType::VAR : {
                Var* var = new Var(stk.size());
                auto string_ptr = code->getValue1()->to_son_class<string>();
                if(string_ptr != nullptr) {
                    string key = string_ptr->c_str();
                    varTable[key] = var;
                }
            } break;
```

Var类:

```c++
class Var {
private:
    int idx, dimension = 0, dim1 = 0, dim2 = 0;

public:
    Var(int idx) { this->idx = idx; }

    int getIdx() { return this->idx; }

    int getDim1() { return this->dim1; }

    int getDim2() { return this->dim2; }

    void setDim1(int dim1) { this->dim1 = dim1; }

    void setDim2(int dim2) { this->dim2 = dim2; }

    int getDimension() { return dimension; }

    void setDimension(int dimension) { this->dimension = dimension; }
  
};
```

##### DIMVAR

**DIMVAR**命令声明一个数组。设置变量的尺寸信息。

```c++
case CodeType::DIMVAR : {
                auto string_ptr = code->getValue1()->to_son_class<string>();
                auto int_ptr = code->getValue2()->to_son_class<int>();
                if(string_ptr != nullptr && int_ptr != nullptr) {
                    string ident = string_ptr->c_str();
                    Var* var = getVar(ident);
                    int n = *int_ptr;
                    var->setDimension(n);
                    if(n == 1) {
                        int a = stk_pop();
                        var->setDim1(a);
                    }
                    else if(n == 2) {
                        int b = stk_pop(), a = stk_pop();
                        var->setDim1(a);
                        var->setDim2(b);
                    }
                }
                else {
                    cout << "DIMVAR Error" << endl;
                    break;
                }
            } break;

```

##### PLACEHOLDER

**PLACEHOLDER**命令将堆栈向下扩展，将新空间分配给品种和数组。

```c++
case CodeType::PLACEHOLDER : {
                auto string_ptr = code->getValue1()->to_son_class<string>();
                auto int_ptr = code->getValue2()->to_son_class<int>();
                if(string_ptr != nullptr && int_ptr != nullptr) {
                    string ident = string_ptr->c_str();
                    Var* var = getVar(ident);
                    int n = *int_ptr;
                    if(n == 0) stk_push(0);
                    else if(n == 1) {
                        for(int i = 0; i < var->getDim1(); i++) 
                            stk_push(0);
                    }
                    else if(n == 2) {
                        for(int i = 0; i < var->getDim1() * var->getDim2(); i++) 
                            stk_push(0);
                    }
                }
                else {
                    cout << "PLACEHOLDER Error" << endl;
                    break;
                }
            } break;
```

##### 其他

计算类型:弹出堆栈顶部一次或两次，计算它们并再次推入。

跳转类型:当它是关于跳转的命令时，只需检查条件是否满足并更改' eip '。

**函数调用:如下**

#### 函数调用操作

首先，在函数调用之前，会有一些参数被推入堆栈。**每个变量后面都有一个“RPARA”命令，用来记住前一个变量的地址。

```c++
case CodeType::PARA : {
                Var* param = new Var(realParams[realParams.size() - callArgsNum + curArsgNum]);
                auto int_ptr = code->getValue2()->to_son_class<int>();
                auto string_ptr = code->getValue1()->to_son_class<string>();
                if(int_ptr != nullptr && string_ptr != nullptr) {
                    int n = *int_ptr;
                    param->setDimension(n);
                    if(n == 2) param->setDim2(stk_pop());
                    string key = string_ptr->c_str();
                    varTable[key] = param;
                    curArsgNum += 1;
                    if(curArsgNum == callArgsNum) realParams.erase(realParams.end() - callArgsNum, realParams.end());
                }
                else {
                    cout << "PARA Error" << endl;
                }
            } break;
```

第二，函数' CALL '。记住eip、堆栈顶部地址和关于函数的信息(事实上，它们也将被推入堆栈)。**然后更新' varTable '和' eip '。准备执行函数。

```c++
case CodeType::CALL : {
                auto string_ptr = code->getValue1()->to_son_class<string>();
                if(string_ptr != nullptr) {
                    string key = string_ptr->c_str();
                    Func* func = funcTable[key];
                    returnInfoList.push_back(new ReturnInfo(eip, stk.size() - 1, func->getArgs(), func->getArgs(), curArsgNum, varTable));
                    eip = func->getIdx();
                    varTable.clear();
                    callArgsNum = func->getArgs();
                    curArsgNum = 0;
                }
                else {
                    cout << "CALL Error" << endl;
                    break;
                }
            } break;
```

最后，当它是' RET '时返回。从' RetInfo '中恢复' eip '， ' varTable '，清除函数在堆栈中推入的新信息。

```c++
case CodeType::RET : {
                auto int_ptr = code->getValue1()->to_son_class<int>();
                if(int_ptr != nullptr) {
                    int n = *int_ptr;
                    ReturnInfo* info = returnInfoList.back();
                    returnInfoList.pop_back();
                    eip = info->getEip();
                    varTable = info->getVarTable();
                    callArgsNum = info->getCallArgsNum();
                    curArsgNum = info->getCurArgsNum();
                    auto left = stk.begin() + (info->getStackPtr() + 1 - info->getParamNum());
                    if(n == 1) stk.erase(left, stk.end() - 1);
                    else stk.erase(left, stk.end());
                }
                else {
                    cout << "RET Error" << endl;
                    break;
                }
            } break;
```

#### 变量值和地址

推送值或地址的种类很重要，这取决于我需要什么，我会在描述如何生成代码时给出。

命令操作如下(' getAddress '用于获取前一个类型的地址)。

```c++
case CodeType::VALUE : {
                auto string_ptr = code->getValue1()->to_son_class<string>();
                auto int_ptr = code->getValue2()->to_son_class<int>();
                if(string_ptr != nullptr && int_ptr != nullptr) {
                    string ident = string_ptr->c_str();
                    Var* var = getVar(ident);
                    int n = *int_ptr;
                    int addr = getAddr(var, n);
                    stk_push(stk[addr]);
                }
                else {
                    cout << "VALUE Error" << endl;
                    break;
                }
            } break;

            case CodeType::ADDRESS : {
                auto string_ptr = code->getValue1()->to_son_class<string>();
                auto int_ptr = code->getValue2()->to_son_class<int>();
                if(string_ptr != nullptr && int_ptr != nullptr) {
                    string ident = string_ptr->c_str();
                    Var* var = getVar(ident);
                    int n = *int_ptr;
                    int addr = getAddr(var, n);
                    stk_push(addr);
                }
                else {
                    cout << "ADDRESS Error" << endl;
                    break;
                }
            } break;
```

#### 代码生成

从语法分析程序生成的代码。

##### Declaration

There is no need to distinguish const and var. When declare a variety, just new a variety and let it point to the stack top. Then if it has an initialization, just push the values one after another. If not, add a `PLACEHOLDER` command to push something(I push 0) to the stack to hold the place.

##### Assign sentence

In this situation, first calculate and push the address of the variety to the stack top. Then analyze expressions. After that, there are only two number in the stack, which are address and value. Assign the value to the address.

##### Condition control sentence

First, generate labels. Then, place jump sentences in the proper places.

labels about if and while will be generated and then stored in a stack type structure. like:

```java
whileLabels.add(new HashMap<>());
whileLabels.get(whileLabels.size() - 1).put("while", labelGenerator.getLabel("while"));
whileLabels.get(whileLabels.size() - 1).put("while_end", labelGenerator.getLabel("while_end"));
whileLabels.get(whileLabels.size() - 1).put("while_block", labelGenerator.getLabel("while_block"));
```

Take if as example:

```java
if (word.typeEquals("IFTK")) {
    codes.add(new PCode(CodeType.LABEL, ifLabels.get(ifLabels.size() - 1).get("if")));
    ...
    analyseCond("IFTK");
    ...
    codes.add(new PCode(CodeType.JZ, ifLabels.get(ifLabels.size() - 1).get("else")));
    codes.add(new PCode(CodeType.LABEL, ifLabels.get(ifLabels.size() - 1).get("if_block")));
    analyseStmt();
    codes.add(new PCode(CodeType.JMP, ifLabels.get(ifLabels.size() - 1).get("if_end")));
    codes.add(new PCode(CodeType.LABEL, ifLabels.get(ifLabels.size() - 1).get("else")));
    if (word.typeEquals("ELSETK")) {
        getWord(); //else
        analyseStmt();
    }
    codes.add(new PCode(CodeType.LABEL, ifLabels.get(ifLabels.size() - 1).get("if_end")));
}
```

while:

```java
if (word.typeEquals("WHILETK")) {
    ...
    codes.add(new PCode(CodeType.LABEL, whileLabels.get(whileLabels.size() - 1).get("while")));
    ...
    analyseCond("WHILETK");
    ...
    codes.add(new PCode(CodeType.JZ, whileLabels.get(whileLabels.size() - 1).get("while_end")));
    codes.add(new PCode(CodeType.LABEL, whileLabels.get(whileLabels.size() - 1).get("while_block")));
    analyseStmt();
    ...
    codes.add(new PCode(CodeType.JMP, whileLabels.get(whileLabels.size() - 1).get("while")));
    codes.add(new PCode(CodeType.LABEL, whileLabels.get(whileLabels.size() - 1).get("while_end")));
    whileLabels.remove(whileLabels.size() - 1);
}

// break
if (word.typeEquals("BREAKTK")) {
    getWord();//break
    codes.add(new PCode(CodeType.JMP, whileLabels.get(whileLabels.size() - 1).get("while_end")));
 		...
}

// continue
if (word.typeEquals("CONTINUETK")) {
    getWord();//continue
    codes.add(new PCode(CodeType.JMP, whileLabels.get(whileLabels.size() - 1).get("while")));
    ...
} 
```

### 后续工作

由于一些运行时错误和信息短缺，我添加和删除了一些Pcode。**同时，在地址传递和短路计算方面也出现了一些新的问题。

#### 特定代码定义

在Operation中，' push() '意味着将值放入堆栈顶部。' pop() '意味着从堆栈顶部弹出值。

**Pcode类型**

| Pcode类型   | Value1              | Value2            | 操作                                    |
| ----------- | ------------------- | ----------------- | --------------------------------------- |
| LABEL       | Label_name          | Set a label       |                                         |
| VAR         | Ident_name          | Declare a variety |                                         |
| PUSH        | Ident_name/Digit    | push(value1)      |                                         |
| POP         | Address             | Ident_name        | *value1 = value2                        |
| JZ          | Label_name          |                   | Jump if stack top is zero               |
| JNZ         | Label_name          |                   | Jump if stack top is not zero           |
| JMP         | Label_name          |                   | Jump unconditionally                    |
| MAIN        |                     |                   | Main function label                     |
| FUNC        |                     |                   | Function label                          |
| ENDFUNC     |                     |                   | End of function label                   |
| PARA        | Ident_name          | Type              | Parameters                              |
| RET         | Return value or not |                   | Function return                         |
| CALL        | Function name       |                   | Function call                           |
| RPARA       | Type                |                   | Get parameters ready for function call  |
| GETINT      |                     |                   | Get a integer and put it into stack top |
| PRINT       | String              | Para num          | Pop values and print.                   |
| DIMVAR      | Ident_name          | Type              | Set dimension info for array variety    |
| VALUE       | Ident_name          | Type              | Get the variety value                   |
| ADDRESS     | Ident_name          | Type              | Get the variety address                 |
| PLACEHOLDER |                     |                   | Push something to hold places           |
| EXIT        |                     |                   | Exit                                    |

| Pcode类型 | Value1 | Value2 | 操作 |
| --------- | ------ | ------ | ---- |
| ADD       |        |        | +    |
| SUB       |        |        | -    |
| MUL       |        |        | *    |
| DIV       |        |        | /    |
| MOD       |        |        | %    |
| CMPEQ     |        |        | ==   |
| CMPNE     |        |        | !=   |
| CMPGT     |        |        | >    |
| CMPLT     |        |        | <    |
| CMPGE     |        |        | >=   |
| CMPLE     |        |        | <=   |
| AND       |        |        | &&   |
| OR        |        |        |      |
| NOT       |        |        | !    |
| NEG       |        |        | -    |
| POS       |        |        | +    |

#### 短路原则

有两种情况我需要使用短路计算:

```c
1. if(a&&b) // a is false
2. if(a||b) // b is true
```

首先，当我分析' analyseLOrExp '时，每个' analyselandxp '后面都会跟随一个' JNZ '，这用于检测cond是否为假。如果是，则跳转到If主体标签。同时，我生成了cond标签，为“analyselandxp”做好了准备。

```c++
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
```

在' analyselanexp '中，每个' analyseEqExp '后面都将跟随一个' JZ '，用于检测cond是否为真。如果是，跳转到我刚才设置的cond标签。

```c++
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
```
