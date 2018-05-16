#include "pybind11/pybind11.h"
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

