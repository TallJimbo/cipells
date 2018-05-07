#include "cipells/python.h"
#include "cipells/Image.h"

namespace py = pybind11;

namespace cipells {
namespace {

template <typename T>
Image<T> passImage(Image<T> const & in) {
    return in;
}

template <typename T>
Image<T const> passImageToConst(Image<T> const & in) {
    return in;
}

template <typename T>
Image<T const> passConstImage(Image<T const> const & in) {
    return in;
}

template <typename T>
void wrapImageHelpers(py::module & m) {
    m.def("passImage", &passImage<T>);
    m.def("passImageToConst", &passImageToConst<T>);
    m.def("passConstImage", &passConstImage<T>);
}

} // anonymous
} // namespace cipells

PYBIND11_MODULE(_tests, m) {
    cipells::wrapImageHelpers<float>(m);
    cipells::wrapImageHelpers<std::complex<float>>(m);
}
