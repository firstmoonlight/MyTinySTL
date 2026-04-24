#include <iostream>
int main() {
    std::cout << alignof(std::max_align_t) << std::endl;
    std::cout << alignof(int) << std::endl;
    std::cout << alignof(double) << std::endl;
}