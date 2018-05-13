#include "pybind11/pybind11.h"
#include "pybind11/numpy.h"

#include "cipells/python.h"
#include "cipells/Interpolant.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace cipells {

utils::Deferrer pyInterpolant(py::module & module) {
    utils::Deferrer helper;
    helper.add(
        py::class_<Interpolant, std::shared_ptr<Interpolant>>(module, "Interpolant"),
        [](auto & cls) {
            cls.def_property_readonly_static("default", [](py::object) { return Interpolant::default_(); });
            cls.def_property_readonly_static("sinc", [](py::object) { return Interpolant::sinc(); });
            cls.def_property_readonly_static("cubic", [](py::object) { return Interpolant::cubic(); });
            cls.def_property_readonly_static("quintic", [](py::object) { return Interpolant::quintic(); });
            cls.def("__call__", py::vectorize(&Interpolant::operator()));
        }
    );
    return helper;
}

} // namespace cipells

