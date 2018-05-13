#define CIPELLS_Kernel_cc_SRC

#include <cmath>

#include "cipells/Kernel.h"

namespace cipells {

namespace {

void checkKernelDimensions(IndexBox const & bbox) {
    if (bbox.width() % 2 != 1 || bbox.height() % 2 != 1) {
        throw std::invalid_argument("Kernel width and height must be odd.");
    }
    if (-bbox.min() != bbox.max()) {
        throw std::invalid_argument("Kernel image must be centered on zero.");
    }
}

} // anonymous

Kernel::Kernel(Image<float const> && image, Index upsampling,
               std::shared_ptr<Interpolant const> interpolant) :
    _image(std::move(image).freeze()),
    _upsampling(upsampling),
    _interpolant(interpolant ? std::move(interpolant) : Interpolant::default_())
{
    checkKernelDimensions(_image.bbox());
}

Kernel::Kernel(Image<float const> const & image, Index upsampling,
               std::shared_ptr<Interpolant const> interpolant) :
    _image(image.copy()),
    _upsampling(upsampling),
    _interpolant(interpolant ? std::move(interpolant) : Interpolant::default_())
{
    checkKernelDimensions(_image.bbox());
}

Kernel Kernel::resample(Index upsampling, std::shared_ptr<Interpolant const> interpolant) const {
    if (interpolant == nullptr) {
        interpolant = _interpolant;
    }
    if (upsampling == _upsampling) {
        return Kernel(_image, _upsampling, interpolant);
    }
    Index2 half = _image.bbox().max()*upsampling/_upsampling;
    auto bbox = IndexBox::fromMinMax(-half, half);
    auto transform = Jacobian::makeScaling(Real(upsampling)/Real(_upsampling));
    Image<float> output(bbox);
    _interpolant->warp(_image, transform, output);
    return Kernel(std::move(output), upsampling, std::move(interpolant));
}

Kernel Kernel::warp(Affine const & transform, IndexBox const & bbox, Index upsampling,
                    std::shared_ptr<Interpolant const> interpolant) const {
    checkKernelDimensions(bbox);
    if (interpolant == nullptr) {
        interpolant = _interpolant;
    }
    Affine fullTransform = Jacobian::makeScaling(1.0/_upsampling)
        .then(transform)
        .then(Jacobian::makeScaling(upsampling));
    Image<float> output(bbox);
    _interpolant->warp(_image, fullTransform, output);
    return Kernel(std::move(output), upsampling, std::move(interpolant));
}

void Kernel::convolve(
    Image<float const> const & input,
    Affine const & transform,
    Image<float> const & output
) const {
    _interpolant->convolve(input, _image, _upsampling, transform, output, false);
}

Image<float> Kernel::convolve(Image<float const> const & input, Affine const & transform) const {
    Image<float> output(IndexBox(transform(RealBox(input.bbox()))));
    convolve(input, transform, output);
    return output;
}

void Kernel::correlate(
    Image<float const> const & input,
    Affine const & transform,
    Image<float> const & output
) const {
    _interpolant->convolve(input, _image, _upsampling, transform, output, true);
}

Image<float> Kernel::correlate(Image<float const> const & input, Affine const & transform) const {
    Image<float> output(IndexBox(transform(RealBox(input.bbox()))));
    correlate(input, transform, output);
    return output;
}


} // namespace cipells
