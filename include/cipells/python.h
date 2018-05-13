#ifndef CIPELLS_python_h_INCLUDED
#define CIPELLS_python_h_INCLUDED

#include "pybind11/pybind11.h"
#include "fmt/format.h"

#include "cipells/python/XYTuple.h"
#include "cipells/python/Image.h"
#include "cipells/utils/Deferrer.h"
#include "cipells/formatting.h"

namespace cipells {

template <typename T, typename ...Args>
void pyFormattable(pybind11::class_<T, Args...> & cls) {
    cls.def("__str__", &T::str);
    cls.def("__repr__", &T::repr);
    cls.def(
        "__format__",
        [](T const & self, pybind11::str uspec) {
            pybind11::bytes bspec(uspec);
            char * begin = nullptr;
            Py_ssize_t length = 0;
            if (PyBytes_AsStringAndSize(bspec.ptr(), &begin, &length) != 0) {
                throw pybind11::error_already_set();
            }
            detail::FormatSpec spec(begin, begin + length);
            fmt::MemoryWriter writer;
            self.format(writer, spec);
            return pybind11::str(writer.data(), writer.size());
        }
    );
}

utils::Deferrer pyXYTuple(pybind11::module & module);

utils::Deferrer pyInterval(pybind11::module & module);

utils::Deferrer pyBox(pybind11::module & module);

utils::Deferrer pyTransforms(pybind11::module & module);

utils::Deferrer pyImage(pybind11::module & module);

utils::Deferrer pyInterpolant(pybind11::module & module);

} // namespace cipells

#endif // !CIPELLS_python_h_INCLUDED
