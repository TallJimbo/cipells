#ifndef CIPELLS_Interval_h_INCLUDED
#define CIPELLS_Interval_h_INCLUDED

#include <algorithm>
#include <string>
#include "cipells/fwd/Interval.h"

namespace cipells {

namespace detail {

template <typename T, typename Derived>
class BaseInterval {
public:

    using Scalar = T;

    static Derived makeEmpty() { return Derived(); }

    template <typename Iterator>
    static Derived makeHull(Iterator first, Iterator last) {
        auto p = std::minmax_element(first, last);
        return Derived(*p.first, *p.second);
    }

    template <typename Container>
    static Derived makeHull(Container const & container) {
        return makeHull(std::begin(container), std::end(container));
    }

    BaseInterval();

    explicit BaseInterval(std::pair<Scalar, Scalar> const & values);

    Scalar min() const { return _values.first; }
    Scalar max() const { return _values.second; }

    bool isEmpty() const { return !(min() <= max()); }

    bool contains(Derived const & rhs) const;
    bool contains(Scalar rhs) const;

    bool intersects(Derived const & rhs) const;

    bool operator==(Derived const & rhs) const;
    bool operator!=(Derived const & rhs) const { return !(*this == rhs); }

    Derived & clipTo(Derived const & rhs);
    Derived clippedTo(Derived const & rhs) const { return _copy().clipTo(rhs); }

    Derived & expandTo(Derived const & rhs);
    Derived expandedTo(Derived const & rhs) const { return _copy().expandTo(rhs); }

    Derived & expandTo(Scalar rhs);
    Derived expandedTo(Scalar rhs) const { return _copy().expandTo(rhs); }

    Derived & dilateBy(Scalar rhs);
    Derived dilatedBy(Scalar rhs) const { return _copy().dilateBy(rhs); }

    Derived & erodeBy(Scalar rhs) { return dilateBy(-rhs); }
    Derived erodedBy(Scalar rhs) const { return _copy().erodeBy(rhs); }

    std::string str() const;

private:

    Derived & _self() { return static_cast<Derived &>(*this); }
    Derived _copy() const { return Derived(static_cast<Derived const &>(*this)); }

    std::pair<Scalar, Scalar> _values;
};

#ifndef CIPELLS_Interval_cc
extern template class BaseInterval<Index, IndexInterval>;
extern template class BaseInterval<Real, RealInterval>;
#endif

} // namespace detail


class IndexInterval : public detail::BaseInterval<Index, IndexInterval> {
    using Base = detail::BaseInterval<Index, IndexInterval>;
public:

    IndexInterval() : Base() {}

    IndexInterval(Index min, Index max) : Base(std::make_pair(min, max)) {}

    explicit IndexInterval(RealInterval const & other);

    Index size() const { return this->isEmpty() ? 0 : 1 + this->max() - this->min(); }

    std::string repr() const;

};


class RealInterval : public detail::BaseInterval<Real, RealInterval> {
    using Base = detail::BaseInterval<Real, RealInterval>;
public:

    static RealInterval makeUniverse();

    RealInterval() : Base() {}

    RealInterval(Real min, Real max) : Base(std::make_pair(min, max)) {}

    RealInterval(IndexInterval const & other);

    Real size() const { return this->isEmpty() ? 0 : this->max() - this->min(); }

    std::string repr() const;

};


} // namespace cipells

#endif // !CIPELLS_Interval_h_INCLUDED