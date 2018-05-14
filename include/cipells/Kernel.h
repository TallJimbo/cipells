#ifndef CIPELLS_Kernel_h_INCLUDED
#define CIPELLS_Kernel_h_INCLUDED

#include "cipells/Image.h"
#include "cipells/transforms.h"
#include "cipells/Interpolant.h"
#include "cipells/Kernel.h"

namespace cipells {

class Kernel {
public:

    explicit Kernel(Image<float const> && image, Index upsampling=1,
                    std::shared_ptr<Interpolant const> interpolant=nullptr);

    explicit Kernel(Image<float const> const & image, Index upsampling=1,
                    std::shared_ptr<Interpolant const> interpolant=nullptr);

    Image<float const> const & image() const { return _image; }

    Index upsampling() const { return _upsampling; }

    std::shared_ptr<Interpolant const> interpolant() const { return _interpolant; }

    Kernel resample(Index upsampling, std::shared_ptr<Interpolant const> interpolant=nullptr) const;

    Kernel warp(Affine const & transform, IndexBox const & bbox, Index upsampling=1,
                std::shared_ptr<Interpolant const> interpolant=nullptr) const;

    void convolve(
        Image<float const> const & input,
        Affine const & transform,
        Image<float> const & output
    ) const;

    Image<float> convolve(Image<float const> const & input, Affine const & transform) const;

    void correlate(
        Image<float const> const & input,
        Affine const & transform,
        Image<float> const & output
    ) const;

    Image<float> correlate(Image<float const> const & input, Affine const & transform) const;

private:
    Image<float const> _image;
    Index _upsampling;
    std::shared_ptr<Interpolant const> _interpolant;
};


} // namespace cipells


#endif // ! CIPELLS_Kernel_h_INCLUDED
