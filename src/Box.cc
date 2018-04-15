#define CIPELLS_Box_cc_SRC
#include "cipells/Box.h"
#include "impl/formatting.h"

namespace cipells {

namespace detail {

template <typename T, typename Derived>
Derived BaseBox<T, Derived>::fromMinMax(XYTuple<T> const & min_, XYTuple<T> const & max_) {
    return Derived(Interval<T>::fromMinMax(min_.x(), max_.x()),
                   Interval<T>::fromMinMax(min_.y(), max_.y()));
}

template <typename T, typename Derived>
Derived BaseBox<T, Derived>::fromMinSize(XYTuple<T> const & min_, XYTuple<T> const & size_) {
    return Derived(Interval<T>::fromMinSize(min_.x(), size_.x()),
                   Interval<T>::fromMinSize(min_.y(), size_.y()));
}

template <typename T, typename Derived>
Derived BaseBox<T, Derived>::fromMaxSize(XYTuple<T> const & max_, XYTuple<T> const & size_) {
    return Derived(Interval<T>::fromMaxSize(max_.x(), size_.x()),
                   Interval<T>::fromMaxSize(max_.y(), size_.y()));
}

template <typename T, typename Derived>
Derived BaseBox<T, Derived>::fromCenterSize(XYTuple<T> const & center_, XYTuple<T> const & size_) {
    return Derived(Interval<T>::fromCenterSize(center_.x(), size_.x()),
                   Interval<T>::fromCenterSize(center_.y(), size_.y()));
}

template <typename T, typename Derived>
std::array<XYTuple<T>, 4> BaseBox<T, Derived>::corners() const {
    return std::array<XYTuple<T>, 4>{
        XYTuple<T>(x().min(), y().min()),
        XYTuple<T>(x().max(), y().min()),
        XYTuple<T>(x().max(), y().max()),
        XYTuple<T>(x().min(), y().max())
    };
}

template <typename T, typename Derived>
void BaseBox<T, Derived>::format(Formatter & formatter, char const * & tmpl) const {
    if (detail::isTemplateRepr(tmpl)) {
        formatter.writer().write(
            "{0:s}Box(x={1:r}, y={2:r})",
            detail::ScalarFormatTraits<T>::NAME, x(), y()
        );
    } else {
        formatter.writer().write("{0}×{1}", x(), y());
    }
}

template class BaseBox<Real, RealBox>;
template class BaseBox<Index, IndexBox>;
template class Formattable<RealBox>;
template class Formattable<IndexBox>;

} // namespace detail

RealBox RealBox::makeUniverse() {
    return RealBox(RealInterval::makeUniverse(), RealInterval::makeUniverse());
}

} // namespace cipells
