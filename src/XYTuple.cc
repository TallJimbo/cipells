#define CIPELLS_XYTuple_cc
#include "cipells/XYTuple.h"

namespace cipells {

template <typename T>
void XYTuple<T>::format(fmt::BasicFormatter<char> & formatter, char const * & tmpl) const {
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

} // namespace cipells
