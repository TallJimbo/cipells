
#include "pybind11/pybind11.h"
#include "pybind11/numpy.h"
#include <cmath>
#include "Eigen/Core"
#include "Eigen/LU"

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


class TruncatedGaussian {
public:

    explicit TruncatedGaussian(AffineTransform const & transform, double flux=1.0,
                               double s1=1.0, double s2=3.0, int n=4) :
        _transform(transform),
        _inv_transform(transform.get_inverse()),
        _flux(flux),
        _s1(s1),
        _s2(s2),
        _poly(fitPoly(s1, s2, n)),
        _norm(1.0/std::sqrt(2*M_PI))
    {
        double integral = std::erf(_s1/std::sqrt(2));
        double p1 = s1;
        double p2 = s2;
        for (int i = 0; i < n; ++i) {
            integral += 2*_poly[i]*(p2 - p1)/(i + 1);
            p1 *= s1;
            p2 *= s2;
        }
        integral *= transform.get_scaling();
        _norm /= integral;
        _poly /= integral;
    }

    explicit TruncatedGaussian(double scaling=1.0, double offset=0.0, double flux=1.0,
                               double s1=1.0, double s2=3.0, int n=4) :
        TruncatedGaussian(AffineTransform(scaling, offset), flux, s1, s2, n)
    {}

    double operator()(double x) const {
        double t = std::abs(_inv_transform(x));
        if (t >= _s2) {
            return 0.0;
        }
        if (t >= _s1) {
            double r = 0.0;
            double p = 1.0;
            for (int i = 0; i < _poly.size(); ++i) {
                r += p*_poly[i];
                p *= t;
            }
            return r;
        }
        return std::exp(-0.5*t*t)*_norm;
    }

    double get_scaling() const { return _transform.get_scaling(); }
    void set_scaling(double scaling) { _transform.set_scaling(scaling); }

    double get_offset() const { return _transform.get_offset(); }
    void set_offset(double offset) { _transform.set_offset(offset); }

    double get_flux() const { return _flux; }
    void set_flux(double flux) { _flux = flux; }

    double get_s1() const { return _s1; }

    double get_s2() const { return _s2; }

    int get_n() const { return _poly.size(); }

    TruncatedGaussian transformedBy(AffineTransform const & t) const {
        return TruncatedGaussian(_transform.then(t), _flux, _s1, _s2, _poly.size());
    }

private:

    static Eigen::VectorXd fitPoly(double s1, double s2, int n) {
        if (n % 2 != 0 || n < 4) {
            throw std::invalid_argument("n must be a multiple of 2 and >= 4.");
        }
        int const h = n/2;
        double s1e = std::exp(-0.5*s1*s1)/std::sqrt(2*M_PI);
        Eigen::MatrixXd A = Eigen::MatrixXd::Zero(n, n);
        Eigen::VectorXd b = Eigen::VectorXd::Zero(n);
        // Rows corresponding to value constraints:
        // equal to Gaussian at s1, zero at s2.
        double p1 = 1.0;
        double p2 = 1.0;
        for (int j = 0; j < n; ++j) {
            A(0, j) = p1;
            A(h, j) = p2;
            p1 *= s1;
            p2 *= s2;
        }
        b[0] = s1e;
        // b[h] == 0.0

        // Rows corresponding to derivative constraints:
        // again equal to Gaussian at s1, zero at s2.
        for (int i = 1; i < h; ++i) {
            for (int j = i; j < n; ++j) {
                int r = 1 + j - i;
                A(i,     j) = r*A(i - 1,     j)/s1;
                A(i + h, j) = r*A(i + h - 1, j)/s2;
            }

        }
        b[1] = -s1*s1e;
        for (int i = 2; i < h; ++i) {
            b[i] = -s1*b[i - 1] - (i - 1)*b[i - 2];
        }
        // b[h:] === 0.0

        return A.partialPivLu().solve(b);
    }

    AffineTransform _transform;
    AffineTransform _inv_transform;
    double _flux;
    double _s1;
    double _s2;
    double _norm;
    Eigen::VectorXd _poly;
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

    helper.add(
        py::class_<TruncatedGaussian>(m, "TruncatedGaussian"),
        [](auto & cls) {
            cls.def(py::init<double, double, double, double, double, int>(),
                    "scaling"_a=1.0, "offset"_a=0.0, "flux"_a=1.0, "s1"_a=1.0, "s2"_a=3.0, "n"_a=4);
            cls.def("__call__", py::vectorize(&TruncatedGaussian::operator()));
            cls.def("transformedBy", &TruncatedGaussian::transformedBy);
            cls.def_property("scaling", &TruncatedGaussian::get_scaling, &TruncatedGaussian::set_scaling);
            cls.def_property("offset", &TruncatedGaussian::get_offset, &TruncatedGaussian::set_offset);
            cls.def_property("flux", &TruncatedGaussian::get_flux, &TruncatedGaussian::set_flux);
            cls.def_property_readonly("s1", &TruncatedGaussian::get_s1);
            cls.def_property_readonly("s2", &TruncatedGaussian::get_s2);
            cls.def_property_readonly("n", &TruncatedGaussian::get_n);
            cls.def(
                "__repr__",
                [](TruncatedGaussian const & self) {
                    return py::str(
                        "TruncatedGaussian(scaling={}, offset={}, flux={}, s1={}, s2={}, n={})"
                    ).format(
                        self.get_scaling(), self.get_offset(), self.get_flux(),
                        self.get_s1(), self.get_s2(), self.get_n()
                    );
                }
            );
        }
    );

}


}} // namespace cipells::experiments
