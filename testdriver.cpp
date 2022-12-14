#include "termformat.h"

#include <cstdio>
#include <iostream>
#include <sstream>

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
    
    std::cout << tfmt::format(tfmt::red, 0) << std::endl;
    std::cout << "Some other text\n";
    std::cout << tfmt::format(tfmt::magenta | tfmt::bgBlue | tfmt::underline | tfmt::italic | tfmt::bold, "Some text ", "and a number: ", 1236) << std::endl;
    
    tfmt::format(tfmt::bgBrightBlue, std::cout) << "This should be on blue bg. Here some more operands: " << 244 << " " << std::hex << 244 << " " << 11.5 << std::endl;
    
    tfmt::format(tfmt::red, std::cout) << std::endl;
    
    // Desired code:
    
    std::cout << "Some code here, this is " << tfmt::format(tfmt::italic,            "special") << " \n";
    std::cout << "                        " << tfmt::format(tfmt::red | tfmt::blink, "▔▔▔▔▔▔▔") << "\n";
    
    tfmt::format(tfmt::red, [&]{
        std::cout << "this is red and also ";
        tfmt::format(tfmt::underline, [&]{
            std::cout << "underlined";
            tfmt::format(tfmt::italic, [&]{
                std::cout << " and now italic\n";
            });
        });
    });
    
    std::cout << tfmt::bgBrightMagenta << "Hello World" << tfmt::reset << std::endl;
    
    tfmt::format(tfmt::bgBrightMagenta, [&]{
        std::cout << "on magenta bg\n";
    });
    
    {
        tfmt::FormatGuard fmt(tfmt::bgGrey | tfmt::brightWhite);
        std::cout << "on grey bg\n";
    }
    
    {
        tfmt::FormatGuard fmt(tfmt::bgBrightBlue | tfmt::brightWhite, std::cout);
        std::cout << "on blue bg\n";
    }
    
    
    
    std::cout << tfmt::bgBrightMagenta;
    
    std::cout << "A\nB\nC\n";
    
    std::cout << tfmt::reset;
    
    std::cout << "\n\n";
    
    std::stringstream sstr;
    tfmt::setFormattable(sstr);
    tfmt::pushModifier(tfmt::red, sstr);
    std::stringstream sstr2 = std::move(sstr);
    
    sstr2 << "This should be red ";
    tfmt::popModifier(sstr2);
    sstr2 << "but not this.\n";
    
    std::cout << sstr2.rdbuf();
    
}


