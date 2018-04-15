#include "pybind11/stl.h"

#include "cipells/python.h"
#include "cipells/Box.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace cipells {

namespace {

template <typename T, typename Derived, typename ...Args>
void wrapBox(py::class_<Derived, Args...> & cls) {
    cls.def_static("makeEmpty", &Derived::makeEmpty);
    cls.def_static(
        "makeHull",
        [](std::vector<XYTuple<T>> const & container) {
            return Derived::makeHull(container);
        }
    );
    cls.def(py::init<>());
    cls.def(
        py::init(
            [](py::kwargs kw) -> Derived {
                if (kw.size() != 2) {
                    PyErr_SetString(PyExc_TypeError, "Constructor requires exactly 2 arguments.");
                    throw py::error_already_set();
                }
                if (kw.contains("x") && kw.contains("y")) {
                    return Derived(py::cast<Interval<T>>(kw["x"]), py::cast<Interval<T>>(kw["y"]));
                }
                if (kw.contains("min")) {
                    if (kw.contains("max")) {
                        return Derived::fromMinMax(py::cast<XYTuple<T>>(kw["min"]),
                                                   py::cast<XYTuple<T>>(kw["max"]));
                    }
                    if (kw.contains("size")) {
                        return Derived::fromMinSize(py::cast<XYTuple<T>>(kw["min"]),
                                                    py::cast<XYTuple<T>>(kw["size"]));
                    }
                }
                if (kw.contains("max") && kw.contains("size")) {
                    return Derived::fromMaxSize(py::cast<XYTuple<T>>(kw["max"]),
                                                py::cast<XYTuple<T>>(kw["size"]));
                }
                if (kw.contains("center") && kw.contains("size")) {
                    return Derived::fromCenterSize(py::cast<XYTuple<T>>(kw["center"]),
                                                   py::cast<XYTuple<T>>(kw["size"]));
                }
                PyErr_SetString(PyExc_TypeError, "Incompatible constructor arguments.");
                throw py::error_already_set();
            }
        )
    );
    cls.def_property_readonly("x", &Derived::x, py::return_value_policy::copy);
    cls.def_property_readonly("y", &Derived::y, py::return_value_policy::copy);
    cls.def_property_readonly("min", &Derived::min);
    cls.def_property_readonly("max", &Derived::max);
    cls.def_property_readonly("center", &Derived::center);
    cls.def_property_readonly("size", &Derived::size);
    cls.def_property_readonly("corners", &Derived::corners);
    cls.def_property_readonly("x0", &Derived::x0);
    cls.def_property_readonly("x1", &Derived::x1);
    cls.def_property_readonly("y0", &Derived::y0);
    cls.def_property_readonly("y1", &Derived::y1);
    cls.def_property_readonly("width", &Derived::width);
    cls.def_property_readonly("height", &Derived::height);
    cls.def_property_readonly("area", &Derived::area);
    cls.def("isEmpty", &Derived::isEmpty);
    cls.def("contains", py::overload_cast<Derived const &>(&Derived::contains, py::const_));
    cls.def("contains", py::overload_cast<XYTuple<T> const &>(&Derived::contains, py::const_));
    cls.def("intersects", &Derived::intersects);
    cls.def("__eq__", &Derived::operator==, py::is_operator());
    cls.def("__ne__", &Derived::operator!=, py::is_operator());
    cls.def("clippedTo", &Derived::clippedTo);
    cls.def("expandedTo", py::overload_cast<Derived const &>(&Derived::expandedTo, py::const_));
    cls.def("expandedTo", py::overload_cast<XYTuple<T> const &>(&Derived::expandedTo, py::const_));
    cls.def("dilatedBy", py::overload_cast<T>(&Derived::dilatedBy, py::const_));
    cls.def("dilatedBy", py::overload_cast<XYTuple<T> const &>(&Derived::dilatedBy, py::const_));
    cls.def("erodedBy", py::overload_cast<T>(&Derived::erodedBy, py::const_));
    cls.def("erodedBy", py::overload_cast<XYTuple<T> const &>(&Derived::erodedBy, py::const_));
    pyFormattable(cls);
}

} // anonymous


utils::Deferrer pyBox(py::module & module) {
    utils::Deferrer helper;
    helper.add(
        py::class_<IndexBox>(module, "IndexBox"),
        [](auto & cls) {
            wrapBox<Index>(cls);
            cls.def(py::init<RealBox>());
        }
    );
    helper.add(
        py::class_<RealBox>(module, "RealBox"),
        [](auto & cls) {
            wrapBox<Real>(cls);
            cls.def(py::init<IndexBox>());
            cls.def_static("makeUniverse", &RealBox::makeUniverse);
            py::implicitly_convertible<IndexBox, RealBox>();
        }
    );
    return helper;
}

} // namespace cipells

