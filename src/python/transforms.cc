#include "pybind11/pybind11.h"

#include "cipells/python.h"
#include "cipells/transforms.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace cipells {

namespace {

template <typename T, typename ...Args>
void wrapTransform(py::class_<T, Args...> & cls) {
    cls.def("__call__", &T::operator());
    cls.def("inverted", &T::inverted);
    cls.def("then", py::overload_cast<Translation const &>(&T::then, py::const_));
    cls.def("then", py::overload_cast<Jacobian const &>(&T::then, py::const_));
    cls.def("then", py::overload_cast<Affine const &>(&T::then, py::const_));
    cls.def_property_readonly("translation", &T::translation, py::return_value_policy::copy);
    cls.def_property_readonly("jacobian", &T::jacobian, py::return_value_policy::copy);
    cls.def_property_readonly("det", &T::det);
    pyFormattable(cls);
}

} // anonymous

utils::Deferrer pyTransforms(pybind11::module & module) {
    utils::Deferrer helper;
    helper.add(
        py::class_<Translation>(module, "Translation"),
        [](auto & cls) {
            cls.def(py::init<Real, Real>(), "x"_a, "y"_a);
            cls.def(py::init<Real2>(), "offset"_a);
            cls.def_property_readonly("offset", &Translation::offset, py::return_value_policy::copy);
            wrapTransform(cls);
        }
    );
    helper.add(
        py::class_<Jacobian>(module, "Jacobian"),
        [](auto & cls) {
            cls.def(py::init<Real, Real, Real, Real>(), "dudx"_a, "dudy"_a, "dvdx"_a, "dvdy"_a);
            cls.def(py::init<Real2, Real2>(), "du"_a, "dv"_a);
            cls.def_property_readonly("du", &Jacobian::du, py::return_value_policy::copy);
            cls.def_property_readonly("dv", &Jacobian::dv, py::return_value_policy::copy);
            wrapTransform(cls);
        }
    );
    helper.add(
        py::class_<Affine>(module, "Affine"),
        [](auto & cls) {
            cls.def(py::init<Real, Real, Real, Real, Real, Real>(),
                    "dudx"_a=1, "dudy"_a=0, "dvdx"_a=0, "dvdy"_a=1, "x"_a=0, "y"_a=0);
            cls.def(py::init<Jacobian const &, Translation const &>(), "jacobian"_a, "translation"_a);
            cls.def(py::init<Jacobian const &>(), "jacobian"_a);
            cls.def(py::init<Translation const &>(), "translation"_a);
            wrapTransform(cls);
        }
    );
    return helper;
}

} // namespace cipells

