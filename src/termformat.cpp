#include "termfmt/termfmt.h"

#include <cassert>
#include <iostream>
#include <new>
#include <vector>

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) ||               \
                         (defined(__APPLE__) && defined(__MACH__)))
#define TFMT_UNIX 1
#elif defined(_WIN32)
#define TFMT_WINDOWS 1
#else
#error Unknown platform
#endif

#if TFMT_UNIX
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#elif TFMT_WINDOWS
// #   define NOMINMAX
#include <io.h>
#include <windows.h>
#endif

using namespace tfmt;

static int isattyWrapper(FILE* file) {
#if TFMT_UNIX
    return isatty(fileno(file));
#elif TFMT_WINDOWS
    return _isatty(_fileno(file));
#else
#error
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
static bool isTerminalImpl(OStream const& ostream,
                           OStream const& globalOStream,
                           FILE* file) {
    if (&ostream == &globalOStream) {
        return filedescIsTerminal(file);
    }
    return false;
    ;
}

template <>
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

static long& iword(std::ios_base& ios) {
    static auto const index = tcOStreamIndex();
    return ios.iword(index);
}

static long iword(std::ios_base const& ios) {
    static auto const index = tcOStreamIndex();
    // We need to cast away constness to access .iword() method on
    // std::ios_base, as it does not provide a const overload.
    // However we take the argument by const& as conceptually this query does
    // not modify the object.
    std::ios_base& mutIos = const_cast<std::ios_base&>(ios);
    return mutIos.iword(index);
}

static constexpr size_t terminalBit = 0;
static constexpr size_t htmlBit = 1;
static constexpr size_t widthMask = 0xFF00;

static size_t getWidthImpl() {
#if TFMT_UNIX
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
#elif TFMT_WINDOWS
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
#error
#endif
}

template <typename CharT, typename Traits>
std::optional<size_t> tfmt::getWidth(
    std::basic_ostream<CharT, Traits> const& ostream) {
    size_t width = (static_cast<size_t>(iword(ostream)) & widthMask) >> 8;
    if (width > 0) {
        return width;
    }
    if (isTerminal(ostream)) {
        return getWidthImpl();
    }
    return std::nullopt;
}

template std::optional<size_t> tfmt::getWidth(std::ostream const&);
template std::optional<size_t> tfmt::getWidth(std::wostream const&);

template <typename CharT, typename Traits>
void tfmt::setWidth(std::basic_ostream<CharT, Traits>& ostream, size_t width) {
    auto& word = iword(ostream);
    assert(width < 256 && "Cannot set width higher than 256");
    word &= ~static_cast<long>(widthMask);
    word |= static_cast<long>(width) << 8;
}

template void tfmt::setWidth(std::ostream&, size_t);
template void tfmt::setWidth(std::wostream&, size_t);

template <typename CharT, typename Traits>
void tfmt::setTermFormattable(std::basic_ostream<CharT, Traits>& ostream,
                              bool value) {
    iword(ostream) |= static_cast<size_t>(value) << terminalBit;
}

template void tfmt::setTermFormattable(std::ostream&, bool);
template void tfmt::setTermFormattable(std::wostream&, bool);

template <typename CharT, typename Traits>
bool tfmt::isTermFormattable(std::basic_ostream<CharT, Traits> const& ostream) {
    return !!(iword(ostream) & 1 << terminalBit) || isTerminal(ostream);
}

template bool tfmt::isTermFormattable(std::ostream const&);
template bool tfmt::isTermFormattable(std::wostream const&);

template <typename CharT, typename Traits>
void tfmt::setHTMLFormattable(std::basic_ostream<CharT, Traits>& ostream,
                              bool value) {
    iword(ostream) |= value << htmlBit;
}

template void tfmt::setHTMLFormattable(std::ostream&, bool);
template void tfmt::setHTMLFormattable(std::wostream&, bool);

template <typename CharT, typename Traits>
bool tfmt::isHTMLFormattable(std::basic_ostream<CharT, Traits> const& ostream) {
    return !!(iword(ostream) & 1 << htmlBit);
}

template bool tfmt::isHTMLFormattable(std::ostream const&);
template bool tfmt::isHTMLFormattable(std::wostream const&);

template <typename CharT, typename Traits>
void tfmt::copyFormatFlags(std::basic_ostream<CharT, Traits> const& source,
                           std::basic_ostream<CharT, Traits>& dest) {
    if (isTermFormattable(source)) {
        setTermFormattable(dest);
    }
    if (isHTMLFormattable(source)) {
        setHTMLFormattable(dest);
    }
}

template void tfmt::copyFormatFlags(std::ostream const&, std::ostream&);
template void tfmt::copyFormatFlags(std::wostream const&, std::wostream&);

template <typename CharT, typename Traits>
static void putString(std::basic_ostream<CharT, Traits>& ostream,
                      std::string_view str) {
    for (char const c: str) {
        ostream.put(ostream.widen(c));
    }
}

template <typename CharT, typename Traits>
void internal::ModBase::put(std::basic_ostream<CharT, Traits>& ostream) const {
    if (isTermFormattable(ostream)) {
        putString(ostream, ansiBuffer);
    }
    if (isHTMLFormattable(ostream)) {
        if (isReset) {
            ostream << "</font>";
            return;
        }
        ostream << "<font color=\"";
        putString(ostream, htmlBuffer.front());
        ostream << "\">";
    }
}

template void internal::ModBase::put(std::ostream&) const;
template void internal::ModBase::put(std::wostream&) const;

namespace {

class ModStack {
public:
    ModStack() noexcept = default;

    void push(Modifier&& mod) { mods.push_back(std::move(mod)); }

    void pop() { mods.pop_back(); }

    template <typename CharT, typename Traits>
    void apply(std::basic_ostream<CharT, Traits>& ostream) const {
        for (Modifier mod: mods) {
            ostream << mod;
        }
    }

    template <typename CharT, typename Traits>
    void reset(std::basic_ostream<CharT, Traits>& ostream) const {
        for (Modifier mod: mods) {
            ostream << tfmt::Reset;
        }
    }

private:
    std::vector<Modifier> mods;
};

} // namespace

template <typename CharT, typename Traits>
void tfmt::pushModifier(Modifier mod,
                        std::basic_ostream<CharT, Traits>& ostream) {
    static auto const index = tcOStreamIndex();
    auto* stackPtr = static_cast<ModStack*>(ostream.pword(index));
    if (stackPtr == nullptr) {
        stackPtr = ::new ModStack();
        ostream.pword(index) = stackPtr;
        ostream.register_callback(
            [](std::ios_base::event event, std::ios_base& ios, int index) {
            if (event != std::ios_base::erase_event) {
                return;
            }
            auto* stackPtr = static_cast<ModStack*>(ios.pword(index));
            ::delete stackPtr;
            ios.pword(index) = nullptr;
            },
            index);
    }
    auto& stack = *stackPtr;
    stack.reset(ostream);
    stack.push(std::move(mod));
    stack.apply(ostream);
}

template <typename CharT, typename Traits>
void tfmt::popModifier(std::basic_ostream<CharT, Traits>& ostream) {
    static auto const index = tcOStreamIndex();
    auto* const stackPtr = static_cast<ModStack*>(ostream.pword(index));
    assert(stackPtr && "popModifier called without a matching prior call to "
                       "pushModifier()");
    auto& stack = *stackPtr;
    stack.reset(ostream);
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

void tfmt::popModifier() { popModifier(std::cout); }

template <>
tfmt::FormatGuard<std::ostream>::FormatGuard(Modifier mod):
    FormatGuard(std::move(mod), std::cout) {}

template <>
tfmt::FormatGuard<std::wostream>::FormatGuard(Modifier mod):
    FormatGuard(std::move(mod), std::wcout) {}

extern internal::ModBase const modifiers::Reset{
    "\033[00m", internal::ModBase::ResetTag{}
};

extern Modifier const modifiers::None{ "", "" };

extern Modifier const modifiers::Bold{ "\033[1m", "" };
extern Modifier const modifiers::Italic{ "\033[3m", "" };
extern Modifier const modifiers::Underline{ "\033[4m", "" };
extern Modifier const modifiers::Blink{ "\033[5m", "" };
extern Modifier const modifiers::Concealed{ "\033[8m", "" };
extern Modifier const modifiers::Crossed{ "\033[9m", "" };

extern Modifier const modifiers::Grey{ "\033[30m", "DimGray" };
extern Modifier const modifiers::Red{ "\033[31m", "Crimson" };
extern Modifier const modifiers::Green{ "\033[32m", "ForestGreen" };
extern Modifier const modifiers::Yellow{ "\033[33m", "DarkKhaki" };
extern Modifier const modifiers::Blue{ "\033[34m", "RoyalBlue" };
extern Modifier const modifiers::Magenta{ "\033[35m", "MediumVioletRed" };
extern Modifier const modifiers::Cyan{ "\033[36m", "DarkTurquoise" };
extern Modifier const modifiers::White{ "\033[37m", "" };

extern Modifier const modifiers::BrightGrey{ "\033[90m", "LightSlateGray" };
extern Modifier const modifiers::BrightRed{ "\033[91m", "Salmon" };
extern Modifier const modifiers::BrightGreen{ "\033[92m", "MediumSeaGreen" };
extern Modifier const modifiers::BrightYellow{ "\033[93m", "Khaki" };
extern Modifier const modifiers::BrightBlue{ "\033[94m", "CornflowerBlue" };
extern Modifier const modifiers::BrightMagenta{ "\033[95m", "DeepPink" };
extern Modifier const modifiers::BrightCyan{ "\033[96m", "MediumTurquoise" };
extern Modifier const modifiers::BrightWhite{ "\033[97m", "" };

extern Modifier const modifiers::BGGrey{ "\033[40m", "" };
extern Modifier const modifiers::BGRed{ "\033[41m", "" };
extern Modifier const modifiers::BGGreen{ "\033[42m", "" };
extern Modifier const modifiers::BGYellow{ "\033[43m", "" };
extern Modifier const modifiers::BGBlue{ "\033[44m", "" };
extern Modifier const modifiers::BGMagenta{ "\033[45m", "" };
extern Modifier const modifiers::BGCyan{ "\033[46m", "" };
extern Modifier const modifiers::BGWhite{ "\033[47m", "" };

extern Modifier const modifiers::BGBrightGrey{ "\033[100m", "" };
extern Modifier const modifiers::BGBrightRed{ "\033[101m", "" };
extern Modifier const modifiers::BGBrightGreen{ "\033[102m", "" };
extern Modifier const modifiers::BGBrightYellow{ "\033[103m", "" };
extern Modifier const modifiers::BGBrightBlue{ "\033[104m", "" };
extern Modifier const modifiers::BGBrightMagenta{ "\033[105m", "" };
extern Modifier const modifiers::BGBrightCyan{ "\033[106m", "" };
extern Modifier const modifiers::BGBrightWhite{ "\033[107m", "" };
