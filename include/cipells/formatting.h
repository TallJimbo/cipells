#ifndef CIPELLS_formatting_h_INCLUDED
#define CIPELLS_formatting_h_INCLUDED

#include "cipells/common.h"

namespace fmt {

template <typename Char>
class ArgFormatter;

template <typename CharType,
          typename ArgFormatter>
class BasicFormatter;

} // namespace fmt

namespace cipells { namespace detail {

using Formatter = fmt::BasicFormatter<char, fmt::ArgFormatter<char>>;

template <typename Derived>
class Formattable {
public:

    std::string str() const;
    std::string repr() const;

    void format(Formatter & formatter, char const * & tmpl) const = delete;

    friend void format_arg(
        Formatter & formatter,
        char const * & tmpl,
        Formattable<Derived> const & self
    ) {
        static_cast<Derived const &>(self).format(formatter, tmpl);
    }

};

}} // cipells::detail

#endif // !CIPELLS_formatting_h_INCLUDED
