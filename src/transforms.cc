#define CIPELLS_transforms_cc_SRC

#include "Eigen/LU"

#include "cipells/transforms.h"
#include "impl/formatting.h"

namespace cipells {

void Identity::format(detail::Writer & writer, detail::FormatSpec const & spec) const {
    writer.write("Identity()");
}

Affine Translation::then(Jacobian const & next) const {
    return Affine(next.matrix(), next.matrix()*vector());
}

Affine Translation::then(Affine const & next) const {
    return Affine(next.matrix(), next.matrix()*vector() + next.vector());
}

void Translation::format(detail::Writer & writer, detail::FormatSpec const & spec) const {
    writer.write(
        "Translation([{0}, {1}])",
        detail::formatScalar(_vector[0]),
        detail::formatScalar(_vector[1])
    );
}

Jacobian Jacobian::inverted() const {
    return Jacobian(matrix().inverse());
}

Affine Jacobian::then(Translation const & next) const {
    return Affine(*this, next);
}

Jacobian Jacobian::then(Jacobian const & next) const {
    return Jacobian(next.matrix()*matrix());
}

Affine Jacobian::then(Affine const & next) const {
    return Affine(next.matrix()*matrix(), next.vector());
}

Real Jacobian::det() const {
    return matrix().determinant();
}

void Jacobian::format(detail::Writer & writer, detail::FormatSpec const & spec) const {
    writer.write(
        "Jacobian([[{0}, {1}], [{2}, {3}]])",
        detail::formatScalar(_matrix(0, 0)),
        detail::formatScalar(_matrix(0, 1)),
        detail::formatScalar(_matrix(1, 0)),
        detail::formatScalar(_matrix(1, 1))
    );
}

Affine Affine::inverted() const {
    return translation().inverted().then(jacobian().inverted());
}

Affine Affine::then(Translation const & next) const {
    return Affine(jacobian(), translation().then(next));
}

Affine Affine::then(Jacobian const & next) const {
    return Affine(next.matrix()*matrix(), next.matrix()*vector());
}

Affine Affine::then(Affine const & next) const {
    return Affine(next.matrix()*matrix(), next.matrix()*vector() + next.vector());
}

void Affine::format(detail::Writer & writer, detail::FormatSpec const & spec) const {
    writer.write(
        "Affine([[{0}, {1}], [{2}, {3}]], [{4}, {5}])",
        detail::formatScalar(matrix()(0, 0)),
        detail::formatScalar(matrix()(0, 1)),
        detail::formatScalar(matrix()(1, 0)),
        detail::formatScalar(matrix()(1, 1)),
        detail::formatScalar(vector()[0]),
        detail::formatScalar(vector()[1])
    );
}

namespace detail {
    template class Formattable<Identity>;
    template class Formattable<Translation>;
    template class Formattable<Jacobian>;
    template class Formattable<Affine>;
} // namespace detail

} // namespace cipells
