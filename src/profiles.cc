#define CIPELLS_profiles_cc_SRC

#include <cmath>

#include "cipells/profiles.h"
#include "cipells/Image.h"
#include "impl/formatting.h"

namespace cipells {

Gaussian::Gaussian(Affine const & transform, double flux) :
    _transform(transform),
    _inv_transform(transform.inverted()),
    _flux(flux)
{}

double Gaussian::operator()(Real2 const & xy) const {
    double z = _inv_transform(xy).vector().squaredNorm();
    return _flux*std::exp(-0.5*z)/(2*M_PI*_transform.det());
}

Gaussian Gaussian::transformedBy(Affine const & t) const {
    return Gaussian(_transform.then(t), _flux);
}

void Gaussian::addTo(Image<float> & image) const {
    auto func = [this](Index2 const & xy, float & pixel) {
        pixel = (*this)(Real2(xy));
    };
    apply(image, func);
}

void Gaussian::format(detail::Writer & writer, detail::FormatSpec const & spec) const {
    writer.write(
        "Gaussian({0}, {1})",
        _transform,
        detail::formatScalar(_flux)
    );
}

} // namespace cipells
