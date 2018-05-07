#ifndef CIPELLS_XYTuple_h_INCLUDED
#define CIPELLS_XYTuple_h_INCLUDED

#include "Eigen/Core"

#include "cipells/fwd/XYTuple.h"
#include "cipells/formatting.h"

namespace cipells {

template <typename T>
using Vector2 = Eigen::Matrix<T, 2, 1, Eigen::DontAlign>;

template <typename T>
using Matrix2 = Eigen::Matrix<T, 2, 2, Eigen::RowMajor | Eigen::DontAlign>;

template <typename T>
class XYTuple : public detail::Formattable<XYTuple<T>> {
public:

    using Scalar = T;
    using Vector = Vector2<T>;

    XYTuple() : _vector{0, 0} {}

    XYTuple(T x, T y) : _vector{x, y} {}

    template <typename U>
    explicit XYTuple(XYTuple<U> const & other) :
        _vector{static_cast<T>(other.x()), static_cast<T>(other.y())}
    {}

    explicit XYTuple(Eigen::Ref<Vector const> const & vector) : _vector(vector) {}

    Scalar & x() { return _vector[0]; }
    Scalar const & x() const { return _vector[0]; }

    Scalar & y() { return _vector[1]; }
    Scalar const & y() const { return _vector[1]; }

    Scalar & operator[](int i) { return _vector[i]; }
    Scalar const & operator[](int i) const { return _vector[i]; }

    XYTuple operator+() const { return *this; }
    XYTuple operator-() const { return XYTuple(-vector()); }

    XYTuple & operator*=(Scalar rhs) { _vector *= rhs; return *this; }
    XYTuple & operator/=(Scalar rhs) { _vector /= rhs; return *this; }

    XYTuple operator*(Scalar rhs) const { return XYTuple(*this) *= rhs; }
    XYTuple operator/(Scalar rhs) const { return XYTuple(*this) /= rhs; }

    friend XYTuple operator*(Scalar lhs, XYTuple const & rhs) { return rhs*lhs; }

    XYTuple & operator+=(XYTuple const & rhs) { _vector += rhs.vector(); return *this; }
    XYTuple & operator-=(XYTuple const & rhs) { _vector -= rhs.vector(); return *this; }

    XYTuple operator+(XYTuple const & rhs) const { return XYTuple(*this) += rhs; }
    XYTuple operator-(XYTuple const & rhs) const { return XYTuple(*this) -= rhs; }

    bool operator==(XYTuple const & rhs) const { return x() == rhs.x() && y() == rhs.y(); }
    bool operator!=(XYTuple const & rhs) const { return !(*this == rhs); }

    Scalar dot(XYTuple const & other) const { return _vector.dot(other.vector()); }

    void format(detail::Writer & writer, detail::FormatSpec const & spec) const;

    Vector const & vector() const { return _vector; }

private:
    Vector _vector;
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