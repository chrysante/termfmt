#ifndef TERMFORMAT_H_
#define TERMFORMAT_H_

#include <concepts>
#include <functional>
#include <iosfwd>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <termfmt/api.h>

// Forward declarations
// Public interface follows.

namespace tfmt {

class Modifier;

} // namespace tfmt

namespace tfmt::internal {

class ModBase;
template <typename... T>
class ObjectWrapper;
template <typename CharT, typename Traits>
class OStreamWrapper;

template <typename T, typename CharT, typename Traits>
concept Printable =
    requires(std::basic_ostream<CharT, Traits>& ostream, T const& t) {
        {
            ostream << t
        } -> std::convertible_to<std::basic_ostream<CharT, Traits>&>;
    };

} // namespace tfmt::internal

// ===------------------------------------------------------===
// === Public interface ------------------------------------===
// ===------------------------------------------------------===

namespace tfmt {

/// Determine whether \p ostream is backed by a terminal (which supports ANSI
/// format codes).
template <typename CharT, typename Traits>
TFMT_API bool isTerminal(std::basic_ostream<CharT, Traits> const& ostream);

/// \Returns the available width of \p ostream if it is a terminal or the user
/// defined width set by `setWidth()`. Otherwise returns `std::nullopt`
template <typename CharT, typename Traits>
TFMT_API std::optional<size_t> getWidth(
    std::basic_ostream<CharT, Traits> const& ostream);

template <typename CharT, typename Traits>
TFMT_API void setWidth(std::basic_ostream<CharT, Traits>& ostream,
                       size_t width);

/// Set or unset \p ostream to be formattable with ANSI format codes.
/// \details This can be used to force emission of ANSI format codes into
/// `std::ostream` objects which are not determined to be terminals by
/// `tfmt::isTerminal()`
template <typename CharT, typename Traits>
TFMT_API void setTermFormattable(std::basic_ostream<CharT, Traits>& ostream,
                                 bool value = true);

/// Query whether \p ostream has been marked formattable with ANSI format codes
/// with a call to `setTermFormattable()` or is a terminal as determined by
/// `tfmt::isTerminal()`
template <typename CharT, typename Traits>
TFMT_API bool isTermFormattable(
    std::basic_ostream<CharT, Traits> const& ostream);

/// Set or unset \p ostream to be formattable with HTML format codes.
/// \details This can be used to force emission of HTML format codes into
/// `std::ostream` objects.
template <typename CharT, typename Traits>
TFMT_API void setHTMLFormattable(std::basic_ostream<CharT, Traits>& ostream,
                                 bool value = true);

/// Query whether \p ostream has been marked HTML formattable with a call to
/// `setHTMLFormattable()`
template <typename CharT, typename Traits>
TFMT_API bool isHTMLFormattable(
    std::basic_ostream<CharT, Traits> const& ostream);

/// Copies all TFMT format flags from \p source to \p dest
template <typename CharT, typename Traits>
TFMT_API void copyFormatFlags(std::basic_ostream<CharT, Traits> const& source,
                              std::basic_ostream<CharT, Traits>& dest);

/// Combine modifiers \p lhs and \p rhs
TFMT_API Modifier operator|(Modifier const& rhs, Modifier const& lhs);

/// \overload
TFMT_API Modifier operator|(Modifier&& rhs, Modifier const& lhs);

/// Combine \p rhs into \p lhs
/// \Returns A reference to \p lhs
TFMT_API Modifier& operator|=(Modifier& rhs, Modifier const& lhs);

/// Push a modifier to \p ostream .
/// \details `pushModifier()` and `popModifier()` associate objects of type
/// `std::basic_ostream<...>` with a stack of modifiers. Stacks are destroyed
/// with destruction of the ostream object.
template <typename CharT, typename Traits>
TFMT_API void pushModifier(Modifier mod,
                           std::basic_ostream<CharT, Traits>& ostream);

/// \overload
/// Push modifier to `stdout`.
TFMT_API void pushModifier(Modifier);

/// Pop a modifier from \p ostream .
template <typename CharT, typename Traits>
TFMT_API void popModifier(std::basic_ostream<CharT, Traits>& ostream);

/// \overload
/// Pop modifier from `stdout`.
TFMT_API void popModifier();

/// Scope guard object. Push modifier \p mod to \p ostream on construction, pop
/// on destruction.
template <typename OStream>
class TFMT_API FormatGuard {
public:
    /// Apply \p mod to \p ostream for the lifetime of this object.
    explicit FormatGuard(Modifier mod, OStream& ostream);

