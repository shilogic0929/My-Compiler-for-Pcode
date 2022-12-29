#ifndef CG_VARIABLE
#define CG_VARIABLE

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

#endif