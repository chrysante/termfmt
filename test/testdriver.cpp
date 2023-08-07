#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string_view>

#include "termfmt/termfmt.h"

static void separator(int width) {
    for (int i = 0; i < width; ++i) {
        std::cout.put('=');
    }
    std::cout.put('\n');
}

static void header(std::string_view text) {
    int const left = 10;
    int const total = 60;
    std::cout.put('\n');
    separator(total);
    for (int i = 0; i < 10; ++i) {
        std::cout.put('=');
    }
    std::cout << std::setw(total - left) << std::setfill('=') << std::left << text << std::endl;
    separator(total);
}

static void testRaw() {
    header(" Raw ");
    std::cout << tfmt::Red << "This should be red.\n";
    std::cout << tfmt::BGBlue << "This should be red on blue background.\n" << tfmt::Reset;
}

static void testFormatGuard() {
    header(" FormatGuard ");
    tfmt::FormatGuard underline(tfmt::Underline);
    std::cout << "This entire section should be underlined.\n";
    {
        tfmt::FormatGuard italic(tfmt::Italic);
        std::cout << "This line should also be italic.\n";
        tfmt::FormatGuard cyanBold(tfmt::Cyan | tfmt::Bold);
        std::cout << "This line should also be bold and cyan.\n";
    }
    std::cout << "This should be default underlined again.\n";
}

static void testFlagAssociation() {
    std::stringstream a;
    tfmt::setTermFormattable(a);
    std::stringstream b = std::move(a);
    assert(!tfmt::isTermFormattable(a));
    assert(tfmt::isTermFormattable(b));
}

static void testStackAssociation() {
    header(" Stack association ");
    std::stringstream a;
    tfmt::setTermFormattable(a, tfmt::isTerminal(std::cout));
    tfmt::pushModifier(tfmt::Red, a);
    a << "This should be red.\n";
    std::stringstream b = std::move(a);
    b << "This should still be red.\n";
    tfmt::popModifier(b);
    b << "This should be reset.\n";
    std::cout << b.rdbuf();
}

static void testFormatCallback() {
    header(" Format with callback ");
    tfmt::format(tfmt::Red, [&]{
        std::cout << "This should be red and also ";
        tfmt::format(tfmt::Underline, [&]{
            std::cout << "underlined";
            tfmt::format(tfmt::Italic, [&]{
                std::cout << " and now also italic.\n";
            });
        });
    });
}

int main() {
    testRaw();
    testFormatGuard();
    testFlagAssociation();
    testStackAssociation();
    testFormatCallback();
}
