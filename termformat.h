#ifndef TERMFORMAT_H_
#define TERMFORMAT_H_

#include <concepts>
#include <string>
#include <iosfwd>

// Forward declarations
// Public interface follows.

#define TFMT_API /* Add definition here */

namespace tfmt {

class Modifier;

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

}} // namespace tfmt::internal

// ===------------------------------------------------------===
// === Public interface ------------------------------------===
// ===------------------------------------------------------===

namespace tfmt {

/// Determine wether \p ostream is backed by a terminal (which supports ANSI format codes).
template <typename CharT>
TFMT_API bool isTerminal(std::basic_ostream<CharT> const& ostream);

/// Set or unset \p ostream to be formattable.
/// \details This can be used to force emission of format codes into \p std::ostream objects which are not determined to be terminals by \p tfmt::isTerminal()
template <typename CharT>
TFMT_API void setFormattable(std::basic_ostream<CharT>& ostream, bool value = true);

/// Query wether \p ostream has been marked formattable with a call to \p setFormattable() or is a terminal as determined by \p tfmt::isTerminal()
template <typename CharT>
TFMT_API bool isFormattable(std::basic_ostream<CharT> const& ostream);

/// Combine modifiers \p lhs and \p rhs
TFMT_API Modifier operator|(Modifier const& rhs, Modifier const& lhs);

/// \overload
TFMT_API Modifier operator|(Modifier&& rhs, Modifier const& lhs);

/// Push a modifier to \p ostream .
/// \details \p pushModifier() and \p popModifier() associate objects of type \p std::basic_ostream<...> with a stack of modifiers. Stacks are destroyed with destruction of the ostream object.
template <typename CharT>
TFMT_API void pushModifier(Modifier mod, std::basic_ostream<CharT>& ostream);

/// \overload
/// Push modifier to stdout.
TFMT_API void pushModifier(Modifier);

/// Pop a modifier from \p ostream .
template <typename CharT>
TFMT_API void popModifier(std::basic_ostream<CharT>& ostream);

/// \overload
/// Pop modifier from stdout.
TFMT_API void popModifier();

/// Scope guard object. Push modifier \p mod to \p ostream on construction, pop on destruction.
template <typename OStream>
class TFMT_API FormatGuard {
public:
    explicit FormatGuard(Modifier mod, OStream& ostream);
    
    ~FormatGuard();
    
private:
    OStream* ostream;
};

/// \p FormatGuard for \p stdout .
template <>
class TFMT_API FormatGuard<void> {
public:
    explicit FormatGuard(Modifier mod);
    
    ~FormatGuard();
};

FormatGuard(Modifier) -> FormatGuard<void>;

/// Execute \p fn with modifiers applied.
/// \details Push \p mod to a stack of modifiers applied applied to \p ostream and execute \p fn .
/// \details After execution of \p fn the modifier \p mod will be popped from the stack.
template <typename CharT>
TFMT_API void format(Modifier mod, std::basic_ostream<CharT>& ostream, std::invocable auto&& fn);

/// \overload
/// Execute \p fn with modifiers applied to \p stdout
TFMT_API void format(Modifier mod, std::invocable auto&& fn);

/// Wrap a set of objects with a modifier
/// \details Use with \p operator<<(std::ostream&,...) :
/// \details \p mod will be applied to the \p std::ostream object, \p objects... will be inserted and \p mod will be undone.
template <typename... T>
TFMT_API internal::ObjectWrapper<T...> format(Modifier mod, T const&... objects);

/// Wrap \p ostream with the modifier \p mod
/// \details On every insertion into the return object, \p mod will be applied.
template <typename CharT>
TFMT_API internal::OStreamWrapper<CharT> format(Modifier mod, std::basic_ostream<CharT>& ostream);

/// Reset all currently applied ANSI format codes.
/// This should not be used directly. Prefer using the \p format(...) wrapper functions above.
extern internal::ModBase const reset;

extern Modifier const bold;
extern Modifier const italic;
extern Modifier const underline;
extern Modifier const blink;
extern Modifier const concealed;
extern Modifier const crossed;

extern Modifier const grey;
extern Modifier const red;
extern Modifier const green;
extern Modifier const yellow;
extern Modifier const blue;
extern Modifier const magenta;
extern Modifier const cyan;
extern Modifier const white;

extern Modifier const brightGrey;
extern Modifier const brightRed;
extern Modifier const brightGreen;
extern Modifier const brightYellow;
extern Modifier const brightBlue;
extern Modifier const brightMagenta;
extern Modifier const brightCyan;
extern Modifier const brightWhite;

extern Modifier const bgGrey;
extern Modifier const bgRed;
extern Modifier const bgGreen;
extern Modifier const bgYellow;
extern Modifier const bgBlue;
extern Modifier const bgMagenta;
extern Modifier const bgCyan;
extern Modifier const bgWhite;

extern Modifier const bgBrightGrey;
extern Modifier const bgBrightRed;
extern Modifier const bgBrightGreen;
extern Modifier const bgBrightYellow;
extern Modifier const bgBrightBlue;
extern Modifier const bgBrightMagenta;
extern Modifier const bgBrightCyan;
extern Modifier const bgBrightWhite;

} // namespace tfmt