    /// Apply \p mod to `stdout` for the lifetime of this object.
    explicit FormatGuard(Modifier mod);

    FormatGuard(FormatGuard&& rhs) noexcept;

    /// Move assign \p rhs to `*this` . `pop()` will be called on `*this` and \p
    /// rhs will be empty after the call.
    FormatGuard& operator=(FormatGuard&& rhs) noexcept;

    ~FormatGuard();

    void pop();

private:
    OStream* ostream;
};

FormatGuard(Modifier) -> FormatGuard<std::ostream>;

/// Execute \p fn with modifiers applied.
/// \details Push \p mod to a stack of modifiers applied applied to \p ostream
/// and execute \p fn . \details After execution of \p fn the modifier \p mod
/// will be popped from the stack.
template <typename CharT, typename Traits>
TFMT_API void format(Modifier mod,
                     std::basic_ostream<CharT, Traits>& ostream,
                     std::invocable auto&& fn);

/// \overload
/// Execute \p fn with modifiers applied to `stdout`
TFMT_API void format(Modifier mod, std::invocable auto&& fn);

/// Wrap a set of objects with a modifier
/// \details Use with `operator<<(std::ostream&, ...)`:
/// \p mod will be applied to the `std::ostream` object, \p objects... will be
/// inserted and \p mod will be undone.
template <typename... T>
TFMT_API internal::ObjectWrapper<T...> format(Modifier mod, T&&... objects);

/// Wrap \p ostream with the modifier \p mod
/// \details On every insertion into the return object, \p mod will be applied.
template <typename CharT, typename Traits>
TFMT_API internal::OStreamWrapper<CharT, Traits> format(
    Modifier mod, std::basic_ostream<CharT, Traits>& ostream);

/// Type erased class giving a unified interface for the return types of the
/// `format(Modifier mod, T&&... objects)` functions.
template <typename CharT, typename Traits>
class BasicVObjectWrapper;

/// Typedef of `BasicVObjectWrapper` for `CharT == char` and
/// `Traits == std::char_traits<char>`.
using VObjectWrapper = BasicVObjectWrapper<char, std::char_traits<char>>;

/// List of all modifiers. This is in an inline namespace to use `using
/// namespace tfmt::modifiers;` to pull all modifiers into the global scope

inline namespace modifiers {

/// Reset all currently applied ANSI format codes.
/// This should not be used directly. Prefer using the `format(...)` wrapper
/// functions above.
TFMT_API extern Modifier const Reset;

TFMT_API extern Modifier const None;

TFMT_API extern Modifier const Bold;
TFMT_API extern Modifier const Italic;
TFMT_API extern Modifier const Underline;
TFMT_API extern Modifier const Blink;
TFMT_API extern Modifier const Concealed;
TFMT_API extern Modifier const Crossed;

TFMT_API extern Modifier const Grey;
TFMT_API extern Modifier const Red;
TFMT_API extern Modifier const Green;
TFMT_API extern Modifier const Yellow;
TFMT_API extern Modifier const Blue;
TFMT_API extern Modifier const Magenta;
TFMT_API extern Modifier const Cyan;
TFMT_API extern Modifier const White;

TFMT_API extern Modifier const BrightGrey;
TFMT_API extern Modifier const BrightRed;
TFMT_API extern Modifier const BrightGreen;
TFMT_API extern Modifier const BrightYellow;
TFMT_API extern Modifier const BrightBlue;
TFMT_API extern Modifier const BrightMagenta;
TFMT_API extern Modifier const BrightCyan;
TFMT_API extern Modifier const BrightWhite;

TFMT_API extern Modifier const BGGrey;
TFMT_API extern Modifier const BGRed;
TFMT_API extern Modifier const BGGreen;
TFMT_API extern Modifier const BGYellow;
TFMT_API extern Modifier const BGBlue;
TFMT_API extern Modifier const BGMagenta;
TFMT_API extern Modifier const BGCyan;
TFMT_API extern Modifier const BGWhite;

TFMT_API extern Modifier const BGBrightGrey;
TFMT_API extern Modifier const BGBrightRed;
TFMT_API extern Modifier const BGBrightGreen;
TFMT_API extern Modifier const BGBrightYellow;
TFMT_API extern Modifier const BGBrightBlue;
TFMT_API extern Modifier const BGBrightMagenta;
TFMT_API extern Modifier const BGBrightCyan;
TFMT_API extern Modifier const BGBrightWhite;

} // namespace modifiers

} // namespace tfmt

