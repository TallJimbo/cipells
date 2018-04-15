#define CIPELLS_XYTuple_cc_SRC
#include "cipells/XYTuple.h"
#include "impl/formatting.h"

namespace cipells {

template <typename T>
void XYTuple<T>::format(detail::Writer & writer, detail::FormatSpec const & spec) const {
    if (detail::compareFormatSpec(spec, "r")) {
        writer.write(
            "{0:s}2(x={1}, y={2})",
            detail::ScalarFormatTraits<T>::NAME,
            detail::formatScalar(x()),
            detail::formatScalar(y())
        );
    } else if (detail::compareFormatSpec(spec, "s") || spec.first == spec.second) {
        writer.write(
            "(x={0}, y={1})",
            detail::formatScalar(x()),
            detail::formatScalar(y())
        );
    } else {
        throw fmt::FormatError("Format spec for XYTuple must be 'r', 's', or ''.");
    }
}

template class XYTuple<Index>;
template class XYTuple<Real>;

namespace detail {

template class Formattable<XYTuple<Index>>;
template class Formattable<XYTuple<Real>>;

} // namespace detail

} // namespace cipells
