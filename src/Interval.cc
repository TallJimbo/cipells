#define CIPELLS_Interval_cc_SRC
#include "cipells/Interval.h"
#include "impl/formatting.h"

namespace cipells {

namespace {

std::pair<Real, Real> getStandardEmpty(Real *) {
    return std::make_pair(
        std::numeric_limits<Real>::quiet_NaN(),
        std::numeric_limits<Real>::quiet_NaN()
    );
}

std::pair<Index, Index> getStandardEmpty(Index *) {
    return std::make_pair(
        std::numeric_limits<Index>::max(),
        std::numeric_limits<Index>::min()
    );
}

std::pair<Index, Index> convertReal(RealInterval const & in) {
    if (in.isEmpty()) {
        return getStandardEmpty((Index*)nullptr);
    }
    return std::pair<Index, Index>(std::lround(in.min()), std::lround(in.max()));
}

std::pair<Real, Real> convertIndex(IndexInterval const & in) {
    if (in.isEmpty()) {
        return getStandardEmpty((Real*)nullptr);
    }
    return std::pair<Real, Real>(in.min() - 0.5, in.max() + 0.5);
}

} // anonymous

namespace detail {

template <typename T, typename Derived>
BaseInterval<T, Derived>::BaseInterval() : _values(getStandardEmpty((T*)nullptr)) {}

template <typename T, typename Derived>
BaseInterval<T, Derived>::BaseInterval(std::pair<T, T> const & values) : _values(values) {
    if (isEmpty()) {
        _values = getStandardEmpty((T*)nullptr);
    }
}

template <typename T, typename Derived>
bool BaseInterval<T, Derived>::contains(Derived const & rhs) const {
    if (rhs.isEmpty()) {
        return true;
    }
    if (isEmpty()) {
        return false;
    }
    return min() <= rhs.min() && max() >= rhs.max();
}

template <typename T, typename Derived>
bool BaseInterval<T, Derived>::contains(Scalar rhs) const {
    return min() <= rhs && max() >= rhs;
}

template <typename T, typename Derived>
bool BaseInterval<T, Derived>::intersects(Derived const & rhs) const {
    if (isEmpty() || rhs.isEmpty()) {
        return false;
    }
    return !(max() < rhs.min() || min() > rhs.max());
}

template <typename T, typename Derived>
bool BaseInterval<T, Derived>::operator==(Derived const & rhs) const {
    return (min() == rhs.min() && max() == rhs.max()) || (isEmpty() && rhs.isEmpty());
}

template <typename T, typename Derived>
Derived & BaseInterval<T, Derived>::clipTo(Derived const & rhs) {
    if (!isEmpty()) {
        if (rhs.isEmpty()) {
            _values = getStandardEmpty((T*)nullptr);
        } else {
            *this = Derived::fromMinMax(std::max(min(), rhs.min()), std::min(max(), rhs.max()));
        }
    }
    return _self();
}

template <typename T, typename Derived>
Derived & BaseInterval<T, Derived>::expandTo(Derived const & rhs) {
    if (!rhs.isEmpty()) {
        if (isEmpty()) {
            *this = rhs;
        } else {
            *this = Derived::fromMinMax(std::min(min(), rhs.min()), std::max(max(), rhs.max()));
        }
    }
    return _self();
}

template <typename T, typename Derived>
Derived & BaseInterval<T, Derived>::expandTo(Scalar rhs) {
    if (rhs == rhs) {
        if (isEmpty()) {
            *this = Derived::fromMinMax(rhs, rhs);
        } else {
            *this = Derived::fromMinMax(std::min(min(), rhs), std::max(max(), rhs));
        }
    }
    return _self();
}

template <typename T, typename Derived>
Derived & BaseInterval<T, Derived>::dilateBy(Scalar rhs) {
    if (!isEmpty() && rhs == rhs) {
        *this = Derived::fromMinMax(min() - rhs, max() + rhs);
    }
    return _self();
}

template <typename T, typename Derived>
void BaseInterval<T, Derived>::format(Formatter & formatter, char const * & tmpl) const {
    if (detail::isTemplateRepr(tmpl)) {
        if (isEmpty()) {
            formatter.writer().write("{0:s}Interval()", detail::ScalarFormatTraits<T>::NAME);
        } else {
            formatter.writer().write(
                "{0:s}Interval(min={1}, max={2})",
                detail::ScalarFormatTraits<T>::NAME,
                detail::formatScalar(min()),
                detail::formatScalar(max())
            );
        }
    } else {
        if (isEmpty()) {
            formatter.writer().write("[]");
        } else {
            formatter.writer().write(
                "[{0}, {1}]",
                detail::formatScalar(min()),
                detail::formatScalar(max())
            );
        }
    }
}


template class BaseInterval<Real, RealInterval>;
template class BaseInterval<Index, IndexInterval>;
template class Formattable<RealInterval>;
template class Formattable<IndexInterval>;

} // namespace detail

IndexInterval IndexInterval::fromMinMax(Index min_, Index max_) {
    return IndexInterval(min_, max_);
}

IndexInterval IndexInterval::fromMinSize(Index min_, Index size_) {
    return IndexInterval(min_, min_ + size_ - 1);
}

IndexInterval IndexInterval::fromMaxSize(Index max_, Index size_) {
    return IndexInterval(max_ - size_ + 1, max_);
}

IndexInterval::IndexInterval(RealInterval const & other) :
    Base(convertReal(other))
{}


RealInterval RealInterval::fromMinMax(Real min_, Real max_) {
    return RealInterval(min_, max_);
}

RealInterval RealInterval::fromMinSize(Real min_, Real size_) {
    return RealInterval(min_, min_ + size_);
}

RealInterval RealInterval::fromMaxSize(Real max_, Real size_) {
    return RealInterval(max_ - size_, max_);
}

RealInterval RealInterval::makeUniverse() {
    return RealInterval(-std::numeric_limits<Real>::infinity(),
                        std::numeric_limits<Real>::infinity());
}

RealInterval::RealInterval(IndexInterval const & other) :
    Base(convertIndex(other))
{}

} // namespace cipells
