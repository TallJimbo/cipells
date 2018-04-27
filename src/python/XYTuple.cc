#include "pybind11/pybind11.h"
#include "pybind11/eigen.h"

#include "cipells/python.h"
#include "cipells/XYTuple.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace cipells {

namespace {

template <typename T, typename ...Args>
void wrapXYTuple(py::class_<XYTuple<T>, Args...> & cls) {
    cls.def(py::init<T, T>(), "x"_a, "y"_a);
    cls.def(py::init<Eigen::Ref<Vector2<T> const> const &>(), "vector"_a);
    cls.def_property_readonly("x", [](XYTuple<T> const & self) -> T { return self.x(); });
    cls.def_property_readonly("y", [](XYTuple<T> const & self) -> T { return self.y(); });
    cls.def(
        "__getitem__",
        [](XYTuple<T> const & self, int i) -> T {
            if (i < 0) {
                i = 2 + i;
            }
            if (i < 0 || i > 1) {
                PyErr_SetString(PyExc_IndexError, "Index out of range.");
                throw py::error_already_set();
            }
            return self[i];
        }
    );
    cls.def("__len__", [](py::object) { return 2; });
    cls.def(
        "__pos__",
        [](py::object self) {
            return self;
        },
        py::is_operator()
    );
    cls.def(
        "__neg__",
        [](XYTuple<T> const & self) {
            return -self;
        },
        py::is_operator()
    );
    cls.def(
        "__mul__",
        [](XYTuple<T> const & lhs, T rhs) {
            return lhs * rhs;
        },
        py::is_operator()
    );
    cls.attr("__rmul__") = cls.attr("__mul__");
    cls.def(
        "__add__",
        [](XYTuple<T> const & lhs, XYTuple<T> const & rhs) {
            return lhs + rhs;
        },
        py::is_operator()
    );
    cls.attr("__radd__") = cls.attr("__add__");
    cls.def(
        "__sub__",
        [](XYTuple<T> const & lhs, XYTuple<T> const & rhs) {
            return lhs - rhs;
        },
        py::is_operator()
    );
    cls.attr("__radd__") = cls.attr("__sub__");
    cls.def("__eq__", &XYTuple<T>::operator==, py::is_operator());
    cls.def("__ne__", &XYTuple<T>::operator!=, py::is_operator());
    cls.def("dot", &XYTuple<T>::dot);
    cls.def_property_readonly(
        "vector",
        [](XYTuple<T> const & self) -> Vector2<T> const {
            return self.vector();
        }
    );
    pyFormattable(cls);
}

template <typename T>
XYTuple<T> * loadFromPyTuple(py::tuple const & t) {
    if (t.size() != 2u) {
        PyErr_SetString(PyExc_ValueError, "Only 2-element tuples are interoperable with XYTuple.");
        throw py::error_already_set();
    }
    T x = py::cast<T>(t[0]);
    T y = py::cast<T>(t[1]);
    return new XYTuple<T>(x, y);
}


Index pyFloorDiv(Index lhs, Index rhs) {
    Index r = lhs/rhs;
    if (lhs % rhs != 0 && r < 0) --r;
    return r;
}


Index pyModulo(Index lhs, Index rhs) {
    return ((lhs % rhs) + rhs) % rhs;
}


} // anonymous


utils::Deferrer pyXYTuple(py::module & module) {
    utils::Deferrer helper;
    helper.add(
        py::class_<Index2>(module, "Index2"),
        [](auto & cls) {
            wrapXYTuple(cls);
            cls.def(
                "__floordiv__",
                [](Index2 const & lhs, Index rhs) {
                    return Index2(pyFloorDiv(lhs.x(), rhs), pyFloorDiv(lhs.y(), rhs));
                },
                py::is_operator()
            );
            cls.def(
                "__mod__",
                [](Index2 const & lhs, Index rhs) {
                    return Index2(pyModulo(lhs.x(), rhs), pyModulo(lhs.y(), rhs));
                },
                py::is_operator()
            );
        }
    );
    helper.add(
        py::class_<Real2>(module, "Real2"),
        [](auto & cls) {
            wrapXYTuple(cls);
            cls.def(
                "__truediv__",
                [](Real2 const & lhs, Real rhs) {
                    return lhs / rhs;
                },
                py::is_operator()
            );
        }
    );
    return helper;
}

} // namespace cipells



namespace pybind11 { namespace detail {

bool type_caster<cipells::Index2>::load(handle src, bool convert) {
    if (base::load(src, convert)) {
        return true;
    }
    if (py::isinstance<tuple>(src)) {
        value = cipells::loadFromPyTuple<cipells::Index>(reinterpret_borrow<tuple>(src));
        return true;
    }
    return false;
}


bool type_caster<cipells::Real2>::load(handle src, bool convert) {
    if (base::load(src, convert)) {
        return true;
    }
    if (py::isinstance<tuple>(src)) {
        value = cipells::loadFromPyTuple<cipells::Real>(reinterpret_borrow<tuple>(src));
        return true;
    }
    return false;
}

}} // namespace pybind11::detail

