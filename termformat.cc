#include "termformat.h"

#include <iostream>
#include <unordered_map>
#include <vector>

using namespace tfmt;

template<>
bool tfmt::isTerminal(std::ostream const& ostream) {
    return &ostream == &std::cout && getenv("TERM");
}

template <>
bool tfmt::isTerminal(std::wostream const& ostream) {
    return &ostream == &std::wcout && getenv("TERM");
}

static auto tcGetIndex() {
    static auto const index = std::ios_base::xalloc();
    return index;
}

template <typename CharT>
void tfmt::setFormattable(std::basic_ostream<CharT>& ostream, bool value) {
    static auto const index = tcGetIndex();
    ostream.iword(index) = value;
}

template void tfmt::setFormattable(std::ostream&, bool);
template void tfmt::setFormattable(std::wostream&, bool);

template <typename CharT>
bool tfmt::isFormattable(std::basic_ostream<CharT> const& ostream) {
    static auto const index = tcGetIndex();
    // We need to cast away constness to access .iword() method on std::ostream, as it does not provide a const overload.
    // However we take the ostream argument by const& as conceptually this method is const.
    std::basic_ostream<CharT>& mutableOstream = const_cast<std::basic_ostream<CharT>&>(ostream);
    return static_cast<bool>(mutableOstream.iword(index)) || isTerminal(ostream);
}

extern internal::ModBase const tfmt::reset{ "\033[00m"  };

extern Modifier const tfmt::bold            { "\033[1m"   };
extern Modifier const tfmt::italic          { "\033[3m"   };
extern Modifier const tfmt::underline       { "\033[4m"   };
extern Modifier const tfmt::blink           { "\033[5m"   };
extern Modifier const tfmt::concealed       { "\033[8m"   };
extern Modifier const tfmt::crossed         { "\033[9m"   };

extern Modifier const tfmt::grey            { "\033[30m"  };
extern Modifier const tfmt::red             { "\033[31m"  };
extern Modifier const tfmt::green           { "\033[32m"  };
extern Modifier const tfmt::yellow          { "\033[33m"  };
extern Modifier const tfmt::blue            { "\033[34m"  };
extern Modifier const tfmt::magenta         { "\033[35m"  };
extern Modifier const tfmt::cyan            { "\033[36m"  };
extern Modifier const tfmt::white           { "\033[37m"  };
extern Modifier const tfmt::brightGrey      { "\033[90m"  };
extern Modifier const tfmt::brightRed       { "\033[91m"  };
extern Modifier const tfmt::brightGreen     { "\033[92m"  };
extern Modifier const tfmt::brightYellow    { "\033[93m"  };
extern Modifier const tfmt::brightBlue      { "\033[94m"  };
extern Modifier const tfmt::brightMagenta   { "\033[95m"  };
extern Modifier const tfmt::brightCyan      { "\033[96m"  };
extern Modifier const tfmt::brightWhite     { "\033[97m"  };
extern Modifier const tfmt::bgGrey          { "\033[40m"  };
extern Modifier const tfmt::bgRed           { "\033[41m"  };
extern Modifier const tfmt::bgGreen         { "\033[42m"  };
extern Modifier const tfmt::bgYellow        { "\033[43m"  };
extern Modifier const tfmt::bgBlue          { "\033[44m"  };
extern Modifier const tfmt::bgMagenta       { "\033[45m"  };
extern Modifier const tfmt::bgCyan          { "\033[46m"  };
extern Modifier const tfmt::bgWhite         { "\033[47m"  };
extern Modifier const tfmt::bgBrightGrey    { "\033[100m" };
extern Modifier const tfmt::bgBrightRed     { "\033[101m" };
extern Modifier const tfmt::bgBrightGreen   { "\033[102m" };
extern Modifier const tfmt::bgBrightYellow  { "\033[103m" };
extern Modifier const tfmt::bgBrightBlue    { "\033[104m" };
extern Modifier const tfmt::bgBrightMagenta { "\033[105m" };
extern Modifier const tfmt::bgBrightCyan    { "\033[106m" };
extern Modifier const tfmt::bgBrightWhite   { "\033[107m" };

namespace {

class ModStack {
public:
    void push(Modifier mod) {
        mods.push_back(mod);
    }
    
    void pop() {
        mods.pop_back();
    }
    
    template <typename CharT>
    void apply(std::basic_ostream<CharT>& ostream) const {
        ostream << reset;
        for (Modifier mod: mods) {
            ostream << mod;
        }
    }
    
private:
    std::vector<Modifier> mods;
};

} // namespace

static std::unordered_map<std::ios_base*, ModStack> globalModStacks;

template <typename CharT>
void internal::pushMod(std::basic_ostream<CharT>& ostream, Modifier const& mod) {
    auto& stack = globalModStacks[&ostream];
    stack.push(mod);
    stack.apply(ostream);
}

template <typename CharT>
void internal::popMod(std::basic_ostream<CharT>& ostream) {
    auto& stack = globalModStacks[&ostream];
    stack.pop();
    stack.apply(ostream);
}

template void internal::pushMod(std::ostream&, Mod const&);
template void internal::pushMod(std::wostream&, Mod const&);

template void internal::popMod(std::ostream&);
template void internal::popMod(std::wostream&);
