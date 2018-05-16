#include "cipells/python.h"

#include "fmt/format.h"

#include "cipells/Image.h"
#include "cipells/transforms.h"

namespace py = pybind11;

namespace cipells {
namespace {

#define CIPELLS_TEST_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            fmt::print(stderr, "Failure at {}:{}: {}\n", __func__, __LINE__, #cond); \
            return false; \
        } \
    } while(false)

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

// Test that we don't copy when freezing when there's only one owner.
template <typename T>
bool testImageFreeze1(Image<T const> const & original) {
    auto copy1 = original.copy();
    T * data = copy1.data();
    auto frozen1 = std::move(copy1).freeze();
    CIPELLS_TEST_ASSERT(copy1.data() == nullptr);
    CIPELLS_TEST_ASSERT(copy1.bbox().isEmpty());
    CIPELLS_TEST_ASSERT(copy1.stride() == 0);
    CIPELLS_TEST_ASSERT(copy1.owner() == nullptr);
    CIPELLS_TEST_ASSERT(frozen1.data() == data);
    CIPELLS_TEST_ASSERT(frozen1.bbox() == original.bbox());
    return true;
}

// Test that we do copy when freezing when there's more than one owner.
template <typename T>
bool testImageFreeze2(Image<T const> const & original) {
    auto copy1 = original.copy();
    auto copy2(copy1);
    auto frozen1 = std::move(copy2).freeze();
    CIPELLS_TEST_ASSERT(copy2.data() == nullptr);
    CIPELLS_TEST_ASSERT(copy2.bbox().isEmpty());
    CIPELLS_TEST_ASSERT(copy2.stride() == 0);
    CIPELLS_TEST_ASSERT(copy2.owner() == nullptr);
    CIPELLS_TEST_ASSERT(frozen1.data() != copy1.data());
    CIPELLS_TEST_ASSERT(frozen1.owner() != copy1.owner());
    CIPELLS_TEST_ASSERT(frozen1.bbox() == original.bbox());
    return true;
}

template <typename T>
void wrapImageHelpers(py::module & m) {
    m.def("passImage", &passImage<T>);
    m.def("passImageToConst", &passImageToConst<T>);
    m.def("passConstImage", &passConstImage<T>);
    m.def("testImageFreeze1", &testImageFreeze1<T>);
    m.def("testImageFreeze2", &testImageFreeze2<T>);
}

bool acceptJacobian(Jacobian const & jacobian) { return true; }
bool acceptTranslation(Translation const & translation) { return true; }
bool acceptAffine(Affine const & affine) { return true; }

} // anonymous
} // namespace cipells

PYBIND11_MODULE(_tests, m) {
    cipells::wrapImageHelpers<float>(m);
    cipells::wrapImageHelpers<std::complex<float>>(m);
    m.def("acceptJacobian", &cipells::acceptJacobian);
    m.def("acceptTranslation", &cipells::acceptTranslation);
    m.def("acceptAffine", &cipells::acceptAffine);
}
