#include <iostream>
#include "MySmallVector.h"
int main() {
    MySmallVector<int, 8> MVec(6, 42); // 测试带值构造
    MySmallVector<int, 8> MVec2 = MVec; // 测试复制构造
    MySmallVector<int, 8> MVec3 = std::move(MVec); // 测试移动构造
    for (int i = 0; i < 10; ++i) {
        MVec.push_back(i);
    }
    for (size_t i = 0; i < MVec.size(); ++i) {
        std::cout << MVec[i] << " ";
    }
    std::cout << std::endl;
    for (size_t i = 0; i < MVec2.size(); ++i) {
        std::cout << MVec2[i] << " ";
    }
    std::cout << std::endl;
    for (size_t i = 0; i < MVec3.size(); ++i) {
        std::cout << MVec3[i] << " ";
    }
    std::cout << std::endl;
}



