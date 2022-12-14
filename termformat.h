#ifndef TERMFORMAT_H_
#define TERMFORMAT_H_

#include <string>
#include <concepts>
#include <iosfwd>

// MARK: Forward declarations
// Public interface follows.

namespace tfmt {

class Mod;

} // namespace tfmt

namespace tfmt {
namespace internal {

class ModBase;
template <typename... T>
class ObjectWrapper;
template <typename CharT>
class OStreamWrapper;

template <typename T, typename CharT>
concept Printable = requires(std::basic_ostream<CharT>& ostream, T const& t) {
    { ostream << t } -> std::convertible_to<std::basic_ostream<CharT>&>;
};
template <typename T>
concept AnyPrintable = Printable<T, char> || Printable<T, wchar_t>;


}} // namespace tfmt::internal

// MARK: Public interface

namespace tfmt {

/// Determine wether \p ostream is backed by a terminal (which supports ANSI format codes).
template <typename CharT>
bool isTerminal(std::basic_ostream<CharT> const& ostream);

/// Set or unset \p ostream to be formattable.
/// \details This can be used to force emission of format codes into \p std::ostream objects which are not determined to be terminals by \p tfmt::isTerminal()
template <typename CharT>
void setFormattable(std::basic_ostream<CharT>& ostream, bool value = true);

/// Query wether \p ostream has been marked formattable with a call to \p setFormattable() or is a terminal as determined by \p tfmt::isTerminal()
template <typename CharT>
bool isFormattable(std::basic_ostream<CharT> const& ostream);

/// Combine modifiers \p lhs and \p rhs
Mod operator|(Mod const& rhs, Mod const& lhs);

/// \overload
Mod operator|(Mod&& rhs, Mod const& lhs);

/// Execute \p fn with modifiers applied.
/// \details Push \p mod to a stack of modifiers applied applied to \p ostream and execute \p fn .
/// \details After execution of \p fn the modifier \p mod will be popped from the stack.
template <typename CharT>
void format(Mod const& mod, std::basic_ostream<CharT>& ostream, std::invocable auto&& fn);

/// Wrap a set of objects with a modifier
/// \details Use with \p operator<<(std::ostream&,...) :
/// \details \p mod will be applied to the \p std::ostream object, \p objects... will be inserted and \p mod will be undone.
template <internal::AnyPrintable... T>
internal::ObjectWrapper<T...> format(Mod const& mod, T const&... objects);

/// Wrap \p ostream with the modifier \p mod
/// \details On every insertion into the return object, \p mod will be applied.
template <typename CharT>
internal::OStreamWrapper<CharT> format(Mod mod, std::basic_ostream<CharT>& ostream);

/// Reset all currently applied ANSI format codes.
/// This should not be used directly. Prefer using the \p format(...) wrapper functions above.
extern internal::ModBase const reset;

extern Mod const bold;
extern Mod const italic;
extern Mod const underline;
extern Mod const blink;
extern Mod const concealed;
extern Mod const crossed;

extern Mod const grey;
extern Mod const red;
extern Mod const green;
extern Mod const yellow;
extern Mod const blue;
extern Mod const magenta;
extern Mod const cyan;
extern Mod const white;

extern Mod const brightGrey;
extern Mod const brightRed;
extern Mod const brightGreen;
extern Mod const brightYellow;
extern Mod const brightBlue;
extern Mod const brightMagenta;
extern Mod const brightCyan;
extern Mod const brightWhite;

extern Mod const bgGrey;
extern Mod const bgRed;
extern Mod const bgGreen;
extern Mod const bgYellow;
extern Mod const bgBlue;
extern Mod const bgMagenta;
extern Mod const bgCyan;
extern Mod const bgWhite;

extern Mod const bgBrightGrey;
extern Mod const bgBrightRed;
extern Mod const bgBrightGreen;
extern Mod const bgBrightYellow;
extern Mod const bgBrightBlue;
extern Mod const bgBrightMagenta;
extern Mod const bgBrightCyan;
extern Mod const bgBrightWhite;

} // namespace tfmt

// MARK: Implementation

