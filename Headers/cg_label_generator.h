#ifndef CG_LABEL_GENERATOR 
#define CG_LABEL_GENERATOR

#include <string>
using namespace std;

class LabelGenerator {
private:
    int cnt = 0;

public:
    string getLabel(string type) {
        this->cnt += 1;
        string res = "label_";
        res += type;
        res += "_";
        res += to_string(cnt);
        return res;
    }
};

#endif