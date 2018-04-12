#define CIPELLS_Interval_cc
#include "cipells/Interval.h"

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
            *this = Derived(std::max(min(), rhs.min()), std::min(max(), rhs.max()));
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
            *this = Derived(std::min(min(), rhs.min()), std::max(max(), rhs.max()));
        }
    }
    return _self();
}

template <typename T, typename Derived>
Derived & BaseInterval<T, Derived>::expandTo(Scalar rhs) {
    if (rhs == rhs) {
        if (isEmpty()) {
            *this = Derived(rhs, rhs);
        } else {
            *this = Derived(std::min(min(), rhs), std::max(max(), rhs));
        }
    }
    return _self();
}

template <typename T, typename Derived>
Derived & BaseInterval<T, Derived>::dilateBy(Scalar rhs) {
    if (!isEmpty() && rhs == rhs) {
        *this = Derived(min() - rhs, max() + rhs);
    }
    return _self();
}

template class BaseInterval<Real, RealInterval>;
template class BaseInterval<Index, IndexInterval>;

} // namespace detail


IndexInterval::IndexInterval(RealInterval const & other) :
    Base(convertReal(other))
{}

RealInterval RealInterval::makeUniverse() {
    return RealInterval(-std::numeric_limits<Real>::infinity(),
                        std::numeric_limits<Real>::infinity());
}

RealInterval::RealInterval(IndexInterval const & other) :
    Base(convertIndex(other))
{}

} // namespace cipells