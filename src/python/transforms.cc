#include "pybind11/pybind11.h"
#include "pybind11/numpy.h"
#include "pybind11/eigen.h"

#include "cipells/python.h"
#include "cipells/transforms.h"
#include "cipells/Box.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace cipells {

namespace {

template <typename T, typename ...Args>
void wrapTransform(py::class_<T, Args...> & cls) {
    cls.def(py::init<>());
    cls.def("__call__", py::overload_cast<Real2 const &>(&T::operator(), py::const_));
    cls.def("__call__", py::overload_cast<RealBox const &>(&T::operator(), py::const_));
    cls.def(
        "__call__",
        [](T const & self, Real x, Real y) { return self(Real2(x, y)); },
        "x"_a, "y"_a
    );
    cls.def(
        "__call__",
        [](T const & self, py::array_t<Real> x, py::array_t<Real> y) {
            ssize_t nd = 0;
            std::vector<ssize_t> shape(0);
            std::array<py::buffer_info, 2> buffers({x.request(), y.request()});
            py::detail::broadcast(buffers, nd, shape);
            py::array_t<Real> x_result = py::array_t<Real>(shape);
            py::array_t<Real> y_result = py::array_t<Real>(shape);
            py::buffer_info x_out = x_result.request(true);
            py::buffer_info y_out = y_result.request(true);
            py::detail::multi_array_iterator<2> input_iter(buffers, shape);
            py::detail::array_iterator<Real> const x_end = py::detail::array_end<Real>(x_out);
            for (
                 py::detail::array_iterator<Real> x_iter = py::detail::array_begin<Real>(x_out),
                                                  y_iter = py::detail::array_begin<Real>(y_out);
                 x_iter != x_end;
                 ++x_iter, ++y_iter, ++input_iter
            ) {
                Real2 xy = self(Real2(*input_iter.data<0, Real>(), *input_iter.data<1, Real>()));
                *x_iter = xy.x();
                *y_iter = xy.y();
            }
            return std::make_pair(x_result, y_result);
        },
        "x"_a, "y"_a
    );
    cls.def("inverted", &T::inverted);
    cls.def("then", py::overload_cast<Identity const &>(&T::then, py::const_));
    cls.def("then", py::overload_cast<Translation const &>(&T::then, py::const_));
    cls.def("then", py::overload_cast<Jacobian const &>(&T::then, py::const_));
    cls.def("then", py::overload_cast<Affine const &>(&T::then, py::const_));
    cls.def_property_readonly("det", &T::det);
    cls.def_property_readonly(
        "matrix",
        [](T const & self) -> Matrix2<Real> const {
            return self.matrix();
        }
    );
    cls.def_property_readonly(
        "vector",
        [](T const & self) -> Vector2<Real> const {
            return self.vector();
        }
    );
    pyFormattable(cls);
}

} // anonymous

utils::Deferrer pyTransforms(pybind11::module & module) {
    utils::Deferrer helper;
    helper.add(
        py::class_<Identity>(module, "Identity"),
        [](auto & cls) {
            wrapTransform(cls);
        }
    );
    helper.add(
        py::class_<Translation>(module, "Translation"),
        [](auto & cls) {
            cls.def(py::init<Identity const &>());
            cls.def(py::init<Real2 const &>());
            cls.def(py::init<Eigen::Ref<Vector2<Real> const> const &>());
            wrapTransform(cls);
            py::implicitly_convertible<Identity, Translation>();
        }
    );
    helper.add(
        py::class_<Jacobian>(module, "Jacobian"),
        [](auto & cls) {
            cls.def(py::init<Identity const &>());
            cls.def(py::init<Eigen::Ref<Matrix2<Real> const> const &>());
            wrapTransform(cls);
            py::implicitly_convertible<Identity, Jacobian>();
        }
    );
    helper.add(
        py::class_<Affine>(module, "Affine"),
        [](auto & cls) {
            cls.def(py::init<Identity const &>());
            cls.def(py::init<Eigen::Ref<Matrix2<Real> const> const &, Eigen::Ref<Vector2<Real> const> const &>());
            cls.def(py::init<Jacobian const &, Translation const &>(), "jacobian"_a, "translation"_a);
            cls.def(py::init<Jacobian const &>(), "jacobian"_a);
            cls.def(py::init<Translation const &>(), "translation"_a);
            cls.def_property_readonly("translation", &Affine::translation, py::return_value_policy::copy);
            cls.def_property_readonly("jacobian", &Affine::jacobian, py::return_value_policy::copy);
            wrapTransform(cls);
            py::implicitly_convertible<Translation, Affine>();
            py::implicitly_convertible<Jacobian, Affine>();
            py::implicitly_convertible<Identity, Affine>();
        }
    );
    return helper;
}

} // namespace cipells

