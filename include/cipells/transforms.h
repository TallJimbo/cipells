#ifndef CIPELLS_transforms_h_INCLUDED
#define CIPELLS_transforms_h_INCLUDED

#include "Eigen/Core"

#include "cipells/XYTuple.h"
#include "cipells/fwd/transforms.h"
#include "cipells/formatting.h"

namespace cipells {

class Identity : public detail::Formattable<Identity> {
public:

    using Matrix = decltype(Matrix2<Real>::Identity());
    using Vector = decltype(Vector2<Real>::Zero());

    Identity() {}

    Real2 operator()(Real2 const & xy) const { return xy; }

    Identity inverted() const { return Identity(); }

    Identity then(Identity const &) const { return Identity(); }
    Translation then(Translation const & next) const;
    Jacobian then(Jacobian const & next) const;
    Affine then(Affine const & next) const;

    Real det() const { return 1; }

    void format(detail::Writer & writer, detail::FormatSpec const & spec) const;

    Matrix matrix() const { return Matrix2<Real>::Identity(); }

    Vector vector() const { return Vector2<Real>::Zero(); }

};

class Translation : public detail::Formattable<Translation> {
public:

    using Matrix = decltype(Matrix2<Real>::Identity());
    using Vector = Vector2<Real>;

    Translation() : _vector(0, 0) {}

    Translation(Identity const &) : Translation() {}

    explicit Translation(Real2 const & offset) : _vector(offset.vector()) {}

    explicit Translation(Eigen::Ref<Vector const> const & vector) : _vector(vector) {}

    Real2 operator()(Real2 const & xy) const { return Real2(xy.vector() + _vector); }

    Translation inverted() const { return Translation(-_vector); }

    Translation then(Identity const &) const { return *this; }
    Translation then(Translation const & next) const { return Translation(_vector + next._vector); }
    Affine then(Jacobian const & next) const;
    Affine then(Affine const & next) const;

    Real det() const { return 1; }

    void format(detail::Writer & writer, detail::FormatSpec const & spec) const;

    Matrix matrix() const { return Matrix2<Real>::Identity(); }

    Vector const & vector() const { return _vector; }

private:
    Vector _vector;
};


class Jacobian : public detail::Formattable<Jacobian> {
public:

    using Matrix = Matrix2<Real>;
    using Vector = decltype(Vector2<Real>::Zero());

    Jacobian() : _matrix(Matrix::Identity()) {}

    Jacobian(Identity const &) : Jacobian() {}

    explicit Jacobian(Eigen::Ref<Matrix const> const & matrix) : _matrix(matrix) {}

    Real2 operator()(Real2 const & xy) const { return Real2(matrix()*xy.vector()); }

    Jacobian inverted() const;

    Jacobian then(Identity const &) const { return *this; }
    Affine then(Translation const & next) const;
    Jacobian then(Jacobian const & next) const;
    Affine then(Affine const & next) const;

    Real det() const;

    void format(detail::Writer & writer, detail::FormatSpec const & spec) const;

    Matrix const & matrix() const { return _matrix; }

    Vector vector() const { return Vector2<Real>::Zero(); }

private:
     Matrix _matrix;
};


class Affine : public detail::Formattable<Affine> {
public:

    using Matrix = Jacobian::Matrix;
    using Vector = Translation::Vector;

    Affine() : _jacobian(), _translation() {}

    Affine(Identity const &) : Affine() {}

    explicit Affine(Eigen::Ref<Matrix const> const & matrix, Eigen::Ref<Vector const> const & vector) :
        _jacobian(matrix), _translation(vector)
    {}

    explicit Affine(Jacobian const & jacobian, Translation const & translation) :
        _jacobian(jacobian),
        _translation(translation)
    {}

    Affine(Jacobian const & jacobian) : _jacobian(jacobian), _translation() {}

    Affine(Translation const & translation) : _jacobian(), _translation(translation) {}

    Real2 operator()(Real2 const & xy) const { return _translation(_jacobian(xy)); }

    Affine inverted() const;

    Affine then(Identity const &) const { return *this; }
    Affine then(Translation const & next) const;
    Affine then(Jacobian const & next) const;
    Affine then(Affine const & next) const;

    Real det() const { return _jacobian.det(); }

    Matrix const & matrix() const { return _jacobian.matrix(); }
    Vector const & vector() const { return _translation.vector(); }

    Jacobian const & jacobian() const { return _jacobian; }
    Translation const & translation() const { return _translation; }

    void format(detail::Writer & writer, detail::FormatSpec const & spec) const;

private:
    Jacobian _jacobian;
    Translation _translation;
};


inline Translation Identity::then(Translation const & next) const { return next; }
inline Jacobian Identity::then(Jacobian const & next) const { return next; }
inline Affine Identity::then(Affine const & next) const { return next; }

} // namespace cipells

#endif // !CIPELLS_transforms_h_INCLUDED
