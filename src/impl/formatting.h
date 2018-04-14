#ifndef CIPELLS_IMPL_formatting_h_INCLUDED
#define CIPELLS_IMPL_formatting_h_INCLUDED

#include "fmt/format.h"

#include "cipells/formatting.h"
#include "cipells/common.h"

namespace cipells { namespace detail {

template <typename T>
class ScalarFormatTraits;

template <>
class ScalarFormatTraits<Index> {
public:

    static char const * const NAME;
    static char const * const TMPL;

};

template <>
class ScalarFormatTraits<Real> {
public:

    static char const * const NAME;
    static char const * const TMPL;

};

template <typename T>
class ScalarFormatProxy {
public:

    ScalarFormatProxy(T v) : _value(v) {}

    friend void format_arg(
        Formatter & formatter,
        char const * &,
        ScalarFormatProxy<T> const & self
    ) {
        formatter.writer().write(ScalarFormatTraits<T>::TMPL, self._value);
    }

private:
    T _value;
};

template <typename T>
inline ScalarFormatProxy<T> formatScalar(T v) { return ScalarFormatProxy<T>(v); }


template <typename Derived>
std::string Formattable<Derived>::str() const { return fmt::format("{:s}", *this); }

template <typename Derived>
std::string Formattable<Derived>::repr() const { return fmt::format("{:r}", *this); }


bool isTemplateRepr(char const * & tmpl);


#ifndef CIPELLS_formatting_cc_SRC
extern template class ScalarFormatProxy<Index>;
extern template class ScalarFormatProxy<Real>;
#endif

}} // cipells::detail

#endif // !CIPELLS_IMPL_formatting_h_INCLUDED
