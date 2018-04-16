#ifndef CIPELLS_transforms_h_INCLUDED
#define CIPELLS_transforms_h_INCLUDED

#include "cipells/XYTuple.h"
#include "cipells/fwd/transforms.h"
#include "cipells/formatting.h"

namespace cipells {

class Translation : public detail::Formattable<Translation> {
public:

    Translation() : _offset(0, 0) {}

    Translation(Real x, Real y) : _offset(x, y) {}

    explicit Translation(Real2 offset) : _offset(offset) {}

    Real2 operator()(Real2 const & xy) const { return xy + _offset; }

    Translation inverted() const { return Translation(-_offset); }

    Translation then(Translation const & next) const { return Translation(_offset + next._offset); }
    Affine then(Jacobian const & next) const;
    Affine then(Affine const & next) const;

    Translation const & translation() const { return *this; }
    Jacobian jacobian() const;

    Real det() const { return 1; }

    void format(detail::Writer & writer, detail::FormatSpec const & spec) const;

    Real2 const & offset() const { return _offset; }

private:
    Real2 _offset;
};


class Jacobian : public detail::Formattable<Jacobian> {
public:

    Jacobian() : _du(1, 0), _dv(0, 1) {}

    Jacobian(Real dudx, Real dudy, Real dvdx, Real dvdy) : _du(dudx, dudy), _dv(dvdx, dvdy) {}

    explicit Jacobian(Real2 const & du, Real2 const & dv) : _du(du), _dv(dv) {}

    Real2 operator()(Real2 const & xy) const { return Real2(_du.dot(xy), _dv.dot(xy)); }

    Jacobian inverted() const;

    Affine then(Translation const & next) const;
    Jacobian then(Jacobian const & next) const;
    Affine then(Affine const & next) const;

    Translation translation() const { return Translation(); }
    Jacobian const & jacobian() const { return *this; }

    Real det() const;

    void format(detail::Writer & writer, detail::FormatSpec const & spec) const;

    Real2 const & du() const { return _du; }
    Real2 const & dv() const { return _dv; }

private:
    Real2 _du;
    Real2 _dv;
};


class Affine : public detail::Formattable<Affine> {
public:

    Affine() : _jacobian(), _translation() {}

    Affine(Real dudx, Real dudy, Real dvdx, Real dvdy, Real x, Real y) :
        _jacobian(dudx, dudy, dvdx, dvdy),
        _translation(x, y)
    {}

    explicit Affine(Jacobian const & jacobian, Translation const & translation) :
        _jacobian(jacobian),
        _translation(translation)
    {}

    Affine(Jacobian const & jacobian) : _jacobian(jacobian), _translation() {}

    Affine(Translation const & translation) : _jacobian(), _translation(translation) {}

    Real2 operator()(Real2 const & xy) const { return _translation(_jacobian(xy)); }

    Affine inverted() const;

    Affine then(Translation const & next) const;
    Affine then(Jacobian const & next) const;
    Affine then(Affine const & next) const;

    Translation const & translation() const { return _translation; }
    Jacobian const & jacobian() const { return _jacobian; }

    Real det() const { return _jacobian.det(); }

    void format(detail::Writer & writer, detail::FormatSpec const & spec) const;

private:
    Jacobian _jacobian;
    Translation _translation;
};


inline Jacobian Translation::jacobian() const { return Jacobian(); }

} // namespace cipells

#endif // !CIPELLS_transforms_h_INCLUDED
