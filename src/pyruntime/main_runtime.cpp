//
// Created by matthew on 3/24/26.
//

#include <iostream>

extern "C" void __pymodule(); // Provided by the translated MLIR module

int main() {
    try {
        __pymodule();
        return 0;
    } catch (std::runtime_error& e) {
        std::cerr << "Traceback (most recent call last):" << std::endl;
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
