#ifndef CIPELLS_Interpolant_h_INCLUDED
#define CIPELLS_Interpolant_h_INCLUDED

#include "cipells/Image.h"
#include "cipells/transforms.h"

namespace cipells {

class Interpolant {
public:

    static std::shared_ptr<Interpolant const> default_();

    static std::shared_ptr<Interpolant const> sinc();

    static std::shared_ptr<Interpolant const> cubic();

    static std::shared_ptr<Interpolant const> quintic();

    virtual double operator()(double x) const = 0;

    virtual void convolve(
        Image<float const> const & input,
        Image<float const> const & kernel,
        Index upsampling,
        Affine const & transform,
        Image<float> const & output,
        bool transpose
    ) const = 0;

    virtual void warp(
        Image<float const> const & input,
        Affine const & transform,
        Image<float> const & output
    ) const = 0;

    virtual ~Interpolant() {}

};

} // namespace cipells

#endif // ! CIPELLS_Interpolant_h_INCLUDED
