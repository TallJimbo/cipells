#define CIPELLS_Interpolant_cc_SRC

#include <cmath>
#include <limits>

#include "cipells/Interpolant.h"

namespace cipells {

namespace {

template <typename Derived>
class InterpolantImpl : public Interpolant {
public:

    using Array = Eigen::Array<float, Eigen::Dynamic, 1>;

    explicit InterpolantImpl(Real radius) :
        _radius(2*radius)
    {}

    void fill(float x, IndexInterval const & interval, float * output) const {
        for (Index i = interval.min(); i <= interval.max(); ++i, ++output) {
            *output = static_cast<Derived const *>(this)->evaluate(x - i);
        }
    }

    Index computeArraySize(Index input_size) const {
        return static_cast<Index>(std::min(2*std::ceil(_radius) + 1, static_cast<Real>(input_size)));
    }

    double operator()(double x) const override {
        static_cast<Derived const *>(this)->evaluate(x);
    }

    void convolve(
        Image<float const> const & input,
        Image<float const> const & kernel,
        Index upsampling,
        Affine const & transform,
        Image<float> const & output,
        bool transpose
    ) const override {
        throw std::logic_error("Not implemented.");
    }

    void warp(
        Image<float const> const & input,
        Affine const & transform,
        Image<float> const & output
    ) const override {
        Array kx(computeArraySize(input.bbox().width()));
        Array ky(computeArraySize(input.bbox().height()));
        auto func = [&input, &transform, &kx, &ky, this](Index2 const & out_index, float & out_pixel) {
            Real2 in_pos = transform(Real2(out_index));
            IndexBox box(RealBox::fromCenterSize(in_pos, Real2(2*_radius, 2*_radius)));
            box.clipTo(input.bbox());
            assert(box.x().size() <= kx.size());
            assert(box.y().size() <= ky.size());
            fill(in_pos.x(), box.x(), &kx.coeffRef(0));
            fill(in_pos.y(), box.y(), &ky.coeffRef(0));
            out_pixel = (
                input.array(box) *
                (
                    ky.head(box.y().size()).matrix() *
                    kx.head(box.x().size()).matrix().transpose()
                ).array()
            ).sum();
        };
        apply(output, func);
    }

private:
    Real _radius;
};


class SincInterpolant : public InterpolantImpl<SincInterpolant> {
public:

    SincInterpolant() : InterpolantImpl<SincInterpolant>(std::numeric_limits<float>::infinity()) {}

    double evaluate(double x) const {
        double y = x*M_PI;
        return x == 0.0 ? 1.0 : std::sin(y)/y;
    }

};


// Sinc-approximating cubic interpolant from Bernstein & Gruen 2014, Equation A8.
class CubicInterpolant : public InterpolantImpl<CubicInterpolant> {
public:

    CubicInterpolant() : InterpolantImpl<CubicInterpolant>(2.0) {}

    double evaluate(double x) const {
        double y = std::fabs(x);
        if (y >= 2.0) {
            return 0.0;
        }
        if (y >= 1.0) {
            return ((-0.5*y + 2.5)*y - 4.0)*y + 2.0;
        }
        return ((1.5*y - 2.5)*y)*y + 1.0;
    }

};


// Sinc-approximating quintic interpolant from Bernstein & Gruen 2014, Equation A9.
class QuinticInterpolant : public InterpolantImpl<QuinticInterpolant> {
public:

    QuinticInterpolant() : InterpolantImpl<QuinticInterpolant>(3.0) {}

    double evaluate(double x) const {
        double y = std::fabs(x);
        if (y >= 3.0) {
            return 0.0;
        }
        if (y >= 2.0) {
            return (y - 2.0)*(y - 3.0)*(y - 3.0)*((-11.0*y + 50.0)*y - 54.0)/24.0;
        }
        if (y >= 1.0) {
            return (y - 1.0)*(y - 2.0)*(((55.0*y + -249.0)*y + 348.0)*y - 138.0)/24.0;
        }
        return ((-55.0*y + 138.0)*y - 95.0)*y*y*y/12.0 + 1.0;
    }

};

} // anonymous


std::shared_ptr<Interpolant const> Interpolant::default_() {
    return quintic();
}

std::shared_ptr<Interpolant const> Interpolant::sinc() {
    static std::shared_ptr<SincInterpolant const> instance = std::make_shared<SincInterpolant>();
    return instance;
}

std::shared_ptr<Interpolant const> Interpolant::cubic() {
    static std::shared_ptr<CubicInterpolant const> instance = std::make_shared<CubicInterpolant>();
    return instance;
}

std::shared_ptr<Interpolant const> Interpolant::quintic() {
    static std::shared_ptr<QuinticInterpolant const> instance = std::make_shared<QuinticInterpolant>();
    return instance;
}

} // namespace cipells
