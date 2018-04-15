#ifndef CIPELLS_Box_h_INCLUDED
#define CIPELLS_Box_h_INCLUDED

#include <array>

#include "cipells/fwd/Box.h"
#include "cipells/XYTuple.h"
#include "cipells/Interval.h"
#include "cipells/formatting.h"

namespace cipells {

namespace detail {

template <typename T, typename Derived>
class BaseBox : public Formattable<Derived> {
public:

    using Scalar = T;

    static Derived makeEmpty() { return Derived(); }

    template <typename Iterator>
    static Derived makeHull(Iterator first, Iterator last) {
        Derived r;
        for (auto iter = first; iter != last; ++iter) {
            r.expandTo(*iter);
        }
        return r;
    }

    template <typename Container>
    static Derived makeHull(Container const & container) {
        return makeHull(std::begin(container), std::end(container));
    }

    static Derived fromMinMax(XYTuple<T> const & min, XYTuple<T> const & max);
    static Derived fromMinSize(XYTuple<T> const & min, XYTuple<T> const & size);
    static Derived fromMaxSize(XYTuple<T> const & max, XYTuple<T> const & size);
    static Derived fromCenterSize(XYTuple<T> const & center, XYTuple<T> const & size);

    BaseBox() : _x(), _y() {}

    explicit BaseBox(Interval<T> const & x, Interval<T> const & y) : _x(x), _y(y) {}

    Interval<T> const & x() const { return _x; }
    Interval<T> const & y() const { return _y; }

    XYTuple<T> min() const { return XYTuple<T>(x().min(), y().min()); }
    XYTuple<T> max() const { return XYTuple<T>(x().max(), y().max()); }
    XYTuple<T> size() const { return XYTuple<T>(x().size(), y().size()); }
    XYTuple<T> center() const { return XYTuple<T>(x().center(), y().center()); }

    std::array<XYTuple<T>, 4> corners() const;

    Scalar x0() const { return x().min(); }
    Scalar y0() const { return y().min(); }

    Scalar x1() const { return x().max(); }
    Scalar y1() const { return y().max(); }

    Scalar width() const { return x().size(); }
    Scalar height() const { return y().size(); }
    Scalar area() const { return width()*height(); }

    bool isEmpty() const { return _x.isEmpty() || _y.isEmpty(); }

    bool contains(Derived const & rhs) const { return x().contains(rhs.x()) && y().contains(rhs.y()); }
    bool contains(XYTuple<T> const & rhs) const { return x().contains(rhs.x()) && y().contains(rhs.y()); }

    bool intersects(Derived const & rhs) const { return x().intersects(rhs.x()) && y().intersects(rhs.y()); }

    bool operator==(Derived const & rhs) const { return x() == rhs.x() && y() == rhs.y(); }
    bool operator!=(Derived const & rhs) const { return !(*this == rhs); }

    Derived & clipTo(Derived const & rhs) { _x.clipTo(rhs.x()); _y.clipTo(rhs.y()); return _self(); }
    Derived clippedTo(Derived const & rhs) const { return _copy().clipTo(rhs); }

    Derived & expandTo(Derived const & rhs) { _x.expandTo(rhs.x()); _y.expandTo(rhs.y()); return _self(); }
    Derived expandedTo(Derived const & rhs) const { return _copy().expandTo(rhs); }

    Derived & expandTo(XYTuple<T> const & rhs) { _x.expandTo(rhs.x()); _y.expandTo(rhs.y()); return _self(); }
    Derived expandedTo(XYTuple<T> const & rhs) const { return _copy().expandTo(rhs); }

    Derived & dilateBy(XYTuple<T> const & rhs) { _x.dilateBy(rhs.x()); _y.dilateBy(rhs.y()); return _self(); }
    Derived dilatedBy(XYTuple<T> const & rhs) const { return _copy().dilateBy(rhs); }

    Derived & dilateBy(Scalar rhs) { return dilateBy(XYTuple<T>(rhs, rhs)); }
    Derived dilatedBy(Scalar rhs) const { return _copy().dilateBy(rhs); }

    Derived & erodeBy(XYTuple<T> const & rhs) { return dilateBy(-rhs); }
    Derived erodedBy(XYTuple<T> const & rhs) const { return _copy().erodeBy(rhs); }

    Derived & erodeBy(Scalar rhs) { return dilateBy(-rhs); }
    Derived erodedBy(Scalar rhs) const { return _copy().erodeBy(rhs); }

    void format(Writer & writer, FormatSpec const & spec) const;

private:

    Derived & _self() { return static_cast<Derived &>(*this); }
    Derived _copy() const { return Derived(static_cast<Derived const &>(*this)); }

    Interval<T> _x;
    Interval<T> _y;
};

#ifndef CIPELLS_Box_cc
extern template class BaseBox<Index, IndexBox>;
extern template class BaseBox<Real, RealBox>;
#endif

} // namespace detail


class IndexBox : public detail::BaseBox<Index, IndexBox> {
    using Base = detail::BaseBox<Index, IndexBox>;
public:

    using Base::Base;

    IndexBox() = default;

    explicit IndexBox(RealBox const & rhs);

};


class RealBox : public detail::BaseBox<Real, RealBox> {
    using Base = detail::BaseBox<Real, RealBox>;
public:

    static RealBox makeUniverse();

    using Base::Base;

    RealBox() = default;

    RealBox(IndexBox const & rhs) : Base(rhs.x(), rhs.y()) {}

};


inline IndexBox::IndexBox(RealBox const & rhs) : Base(IndexInterval(rhs.x()), IndexInterval(rhs.y())) {}


} // namespace cipells

#endif // !CIPELLS_Box_h_INCLUDED