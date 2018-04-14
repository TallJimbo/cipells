#ifndef CIPELLS_XYTuple_h_INCLUDED
#define CIPELLS_XYTuple_h_INCLUDED

#include <array>
#include <string>

#include "cipells/fwd/XYTuple.h"
#include "cipells/formatting.h"

namespace cipells {

template <typename T>
class XYTuple : public detail::Formattable<XYTuple<T>> {
public:

    using Scalar = T;

    XYTuple(T x, T y) : _array{x, y} {}

    template <typename U>
    explicit XYTuple(XYTuple<U> const & other) :
        _array{static_cast<T>(other.x()), static_cast<T>(other.y())}
    {}

    Scalar & x() { return _array[0]; }
    Scalar const & x() const { return _array[0]; }

    Scalar & y() { return _array[1]; }
    Scalar const & y() const { return _array[1]; }

    Scalar & operator[](int i) { return _array[i]; }
    Scalar const & operator[](int i) const { return _array[i]; }

    XYTuple operator+() const { return *this; }
    XYTuple operator-() const { return XYTuple(-x(), -y()); }

    XYTuple & operator*=(Scalar rhs) { x() *= rhs; y() *= rhs; return *this; }
    XYTuple & operator/=(Scalar rhs) { x() /= rhs; y() /= rhs; return *this; }

    XYTuple operator*(Scalar rhs) const { return XYTuple(*this) *= rhs; }
    XYTuple operator/(Scalar rhs) const { return XYTuple(*this) /= rhs; }

    friend XYTuple operator*(Scalar lhs, XYTuple const & rhs) { return rhs*lhs; }

    XYTuple & operator+=(XYTuple const & rhs) { x() += rhs.x(); y() += rhs.y(); return *this; }
    XYTuple & operator-=(XYTuple const & rhs) { x() -= rhs.x(); y() -= rhs.y(); return *this; }

    XYTuple operator+(XYTuple const & rhs) const { return XYTuple(*this) += rhs; }
    XYTuple operator-(XYTuple const & rhs) const { return XYTuple(*this) -= rhs; }

    bool operator==(XYTuple const & rhs) const { return x() == rhs.x() && y() == rhs.y(); }
    bool operator!=(XYTuple const & rhs) const { return !(*this == rhs); }

    Scalar dot(XYTuple const & other) const { return x()*other.x() + y()*other.y(); }

    void format(detail::Formatter & formatter, char const * & tmpl) const;

private:
    std::array<T,2> _array;
};

inline Index2 operator%(Index2 const & lhs, Index rhs) {
    return Index2(lhs.x() % rhs, lhs.y() % rhs);
}

#ifndef CIPELLS_XYTuple_cc_SRC
    extern template class XYTuple<Index>;
    extern template class XYTuple<Real>;
    namespace detail {
        extern template class Formattable<XYTuple<Index>>;
        extern template class Formattable<XYTuple<Real>>;
    } // namespace detail
#endif



} // namespace cipells

#endif // !CIPELLS_XYTuple_h_INCLUDED