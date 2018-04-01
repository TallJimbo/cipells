
#include "pybind11/pybind11.h"
#include "pybind11/numpy.h"
#include <cmath>

#include "cipells/utils/Deferrer.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace cipells { namespace experiments {


class AffineTransform {
public:

    AffineTransform(double scaling=1.0, double offset=0.0) : _scaling(scaling), _offset(offset) {}

    double operator()(double x) const { return get_scaling()*x + get_offset(); }

    AffineTransform get_inverse() const {
        return AffineTransform(1.0/get_scaling(), -get_offset()/get_scaling());
    }

    AffineTransform then(AffineTransform const & next) const {
        return AffineTransform(get_scaling()*next.get_scaling(),
                               next.get_scaling()*get_offset() + next.get_offset());
    }

    double get_scaling() const { return _scaling; }
    void set_scaling(double scaling) { _scaling = scaling; }

    double get_offset() const { return _offset; }
    void set_offset(double offset) { _offset = offset; }

private:
    double _scaling;
    double _offset;
};


class Pixel {
public:

    explicit Pixel(AffineTransform const & transform, double flux=1.0) :
        _transform(transform),
        _inv_transform(transform.get_inverse()),
        _flux(flux)
    {}

    explicit Pixel(double scaling=1.0, double offset=0.0, double flux=1.0) :
        Pixel(AffineTransform(scaling, offset), flux)
    {}

    double operator()(double x) const {
        return std::abs(_inv_transform(x)) > 0.5 ? 0.0 : _flux/get_scaling();
    }

    double get_scaling() const { return _transform.get_scaling(); }
    void set_scaling(double scaling) { _transform.set_scaling(scaling); }

    double get_offset() const { return _transform.get_offset(); }
    void set_offset(double offset) { _transform.set_offset(offset); }

    double get_flux() const { return _flux; }
    void set_flux(double flux) { _flux = flux; }

    Pixel transformedBy(AffineTransform const & t) const {
        return Pixel(_transform.then(t), _flux);
    }

private:
    AffineTransform _transform;
    AffineTransform _inv_transform;
    double _flux;
};


class Gaussian {
public:

    explicit Gaussian(AffineTransform const & transform, double flux=1.0) :
        _transform(transform),
        _inv_transform(transform.get_inverse()),
        _flux(flux)
    {}

    explicit Gaussian(double scaling=1.0, double offset=0.0, double flux=1.0) :
        Gaussian(AffineTransform(scaling, offset), flux)
    {}

    double operator()(double x) const {
        double t = _inv_transform(x);
        return _flux*std::exp(-0.5*t*t)/(std::sqrt(2*M_PI)*get_scaling());
    }

    double get_scaling() const { return _transform.get_scaling(); }
    void set_scaling(double scaling) { _transform.set_scaling(scaling); }

    double get_offset() const { return _transform.get_offset(); }
    void set_offset(double offset) { _transform.set_offset(offset); }

    double get_flux() const { return _flux; }
    void set_flux(double flux) { _flux = flux; }

    Gaussian transformedBy(AffineTransform const & t) const {
        return Gaussian(_transform.then(t), _flux);
    }

private:
    AffineTransform _transform;
    AffineTransform _inv_transform;
    double _flux;
};


class GaussianPixelConvolution {
public:

    explicit GaussianPixelConvolution(Gaussian const & gaussian, Pixel const & pixel) :
        _gaussian(gaussian), _pixel(pixel)
    {}

    double operator()(double x) const {
        x -= _pixel.get_offset() + _gaussian.get_offset();
        double const d = 2*std::sqrt(2)*_gaussian.get_scaling();
        return 0.5*_gaussian.get_flux()*_pixel.get_flux()*(
            std::erf((2*x + _pixel.get_scaling())/d) - std::erf((2*x - _pixel.get_scaling())/d)
        )/_pixel.get_scaling();
    }

    GaussianPixelConvolution transformedBy(AffineTransform const & t) const {
        return GaussianPixelConvolution(_gaussian.transformedBy(t), _pixel.transformedBy(t));
    }

    Gaussian get_gaussian() const { return _gaussian; }
    Pixel get_pixel() const { return _pixel; }

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
            cls.def_property_readonly("inverse", &AffineTransform::get_inverse);
            cls.def("then", &AffineTransform::then);
            cls.def_property("scaling", &AffineTransform::get_scaling, &AffineTransform::set_scaling);
            cls.def_property("offset", &AffineTransform::get_offset, &AffineTransform::set_offset);
            cls.def(
                "__repr__",
                [](AffineTransform const & self) {
                    return py::str("AffineTransform(scaling={}, offset={})").format(
                        self.get_scaling(), self.get_offset()
                    );
                }
            );
        }
    );

    helper.add(
        py::class_<Gaussian>(m, "Gaussian"),
        [](auto & cls) {
            cls.def(py::init<double, double, double>(), "scaling"_a=1.0, "offset"_a=0.0, "flux"_a=1.0);
            cls.def("__call__", py::vectorize(&Gaussian::operator()));
            cls.def("transformedBy", &Gaussian::transformedBy);
            cls.def_property("scaling", &Gaussian::get_scaling, &Gaussian::set_scaling);
            cls.def_property("offset", &Gaussian::get_offset, &Gaussian::set_offset);
            cls.def_property("flux", &Gaussian::get_flux, &Gaussian::set_flux);
            cls.def(
                "__repr__",
                [](Gaussian const & self) {
                    return py::str("Gaussian(scaling={}, offset={}, flux={})").format(
                        self.get_scaling(), self.get_offset(), self.get_flux()
                    );
                }
            );
        }
    );

    helper.add(
        py::class_<Pixel>(m, "Pixel"),
        [](auto & cls) {
            cls.def(py::init<double, double, double>(), "scaling"_a=1.0, "offset"_a=0.0, "flux"_a=1.0);
            cls.def("__call__", py::vectorize(&Pixel::operator()));
            cls.def("transformedBy", &Pixel::transformedBy);
            cls.def_property("scaling", &Pixel::get_scaling, &Pixel::set_scaling);
            cls.def_property("offset", &Pixel::get_offset, &Pixel::set_offset);
            cls.def_property("flux", &Pixel::get_flux, &Pixel::set_flux);
            cls.def(
                "__repr__",
                [](Pixel const & self) {
                    return py::str("Pixel(scaling={}, offset={}, flux={})").format(
                        self.get_scaling(), self.get_offset(), self.get_flux()
                    );
                }
            );
        }
    );

    helper.add(
        py::class_<GaussianPixelConvolution>(m, "GaussianPixelConvolution"),
        [](auto & cls) {
            cls.def(py::init<Gaussian const &, Pixel const &>(), "gaussian"_a, "pixel"_a);
            cls.def("__call__", py::vectorize(&GaussianPixelConvolution::operator()));
            cls.def("transformedBy", &GaussianPixelConvolution::transformedBy);
            cls.def_property_readonly("gaussian", &GaussianPixelConvolution::get_gaussian);
            cls.def_property_readonly("pixel", &GaussianPixelConvolution::get_pixel);
        }
    );

}


}} // namespace cipells::experiments
