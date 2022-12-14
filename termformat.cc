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

extern Mod const tfmt::bold            { "\033[1m"   };
extern Mod const tfmt::italic          { "\033[3m"   };
extern Mod const tfmt::underline       { "\033[4m"   };
extern Mod const tfmt::blink           { "\033[5m"   };
extern Mod const tfmt::concealed       { "\033[8m"   };
extern Mod const tfmt::crossed         { "\033[9m"   };

extern Mod const tfmt::grey            { "\033[30m"  };
extern Mod const tfmt::red             { "\033[31m"  };
extern Mod const tfmt::green           { "\033[32m"  };
extern Mod const tfmt::yellow          { "\033[33m"  };
extern Mod const tfmt::blue            { "\033[34m"  };
extern Mod const tfmt::magenta         { "\033[35m"  };
extern Mod const tfmt::cyan            { "\033[36m"  };
extern Mod const tfmt::white           { "\033[37m"  };
extern Mod const tfmt::brightGrey      { "\033[90m"  };
extern Mod const tfmt::brightRed       { "\033[91m"  };
extern Mod const tfmt::brightGreen     { "\033[92m"  };
extern Mod const tfmt::brightYellow    { "\033[93m"  };
extern Mod const tfmt::brightBlue      { "\033[94m"  };
extern Mod const tfmt::brightMagenta   { "\033[95m"  };
extern Mod const tfmt::brightCyan      { "\033[96m"  };
extern Mod const tfmt::brightWhite     { "\033[97m"  };
extern Mod const tfmt::bgGrey          { "\033[40m"  };
extern Mod const tfmt::bgRed           { "\033[41m"  };
extern Mod const tfmt::bgGreen         { "\033[42m"  };
extern Mod const tfmt::bgYellow        { "\033[43m"  };
extern Mod const tfmt::bgBlue          { "\033[44m"  };
extern Mod const tfmt::bgMagenta       { "\033[45m"  };
extern Mod const tfmt::bgCyan          { "\033[46m"  };
extern Mod const tfmt::bgWhite         { "\033[47m"  };
extern Mod const tfmt::bgBrightGrey    { "\033[100m" };
extern Mod const tfmt::bgBrightRed     { "\033[101m" };
extern Mod const tfmt::bgBrightGreen   { "\033[102m" };
extern Mod const tfmt::bgBrightYellow  { "\033[103m" };
extern Mod const tfmt::bgBrightBlue    { "\033[104m" };
extern Mod const tfmt::bgBrightMagenta { "\033[105m" };
extern Mod const tfmt::bgBrightCyan    { "\033[106m" };
extern Mod const tfmt::bgBrightWhite   { "\033[107m" };

namespace {

class ModStack {
public:
    void push(Mod mod) {
        mods.push_back(mod);
    }
    
    void pop() {
        mods.pop_back();
    }
    
    template <typename CharT>
    void apply(std::basic_ostream<CharT>& ostream) const {
        ostream << reset;
        for (Mod mod: mods) {
            ostream << mod;
        }
    }
    
private:
    std::vector<Mod> mods;
};

} // namespace

static std::unordered_map<std::ios_base*, ModStack> globalModStacks;

template <typename CharT>
void internal::pushMod(std::basic_ostream<CharT>& ostream, Mod const& mod) {
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
