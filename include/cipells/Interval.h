#ifndef CIPELLS_Interval_h_INCLUDED
#define CIPELLS_Interval_h_INCLUDED

#include <algorithm>

#include "cipells/fwd/Interval.h"
#include "cipells/formatting.h"

namespace cipells {

namespace detail {

template <typename T, typename Derived>
class BaseInterval : public Formattable<Derived> {
public:

    using Scalar = T;

    static Derived makeEmpty() { return Derived(); }

    template <typename Iterator>
    static Derived makeHull(Iterator first, Iterator last) {
        Derived result;
        for (auto iter = first; iter != last; ++iter) {
            result.expandTo(*iter);
        }
        return result;
    }

    template <typename Container>
    static Derived makeHull(Container const & container) {
        return makeHull(std::begin(container), std::end(container));
    }

    static Derived fromCenterSize(Scalar center, Scalar size) {
        Scalar min = center - size/2;
        return Derived::fromMinSize(min, size);
    }

    BaseInterval();

    explicit BaseInterval(std::pair<Scalar, Scalar> const & values);

    Scalar min() const { return _values.first; }
    Scalar max() const { return _values.second; }

    Scalar center() const { return min() + static_cast<Derived const &>(*this).size()/2; }

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

    void format(Formatter & formatter, char const * & tmpl) const;

private:

    Derived & _self() { return static_cast<Derived &>(*this); }
    Derived _copy() const { return Derived(static_cast<Derived const &>(*this)); }

    std::pair<Scalar, Scalar> _values;
};

} // namespace detail


class IndexInterval : public detail::BaseInterval<Index, IndexInterval> {
    using Base = detail::BaseInterval<Index, IndexInterval>;
public:

    static IndexInterval fromMinMax(Index min, Index max);
    static IndexInterval fromMinSize(Index min, Index size);
    static IndexInterval fromMaxSize(Index max, Index size);

    IndexInterval() : Base() {}

    explicit IndexInterval(RealInterval const & other);

    Index size() const { return this->isEmpty() ? 0 : 1 + this->max() - this->min(); }

private:
    IndexInterval(Index min, Index max) : Base(std::make_pair(min, max)) {}
};


class RealInterval : public detail::BaseInterval<Real, RealInterval> {
    using Base = detail::BaseInterval<Real, RealInterval>;
public:

    static RealInterval fromMinMax(Real min, Real max);
    static RealInterval fromMinSize(Real min, Real size);
    static RealInterval fromMaxSize(Real max, Real size);

    static RealInterval makeUniverse();

    RealInterval() : Base() {}


    RealInterval(IndexInterval const & other);

    Real size() const { return this->isEmpty() ? 0 : this->max() - this->min(); }

private:
    RealInterval(Real min, Real max) : Base(std::make_pair(min, max)) {}
};


#ifndef CIPELLS_Interval_cc_SRC
    namespace detail {
        extern template class BaseInterval<Index, IndexInterval>;
        extern template class BaseInterval<Real, RealInterval>;
        extern template class Formattable<IndexInterval>;
        extern template class Formattable<RealInterval>;
    } // namespace detail
#endif

} // namespace cipells

#endif // !CIPELLS_Interval_h_INCLUDED