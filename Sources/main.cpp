#include <bits/stdc++.h>
#include "lexer.h"
#include "parser.h"
using namespace std;

static string sourcePath = "../Test/testfile.txt";
static string outputPath = "../Test/error.txt";
static string pcodePath = "../Test/pcoderesult.txt";

int main() {
    try {
        filebuf in, out, p_out;
        if(!in.open(sourcePath, ios::in)) {
            throw runtime_error("Fail to open testfile.txt!");
        }
        if(!out.open(outputPath, ios::out)) {
            throw runtime_error("Fail to open output.txt!");
        }
        if(!p_out.open(pcodePath, ios::out)) {
            throw runtime_error("Fail to open pcoderesult.txt!");
        }
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

        // parser
        Parser parser(tokenLists);
        auto grammarLists = parser.grammar;
        auto errorLists = parser.errors;

        // output
        ostream output(&out);
        
        // for(auto& token : tokenLists) {
        //     output << token->outString();
        // }

        // for(auto& word : grammarLists) {
        //     output << word;
        // }

        for(auto& err : errorLists) {
            output << err->outString();
        }

        // pcode
        auto codeList = parser.codeList;
        PCodeExecutor pcodeExecutor(codeList);
        auto printList = pcodeExecutor.printList;
        // pcoderesult
        ostream pcodeResult(&p_out);

        for(auto& res : printList) {
            pcodeResult << res;
            //cout << res;
        }
        
        // release
        for(auto& t : tokenLists){
            if(t) delete t;
        }
        for(auto& e : errorLists){
            if(e) delete e;
        }
        for(auto& cd : codeList) {
            if(cd) delete cd;
        }
        vector<Word*>().swap(tokenLists);
        vector<string>().swap(grammarLists);
        vector<PCode*>().swap(codeList);
        delete[] strbuf;

        output.clear();
        in.close();
        out.close();
    }
    catch(const std::exception& e) {
        std::cerr << e.what();
    }
    return 0;
}