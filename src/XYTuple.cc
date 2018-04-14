#define CIPELLS_XYTuple_cc_SRC
#include "cipells/XYTuple.h"
#include "impl/formatting.h"

namespace cipells {

template <typename T>
void XYTuple<T>::format(detail::Formatter & formatter, char const * & tmpl) const {
    if (detail::isTemplateRepr(tmpl)) {
        formatter.writer().write(
            "{0:s}2(x={1}, y={2})",
            detail::ScalarFormatTraits<T>::NAME,
            detail::formatScalar(x()),
            detail::formatScalar(y())
        );
    } else {
        formatter.writer().write(
            "(x={0}, y={1})",
            detail::formatScalar(x()),
            detail::formatScalar(y())
        );
    }
}

template class XYTuple<Index>;
template class XYTuple<Real>;

namespace detail {

template class Formattable<XYTuple<Index>>;
template class Formattable<XYTuple<Real>>;

} // namespace detail

} // namespace cipells
