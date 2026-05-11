#include <iostream>
#include "MyTuple.h"

int main() {
    MyTuple<int, std::string, double> tuple1(42, "Hello", 3.14);
    MyTuple<int, std::string, double> tuple2(std::move(tuple1));
    std::cout << "First: " << get<0>(tuple1) << ", Second: " << get<1>(tuple1) << ", Third: " << get<2>(tuple1) << std::endl;
    std::cout << "First: " << get<0>(tuple2) << ", Second: " << get<1>(tuple2) << ", Third: " << get<2>(tuple2) << std::endl;

    std::tuple<int, std::string, double> stdTuple = std::make_tuple(42, "Hello", 3.14);
    std::tuple<int, std::string, double> stdTuple2(std::move(stdTuple));
    std::cout << "Standard Tuple - First: " << std::get<0>(stdTuple) << ", Second: " << std::get<1>(stdTuple) << ", Third: " << std::get<2>(stdTuple) << std::endl;
    std::cout << "Standard Tuple - First: " << std::get<0>(stdTuple2) << ", Second: " << std::get<1>(stdTuple2) << ", Third: " << std::get<2>(stdTuple2) << std::endl;

    // 结构化绑定测试
    MyTuple<int, std::string, double> t{42, "C++17 Rocks", 3.14};

    // 见证奇迹的时刻！
    auto [id, msg, pi] = t; 

    std::cout << "ID: " << id << ", Msg: " << msg << ", Pi: " << pi << std::endl;

    // 修改测试
    auto& [refId, refMsg, refPi] = t;
    refId = 100;
    refPi = 2.718;
    std::cout << "Modified original ID: " << get<0>(t) << ", Modified original Pi: " << get<2>(t) << std::endl;
    return 0;
}