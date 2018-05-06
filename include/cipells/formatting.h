#ifndef CIPELLS_formatting_h_INCLUDED
#define CIPELLS_formatting_h_INCLUDED

#include <string>

#include "cipells/common.h"

namespace fmt {

template <typename Char>
class ArgFormatter;

template <typename CharType,
          typename ArgFormatter>
class BasicFormatter;

template <typename CharType>
class BasicWriter;

} // namespace fmt

namespace cipells { namespace detail {

using Formatter = fmt::BasicFormatter<char, fmt::ArgFormatter<char>>;
using Writer = fmt::BasicWriter<char>;

using FormatSpec = std::pair<char const*, char const*>;

FormatSpec extractFormatSpec(char const *& tmpl);

template <typename Derived>
class Formattable {
public:

    std::string str() const;
    std::string repr() const;

    void format(Writer & writer, FormatSpec const & spec) const = delete;

    friend void format_arg(
        Formatter & formatter,
        char const * & tmpl,
        Formattable<Derived> const & self
    ) {
        self._formatArg(formatter, extractFormatSpec(tmpl));
    }

private:
    void _formatArg(Formatter & formatter, FormatSpec const & spec) const;
};

}} // cipells::detail

#endif // !CIPELLS_formatting_h_INCLUDED