// ===------------------------------------------------------===
// === Inline implementation -------------------------------===
// ===------------------------------------------------------===

class tfmt::internal::ModBase {
public:
    enum class ResetTag {};

public:
    explicit ModBase(std::string_view ansiMod, ResetTag):
        ansiBuffer(ansiMod), isReset(true) {}
    explicit ModBase(std::string_view ansiMod, std::string htmlMod):
        ModBase(ansiMod, std::vector{ htmlMod }) {}
    explicit ModBase(std::string_view ansiMod,
                     std::vector<std::string> htmlMod):
        ansiBuffer(ansiMod), htmlBuffer(htmlMod) {}

    template <typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<(
        std::basic_ostream<CharT, Traits>& ostream, ModBase const& mod) {
        mod.put(ostream);
        return ostream;
    }

private:
    template <typename CharT, typename Traits>
    void put(std::basic_ostream<CharT, Traits>& ostream) const;

protected:
    std::string ansiBuffer;
    std::vector<std::string> htmlBuffer;
    bool isReset = false;
};

class tfmt::Modifier: public internal::ModBase {
public:
    using internal::ModBase::ModBase;

    friend Modifier tfmt::operator|(Modifier const& lhs, Modifier const& rhs);
    friend Modifier tfmt::operator|(Modifier&& lhs, Modifier const& rhs);
};

inline tfmt::Modifier tfmt::operator|(Modifier const& lhs,
                                      Modifier const& rhs) {
    auto htmlBuffer = lhs.htmlBuffer;
    htmlBuffer.insert(htmlBuffer.end(),
                      rhs.htmlBuffer.begin(),
                      rhs.htmlBuffer.end());
    return Modifier(lhs.ansiBuffer + rhs.ansiBuffer, std::move(htmlBuffer));
}

inline tfmt::Modifier tfmt::operator|(Modifier&& lhs, Modifier const& rhs) {
    lhs.ansiBuffer += rhs.ansiBuffer;
    lhs.htmlBuffer.insert(lhs.htmlBuffer.end(),
                          rhs.htmlBuffer.begin(),
                          rhs.htmlBuffer.end());
    return lhs;
}

inline tfmt::Modifier& tfmt::operator|=(Modifier& lhs, Modifier const& rhs) {
    return lhs = std::move(lhs) | rhs;
}

