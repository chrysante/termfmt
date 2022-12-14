#include "termformat.h"

#include <iostream>
#include <new>
#include <vector>

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
#   define TFMT_UNIX 1
#elif defined(_WIN32)
#   define TFMT_WINDOWS 1
#else
#   error Unknown platform
#endif

#if TFMT_UNIX
#   include <unistd.h>
#elif TFMT_UNIX
#   error No windows support yet
#endif

using namespace tfmt;

static int isattyWrapper(FILE* file) {
#if TFMT_UNIX
    return isatty(fileno(file));
#elif TFMT_WINDOWS
    return _isatty(_fileno(file));
#endif
}

static bool filedescIsTerminal(FILE* file) {
    bool const isATTY = isattyWrapper(file);
#if defined(__APPLE__) && defined(__MACH__)
    static bool const envTermDefined = std::getenv("TERM") != nullptr;
    return isATTY && envTermDefined;
#else
    return isATTY;
#endif
}

template <typename OStream>
static bool isTerminalImpl(OStream const& ostream, OStream const& globalOStream, FILE* file) {
    if (&ostream == &globalOStream) {
        return filedescIsTerminal(file);
    }
    return false;;
}

template<>
bool tfmt::isTerminal(std::ostream const& ostream) {
    return isTerminalImpl(ostream, std::cout, stdout) ||
           isTerminalImpl(ostream, std::cerr, stderr) ||
           isTerminalImpl(ostream, std::clog, stderr);
}

template <>
bool tfmt::isTerminal(std::wostream const& ostream) {
    return isTerminalImpl(ostream, std::wcout, stdout) ||
           isTerminalImpl(ostream, std::wcerr, stderr) ||
           isTerminalImpl(ostream, std::wclog, stderr);
}

static auto tcOStreamIndex() {
    static auto const index = std::ios_base::xalloc();
    return index;
}

template <typename CharT, typename Traits>
void tfmt::setFormattable(std::basic_ostream<CharT, Traits>& ostream, bool value) {
    static auto const index = tcOStreamIndex();
    ostream.iword(index) = value;
}

template void tfmt::setFormattable(std::ostream&, bool);
template void tfmt::setFormattable(std::wostream&, bool);

template <typename CharT, typename Traits>
bool tfmt::isFormattable(std::basic_ostream<CharT, Traits> const& ostream) {
    static auto const index = tcOStreamIndex();
    // We need to cast away constness to access .iword() method on std::ostream, as it does not provide a const overload.
    // However we take the ostream argument by const& as conceptually this query does not modify the object.
    std::basic_ostream<CharT, Traits>& mutableOstream = const_cast<std::basic_ostream<CharT, Traits>&>(ostream);
    return static_cast<bool>(mutableOstream.iword(index)) || isTerminal(ostream);
}

namespace {

class ModStack {
public:
    ModStack() noexcept = default;
    
    void push(Modifier&& mod) {
        mods.push_back(std::move(mod));
    }
    
    void pop() {
        mods.pop_back();
    }
    
    template <typename CharT, typename Traits>
    void apply(std::basic_ostream<CharT, Traits>& ostream) const {
        ostream << reset;
        for (Modifier mod: mods) {
            ostream << mod;
        }
    }
    
private:
    std::vector<Modifier> mods;
};

} // namespace

template <typename CharT, typename Traits>
void tfmt::pushModifier(Modifier mod, std::basic_ostream<CharT, Traits>& ostream) {
    static auto const index = tcOStreamIndex();
    auto* stackPtr = static_cast<ModStack*>(ostream.pword(index));
    if (stackPtr == nullptr) {
        stackPtr = ::new ModStack();
        ostream.pword(index) = stackPtr;
        ostream.register_callback([](std::ios_base::event event, std::ios_base& ios, int index) {
            if (event != std::ios_base::erase_event) {
                return;
            }
            auto* stackPtr = static_cast<ModStack*>(ios.pword(index));
            ::delete stackPtr;
            ios.pword(index) = nullptr;
        }, index);
    }
    auto& stack = *stackPtr;
    stack.push(std::move(mod));
    stack.apply(ostream);
}

template <typename CharT, typename Traits>
void tfmt::popModifier(std::basic_ostream<CharT, Traits>& ostream) {
    static auto const index = tcOStreamIndex();
    auto* const stackPtr = static_cast<ModStack*>(ostream.pword(index));
    assert(stackPtr != nullptr && "popModifier called without a prior call to pushModifier()?");
    auto& stack = *stackPtr;
    stack.pop();
    stack.apply(ostream);
}

template void tfmt::pushModifier(Modifier, std::ostream&);
template void tfmt::pushModifier(Modifier, std::wostream&);

template void tfmt::popModifier(std::ostream&);
template void tfmt::popModifier(std::wostream&);

void tfmt::pushModifier(Modifier mod) {
    pushModifier(std::move(mod), std::cout);
}

void tfmt::popModifier() {
    popModifier(std::cout);
}

template <>
tfmt::FormatGuard<std::ostream>::FormatGuard(Modifier mod): FormatGuard(std::move(mod), std::cout) {}

template <>
tfmt::FormatGuard<std::wostream>::FormatGuard(Modifier mod): FormatGuard(std::move(mod), std::wcout) {}

extern internal::ModBase const tfmt::reset  { "\033[00m"  };

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
