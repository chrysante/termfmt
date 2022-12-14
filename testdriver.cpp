#include "termformat.h"

#include <cstdio>
#include <iostream>

[[maybe_unused]]
static void printTestColors() {
    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 10; j++) {
            int n = 10 * i + j;
            if (n > 108) break;
            std::printf("\033[%dm %3d\033[00m", n, n);
        }
        std::printf("\n");
    }
}

int main() {
    
    std::wcout << tfmt::format(tfmt::red, 0) << std::endl;
    std::wcout << "Some other text\n";
    std::wcout << tfmt::format(tfmt::magenta | tfmt::bgBlue | tfmt::underline | tfmt::italic | tfmt::bold, "Some text ", "and a number: ", 1236) << std::endl;
    
    tfmt::format(tfmt::bgBrightBlue, std::wcout) << "This should be on blue bg. Here some more operands: " << 244 << " " << std::hex << 244 << " " << 11.5 << std::endl;
    
    tfmt::format(tfmt::red, std::wcout) << std::endl;
    
    // Desired code:
    
    std::cout << "Some code here, this is " << tfmt::format(tfmt::italic,            "special") << " \n";
    std::cout << "                        " << tfmt::format(tfmt::red | tfmt::blink, "▔▔▔▔▔▔▔") << "\n";
    
    tfmt::format(tfmt::red, std::wcout, [&]{
        std::wcout << "this is red and also ";
        tfmt::format(tfmt::underline, std::wcout, [&]{
            std::wcout << "underlined";
            tfmt::format(tfmt::italic, std::wcout, [&]{
                std::wcout << " and now italic\n";
            });
        });
    });
    
    std::wcout << tfmt::bgMagenta << "Hello World" << tfmt::reset << std::endl;
    
    
    tfmt::format(tfmt::bgMagenta, std::wcout, [&]{
        std::wcout << "on magenta bg\n";
    });
    
    
    std::cout << tfmt::bgMagenta;
    
    std::cout << "A\nB\nC\n";
    
    
    std::cout << tfmt::reset;
    
    std::cout << "\n\n";
}