template <typename CharT, typename Traits>
void tfmt::format(Modifier mod,
                  std::basic_ostream<CharT, Traits>& ostream,
                  std::invocable auto&& fn) {
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
    explicit ObjectWrapper(Modifier mod, T&&... objects):
        mod(std::move(mod)), objects(std::forward<T>(objects)...) {}

    template <typename CharT, typename Traits>
        requires(... && Printable<T, CharT, Traits>)
    friend std::basic_ostream<CharT, Traits>& operator<<(
        std::basic_ostream<CharT, Traits>& ostream,
        ObjectWrapper const& wrapper) {
        FormatGuard fmt(wrapper.mod, ostream);
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            ((ostream << std::get<I>(wrapper.objects)), ...);
        }(std::index_sequence_for<T...>{});
        return ostream;
    }

private:
    Modifier mod;
    std::tuple<std::decay_t<T>...> objects;
};

template <typename... T>
tfmt::internal::ObjectWrapper<T...> tfmt::format(Modifier mod, T&&... objects) {
    return internal::ObjectWrapper<T...>(std::move(mod),
                                         std::forward<T>(objects)...);
}

template <typename CharT, typename Traits>
class tfmt::BasicVObjectWrapper {
    struct Tag {};
    using OstreamT = std::basic_ostream<CharT, Traits>;

public:
    template <typename... T>
    BasicVObjectWrapper(internal::ObjectWrapper<T...> const& objWrapper):
        VObjectWrapper(objWrapper, Tag{}) {}

    template <typename... T>
    BasicVObjectWrapper(internal::ObjectWrapper<T...>&& objWrapper):
        VObjectWrapper(std::move(objWrapper), Tag{}) {}

    friend std::basic_ostream<CharT, Traits>& operator<<(
        std::basic_ostream<CharT, Traits>& ostream,
        BasicVObjectWrapper<CharT, Traits> const& wrapper) {
        return wrapper.impl(ostream);
    }

private:
    template <typename T>
    explicit BasicVObjectWrapper(T&& objWrapper, Tag):
        impl([ow = std::forward<T>(objWrapper)](OstreamT& str) -> OstreamT& {
            return str << ow;
        }) {}

    std::function<OstreamT&(OstreamT&)> impl;
};

template <typename CharT, typename Traits>
class tfmt::internal::OStreamWrapper {
public:
    explicit OStreamWrapper(Modifier mod,
                            std::basic_ostream<CharT, Traits>& ostream):
        mod(std::move(mod)), ostream(ostream) {}

    template <Printable<CharT, Traits> T>
    friend OStreamWrapper<CharT, Traits>& operator<<(
        OStreamWrapper<CharT, Traits>& wrapper, T const& object) {
        FormatGuard fmt(wrapper.mod, wrapper.ostream);
        wrapper.ostream << object;
        return wrapper;
    }

    template <Printable<CharT, Traits> T>
    friend OStreamWrapper<CharT, Traits>& operator<<(
        OStreamWrapper<CharT, Traits>&& wrapper, T const& object) {
        return static_cast<OStreamWrapper<CharT, Traits>&>(wrapper) << object;
    }

    using OStreamModifier =
        std::basic_ostream<CharT,
                           Traits>& (&)(std::basic_ostream<CharT, Traits>&);

    friend OStreamWrapper<CharT, Traits>& operator<<(
        OStreamWrapper<CharT, Traits>& wrapper, OStreamModifier modifier) {
        modifier(wrapper.ostream);
        return wrapper;
    }

    friend OStreamWrapper<CharT, Traits>& operator<<(
        OStreamWrapper<CharT, Traits>&& wrapper, OStreamModifier modifier) {
        return static_cast<OStreamWrapper<CharT, Traits>&>(wrapper) << modifier;
    }

private:
    Modifier mod;
    std::basic_ostream<CharT, Traits>& ostream;
};

template <typename CharT, typename Traits>
tfmt::internal::OStreamWrapper<CharT, Traits> tfmt::format(
    Modifier mod, std::basic_ostream<CharT, Traits>& ostream) {
    return internal::OStreamWrapper<CharT, Traits>(std::move(mod), ostream);
}

#endif // TERMFORMAT_H_
