
#include "pybind11/pybind11.h"
#include "pybind11/numpy.h"
#include <cmath>

#include "cipells/utils/Deferrer.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace cipells { namespace experiments {


class AffineTransform {
public:

    AffineTransform(double scaling_=1.0, double offset_=0.0) : scaling(scaling_), offset(offset_) {}

    double operator()(double x) const { return scaling*x + offset; }

    AffineTransform inverse() const { return AffineTransform(1.0/scaling, -offset/scaling); }

    AffineTransform then(AffineTransform const & next) const {
        return AffineTransform(scaling*next.scaling, next.scaling*offset + next.offset);
    }

    double scaling;
    double offset;
};


class Pixel {
public:

    explicit Pixel(double scaling_=1.0, double offset_=0.0, double flux_=1.0) :
        scaling(scaling_), offset(offset_), flux(flux_)
    {}

    double operator()(double x) const {
        return std::abs((x - offset)/scaling) > 0.5 ? 0.0 : flux/scaling;
    }

    double scaling;
    double offset;
    double flux;
};


class Gaussian {
public:

    explicit Gaussian(double scaling_=1.0, double offset_=0.0, double flux_=1.0) :
        scaling(scaling_), offset(offset_), flux(flux_)
    {}

    double operator()(double x) const {
        double t = (x - offset)/scaling;
        return flux*std::exp(-0.5*t*t)/(std::sqrt(2*M_PI)*scaling);
    }

    double scaling;
    double offset;
    double flux;
};


class GaussianPixelConvolution {
public:

    explicit GaussianPixelConvolution(Gaussian const & gaussian, Pixel const & pixel) :
        _gaussian(gaussian), _pixel(pixel)
    {}

    double operator()(double x) const {
        x -= _pixel.offset + _gaussian.offset;
        double const d = 2*std::sqrt(2)*_gaussian.scaling;
        return 0.5*_gaussian.flux*_pixel.flux*(
            std::erf((2*x + _pixel.scaling)/d) + std::erf((2*x - _pixel.scaling)/d)
        )/_pixel.scaling;
    }

private:
    Gaussian _gaussian;
    Pixel _pixel;
};


PYBIND11_MODULE(continuous1d, m) {
    utils::Deferrer helper;

    helper.add(
        py::class_<AffineTransform>(m, "AffineTransform"),
        [](auto & cls) {
            cls.def(py::init<double, double>(), "scaling"_a=1.0, "offset"_a=0.0);
            cls.def("__call__", py::vectorize(&AffineTransform::operator()));
            cls.def_property_readonly("inverse", &AffineTransform::inverse);
            cls.def("then", &AffineTransform::then);
            cls.def_readwrite("scaling", &AffineTransform::scaling);
            cls.def_readwrite("offset", &AffineTransform::offset);
        }
    );

    helper.add(
        py::class_<Gaussian>(m, "Gaussian"),
        [](auto & cls) {
            cls.def(py::init<double, double, double>(), "scaling"_a=1.0, "offset"_a=0.0, "flux"_a=1.0);
            cls.def("__call__", py::vectorize(&Gaussian::operator()));
            cls.def_readwrite("scaling", &Gaussian::scaling);
            cls.def_readwrite("offset", &Gaussian::offset);
            cls.def_readwrite("flux", &Gaussian::flux);
        }
    );

    helper.add(
        py::class_<Pixel>(m, "Pixel"),
        [](auto & cls) {
            cls.def(py::init<double, double, double>(), "scaling"_a=1.0, "offset"_a=0.0, "flux"_a=1.0);
            cls.def("__call__", py::vectorize(&Pixel::operator()));
            cls.def_readwrite("scaling", &Pixel::scaling);
            cls.def_readwrite("offset", &Pixel::offset);
            cls.def_readwrite("flux", &Pixel::flux);
        }
    );

    helper.add(
        py::class_<GaussianPixelConvolution>(m, "GaussianPixelConvolution"),
        [](auto & cls) {
            cls.def(py::init<Gaussian const &, Pixel const &>(), "gaussian"_a, "pixel"_a);
            cls.def("__call__", py::vectorize(&GaussianPixelConvolution::operator()));
        }
    );

}


}} // namespace cipells::experiments
