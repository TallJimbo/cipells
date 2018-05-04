#include "cipells/python.h"
#include "cipells/Image.h"

namespace py = pybind11;

namespace cipells {
namespace {

template <typename T>
Image<T> passthroughImage(Image<T> const & in) {
    return in;
}

template <typename T>
Image<T const> passthroughImageToConst(Image<T> const & in) {
    return in;
}

template <typename T>
Image<T const> passthroughConstImage(Image<T const> const & in) {
    return in;
}

template <typename T>
void wrapImageHelpers(py::module & m) {
    m.def("passthroughImage", &passthroughImage<T>);
    m.def("passthroughImageToConst", &passthroughImageToConst<T>);
    m.def("passthroughConstImage", &passthroughConstImage<T>);
}

} // anonymous
} // namespace cipells

PYBIND11_MODULE(_tests, m) {
    cipells::wrapImageHelpers<float>(m);
    cipells::wrapImageHelpers<std::complex<float>>(m);
}
