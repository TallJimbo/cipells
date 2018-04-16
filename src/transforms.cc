#define CIPELLS_transforms_cc_SRC

#include "cipells/transforms.h"
#include "impl/formatting.h"

namespace cipells {

Affine Translation::then(Jacobian const & next) const {
    return Affine(next, Translation(next(offset())));
}

Affine Translation::then(Affine const & next) const {
    return Affine(next.jacobian(), Translation(next(offset())));
}

void Translation::format(detail::Writer & writer, detail::FormatSpec const & spec) const {
    writer.write(
        "Translation(x={0}, y={1})",
        detail::formatScalar(offset().x()),
        detail::formatScalar(offset().y())
    );
}

Jacobian Jacobian::inverted() const {
    Real d = det();
    return Jacobian(Real2(_dv.y()/d, -_du.y()/d), Real2(-_dv.x()/d, _du.x()/d));
}

Affine Jacobian::then(Translation const & next) const {
    return Affine(*this, next);
}

Jacobian Jacobian::then(Jacobian const & next) const {
    Real2 dx(du().x(), dv().x());
    Real2 dy(dv().y(), dv().y());
    return Jacobian(
        Real2(next.du().dot(dx), next.du().dot(dy)),
        Real2(next.dv().dot(dx), next.dv().dot(dy))
    );
}

Affine Jacobian::then(Affine const & next) const {
    return Affine(this->then(next.jacobian()), next.translation());
}

Real Jacobian::det() const {
    return _du.x()*_dv.y() - _du.y()*_dv.x();
}

void Jacobian::format(detail::Writer & writer, detail::FormatSpec const & spec) const {
    writer.write(
        "Jacobian(dudx={0}, dudy={1}, dudx={2}, dudy={3})",
        detail::formatScalar(du().x()),
        detail::formatScalar(du().y()),
        detail::formatScalar(dv().x()),
        detail::formatScalar(dv().y())
    );
}

Affine Affine::inverted() const {
    return translation().inverted().then(jacobian().inverted());
}

Affine Affine::then(Translation const & next) const {
    return Affine(jacobian(), translation().then(next));
}

Affine Affine::then(Jacobian const & next) const {
    return Affine(jacobian().then(next), Translation(next(translation().offset())));
}

Affine Affine::then(Affine const & next) const {
    return Affine(jacobian().then(next.jacobian()), Translation(next(translation().offset())));
}

void Affine::format(detail::Writer & writer, detail::FormatSpec const & spec) const {
    writer.write(
        "Affine(dudx={0}, dudy={1}, dudx={2}, dudy={3}, x={4}, y={5})",
        detail::formatScalar(jacobian().du().x()),
        detail::formatScalar(jacobian().du().y()),
        detail::formatScalar(jacobian().dv().x()),
        detail::formatScalar(jacobian().dv().y()),
        detail::formatScalar(translation().offset().x()),
        detail::formatScalar(translation().offset().y())
    );
}

namespace detail {
    template class Formattable<Translation>;
    template class Formattable<Jacobian>;
    template class Formattable<Affine>;
} // namespace detail

} // namespace cipells