namespace tfmt::internal {

template <typename CharT>
void pushMod(std::basic_ostream<CharT>&, Mod const&);

template <typename CharT>
void popMod(std::basic_ostream<CharT>&);

template <typename F>
struct ScopeGuard {
    constexpr ScopeGuard(F&& f): f(std::move(f)) {}
    constexpr ~ScopeGuard() { std::invoke(f); }
    F f;
};

} // namespace tfmt::internal

template <typename CharT>
void tfmt::format(Mod const& mod, std::basic_ostream<CharT>& ostream, std::invocable auto&& fn) {
    internal::pushMod(ostream, mod);
    internal::ScopeGuard pop = [&]{ internal::popMod(ostream); };
    std::invoke(fn);
}

class tfmt::internal::ModBase {
public:
    explicit ModBase(std::string_view mod): buffer(mod) {}
    
    template <typename CharT>
    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& ostream, ModBase const& mod) {
        if (!isFormattable(ostream)) { return ostream; }
        for (char const c: mod.buffer) {
            ostream.put(ostream.widen(c));
        }
        return ostream;
    }
    
protected:
    std::string buffer;
};

class tfmt::Mod: public internal::ModBase {
public:
    using internal::ModBase::ModBase;
    
    friend Mod tfmt::operator|(Mod const& lhs, Mod const& rhs);
    friend Mod tfmt::operator|(Mod&& lhs, Mod const& rhs);
};

inline tfmt::Mod tfmt::operator|(Mod const& lhs, Mod const& rhs) {
    return Mod(lhs.buffer + rhs.buffer);
}

inline tfmt::Mod tfmt::operator|(Mod&& lhs, Mod const& rhs) {
    lhs.buffer += rhs.buffer;
    return Mod(std::move(lhs.buffer));
}

template <typename... T>
class tfmt::internal::ObjectWrapper {
public:
    explicit ObjectWrapper(Mod mod, T const&... objects): mod(std::move(mod)), objects(objects...) {}
    
    ObjectWrapper(ObjectWrapper const&) = delete;
    
    template <typename CharT>
    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& ostream, ObjectWrapper const& wrapper) {
        internal::pushMod(ostream, wrapper.mod);
        internal::ScopeGuard pop = [&]{ internal::popMod(ostream); };
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            ((ostream << std::get<I>(wrapper.objects)), ...);
        }(std::index_sequence_for<T...>{});
        return ostream;
    }
    
private:
    Mod mod;
    std::tuple<T const&...> objects;
};

template <tfmt::internal::AnyPrintable... T>
tfmt::internal::ObjectWrapper<T...> tfmt::format(Mod const& mod, T const&... objects) {
    return internal::ObjectWrapper<T...>(mod, objects...);
}

template <typename CharT>
class tfmt::internal::OStreamWrapper {
public:
    explicit OStreamWrapper(Mod mod, std::basic_ostream<CharT>& ostream): mod(std::move(mod)), ostream(ostream) {}
  
    template <Printable<CharT> T>
    friend OStreamWrapper<CharT>& operator<<(OStreamWrapper<CharT>& wrapper, T const& object) {
        internal::pushMod(wrapper.ostream, wrapper.mod);
        internal::ScopeGuard pop = [&]{ internal::popMod(wrapper.ostream); };
        wrapper.ostream << object;
        return wrapper;
    }
    
    template <Printable<CharT> T>
    friend OStreamWrapper<CharT>& operator<<(OStreamWrapper<CharT>&& wrapper, T const& object) {
        return static_cast<OStreamWrapper<CharT>&>(wrapper) << object;
    }
    
    friend OStreamWrapper<CharT>& operator<<(OStreamWrapper<CharT>& wrapper, std::basic_ostream<CharT>&(&modifier)(std::basic_ostream<CharT>&)) {
        modifier(wrapper.ostream);
        return wrapper;
    }
    
    friend OStreamWrapper<CharT>& operator<<(OStreamWrapper<CharT>&& wrapper, std::basic_ostream<CharT>&(&modifier)(std::basic_ostream<CharT>&)) {
        modifier(wrapper.ostream);
        return wrapper;
    }
    
private:
    Mod mod;
    std::basic_ostream<CharT>& ostream;
};

template <typename CharT>
tfmt::internal::OStreamWrapper<CharT> tfmt::format(Mod mod, std::basic_ostream<CharT>& ostream) {
    return internal::OStreamWrapper<CharT>(std::move(mod), ostream);
}

#endif // TERMFORMAT_H_