// ===------------------------------------------------------===
// === Inline implementation -------------------------------===
// ===------------------------------------------------------===

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

class tfmt::Modifier: public internal::ModBase {
public:
    using internal::ModBase::ModBase;
    
    friend Modifier tfmt::operator|(Modifier const& lhs, Modifier const& rhs);
    friend Modifier tfmt::operator|(Modifier&& lhs, Modifier const& rhs);
};

inline tfmt::Modifier tfmt::operator|(Modifier const& lhs, Modifier const& rhs) {
    return Modifier(lhs.buffer + rhs.buffer);
}

inline tfmt::Modifier tfmt::operator|(Modifier&& lhs, Modifier const& rhs) {
    lhs.buffer += rhs.buffer;
    return Modifier(std::move(lhs.buffer));
}

template <typename OStream>
tfmt::FormatGuard<OStream>::FormatGuard(Modifier mod, OStream& ostream): ostream(&ostream) {
    pushModifier(std::move(mod), *this->ostream);
}

template <typename OStream>
tfmt::FormatGuard<OStream>::~FormatGuard() {
    popModifier(*this->ostream);
}

inline tfmt::FormatGuard<void>::FormatGuard(Modifier mod) {
    pushModifier(std::move(mod));
}

inline tfmt::FormatGuard<void>::~FormatGuard() {
    popModifier();
}

template <typename CharT>
void tfmt::format(Modifier mod, std::basic_ostream<CharT>& ostream, std::invocable auto&& fn) {
    FormatGuard fmt(std::move(mod), ostream);
    std::invoke(fn);
}

void tfmt::format(Modifier mod, std::invocable auto&& fn) {
    FormatGuard fmt(std::move(mod));
    std::invoke(fn);
}

template <typename... T>
class tfmt::internal::ObjectWrapper {
public:
    explicit ObjectWrapper(Modifier mod, T const&... objects): mod(std::move(mod)), objects(objects...) {}
    
    ObjectWrapper(ObjectWrapper const&) = delete;
    
    template <typename CharT> requires (... && Printable<T, CharT>)
    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& ostream, ObjectWrapper const& wrapper) {
        FormatGuard fmt(wrapper.mod, ostream);
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            ((ostream << std::get<I>(wrapper.objects)), ...);
        }(std::index_sequence_for<T...>{});
        return ostream;
    }
    
private:
    Modifier mod;
    std::tuple<T const&...> objects;
};

template <typename... T>
tfmt::internal::ObjectWrapper<T...> tfmt::format(Modifier mod, T const&... objects) {
    return internal::ObjectWrapper<T...>(std::move(mod), objects...);
}

template <typename CharT>
class tfmt::internal::OStreamWrapper {
public:
    explicit OStreamWrapper(Modifier mod, std::basic_ostream<CharT>& ostream): mod(std::move(mod)), ostream(ostream) {}
  
    template <Printable<CharT> T>
    friend OStreamWrapper<CharT>& operator<<(OStreamWrapper<CharT>& wrapper, T const& object) {
        FormatGuard fmt(wrapper.mod, wrapper.ostream);
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
    Modifier mod;
    std::basic_ostream<CharT>& ostream;
};

template <typename CharT>
tfmt::internal::OStreamWrapper<CharT> tfmt::format(Modifier mod, std::basic_ostream<CharT>& ostream) {
    return internal::OStreamWrapper<CharT>(std::move(mod), ostream);
}

#undef TFMT_API

#endif // TERMFORMAT_H_
