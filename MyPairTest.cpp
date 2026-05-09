#include <iostream>
#include <string>
#include "MyPair.h"

int main() {
    MyPair<int, std::string> pair1(42, "Hello");
    MyPair<int, std::string> pair2(std::move(pair1));

    std::cout << "First: " << pair1.getFirst() << ", Second: " << pair1.getSecond() << std::endl;
    std::cout << "First: " << pair2.getFirst() << ", Second: " << pair2.getSecond() << std::endl;

    std::cout << "First (get<0>): " << pair2.get<0>() << ", Second (get<1>): " << pair2.get<1>() << std::endl;

    // 结构化绑定测试
    MyPair<int, std::string> p{42, "C++17 Rocks"};

    // 见证奇迹的时刻！
    auto [id, msg] = p; 

    std::cout << "ID: " << id << ", Msg: " << msg << std::endl;

    // 修改测试
    auto& [refId, refMsg] = p;
    refId = 100;
    std::cout << "Modified original ID: " << p.get<0>() << std::endl;
    return 0;
}