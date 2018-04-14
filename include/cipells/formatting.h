#ifndef CIPELLS_formatting_h_INCLUDED
#define CIPELLS_formatting_h_INCLUDED

#include "fmt/format.h"
#include "cipells/common.h"

namespace cipells { namespace detail {

template <typename Derived>
class Formattable {
public:

    std::string str() const;
    std::string repr() const;

    void format(fmt::BasicFormatter<char> & formatter, char const * & tmpl) const = delete;

    friend void format_arg(
        fmt::BasicFormatter<char> & formatter,
        char const * & tmpl,
        Formattable<Derived> const & self
    ) {
        static_cast<Derived const &>(self).format(formatter, tmpl);
    }

};

}} // cipells::detail

#endif // !CIPELLS_formatting_h_INCLUDED
