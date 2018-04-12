#include "cipells/python.h"
#include "cipells/Interval.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace cipells {

namespace {

template <typename T, typename Derived, typename ...Args>
void wrapInterval(py::class_<Derived, Args...> & cls) {
    cls.def_static("makeEmpty", &Derived::makeEmpty);
    cls.def_static(
        "makeHull",
        [](py::iterable const & iterable) {
            Derived result;
            for (auto x : iterable) {
                result.expandTo(py::cast<T>(x));
            }
            return result;
        }
    );
    cls.def(py::init<>());
    cls.def(py::init<T, T>(), "min"_a, "max"_a);
    cls.def_property_readonly("min", [](Derived const & self) -> T { return self.min(); });
    cls.def_property_readonly("max", [](Derived const & self) -> T { return self.max(); });
    cls.def("isEmpty", &Derived::isEmpty);
    cls.def("contains", py::overload_cast<Derived const &>(&Derived::contains, py::const_));
    cls.def("contains", py::overload_cast<T>(&Derived::contains, py::const_));
    cls.def("intersects", &Derived::intersects);
    cls.def("__eq__", &Derived::operator==, py::is_operator());
    cls.def("__ne__", &Derived::operator!=, py::is_operator());
    cls.def("clippedTo", &Derived::clippedTo);
    cls.def("expandedTo", py::overload_cast<Derived const &>(&Derived::expandedTo, py::const_));
    cls.def("expandedTo", py::overload_cast<T>(&Derived::expandedTo, py::const_));
    cls.def("dilatedBy", &Derived::dilatedBy);
    cls.def("erodedBy", &Derived::erodedBy);
    cls.def("__str__", &Derived::str);
    cls.def("__repr__", &Derived::repr);
    cls.def_property_readonly("size", &Derived::size);
}

} // anonymous


utils::Deferrer pyInterval(py::module & module) {
    utils::Deferrer helper;
    helper.add(
        py::class_<IndexInterval>(module, "IndexInterval"),
        [](auto & cls) {
            wrapInterval<Index>(cls);
            cls.def(py::init<RealInterval>());
        }
    );
    helper.add(
        py::class_<RealInterval>(module, "RealInterval"),
        [](auto & cls) {
            wrapInterval<Real>(cls);
            cls.def(py::init<IndexInterval>());
            cls.def_static("makeUniverse", &RealInterval::makeUniverse);
            py::implicitly_convertible<IndexInterval, RealInterval>();
        }
    );
    return helper;
}

} // namespace cipells

