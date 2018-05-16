#ifndef CIPELLS_profiles_h_INCLUDED
#define CIPELLS_profiles_h_INCLUDED

#include "cipells/fwd/Image.h"
#include "cipells/transforms.h"
#include "cipells/formatting.h"

namespace cipells {

class Gaussian : public detail::Formattable<Gaussian> {
public:

    explicit Gaussian(Affine const & transform, double flux=1.0);

    double operator()(Real2 const & xy) const;

    double operator()(Real x, Real y) const { return (*this)(Real2(x, y)); }

    Gaussian transformedBy(Affine const & t) const;

    void addTo(Image<float> & image) const;

    void format(detail::Writer & writer, detail::FormatSpec const & spec) const;

private:
    Affine _transform;
    Affine _inv_transform;
    double _flux;
};

} // namespace cipells

#endif // ! CIPELLS_profiles_h_INCLUDED
