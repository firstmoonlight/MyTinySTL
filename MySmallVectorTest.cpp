#include <iostream>
#include "MySmallVector.h"
int main() {
    MySmallVector<int, 8> MVec;
    int Val = 1;
    MVec.push_back(Val);
    std::cout << MVec[0] << std::endl;
}



