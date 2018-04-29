#include "pybind11/pybind11.h"
#include "pybind11/numpy.h"

#include <memory>
#include <type_traits>

#include "cipells/python.h"
#include "cipells/Image.h"
#include "fmt/format.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace cipells {

namespace {

class PyImage {
public:

    static std::unique_ptr<PyImage> construct(IndexBox const & box, py::object dtype_arg) {
        std::unique_ptr<PyImage> result;
        auto dtype = py::dtype::from_args(dtype_arg);
        if (result = tryConstruct<float>(box, dtype)) return result;
        if (result = tryConstruct<std::complex<float>>(box, dtype)) return result;
        PyErr_SetString(PyExc_TypeError, "dtype not supported");
        throw py::error_already_set();
    }

    template <typename T>
    static std::unique_ptr<PyImage> convert(Image<T> && wrapped);

    template <typename T>
    Image<T> * extract();

    virtual py::object getScalar(Index2 const & index) const = 0;

    virtual void setScalar(Index2 const & index, py::object value) const = 0;

    virtual std::unique_ptr<PyImage> getSubImage(IndexBox const & box) const = 0;

    virtual py::array getArray() const = 0;

    virtual void setArray(py::object value) const = 0;

    virtual py::dtype dtype() const = 0;

    virtual IndexBox bbox() const = 0;

    virtual std::unique_ptr<PyImage> copy() const = 0;

    virtual ~PyImage() = default;

    static void wrap(py::class_<PyImage> & cls) {
        cls.def(py::init(&PyImage::construct), "box"_a, "dtype"_a);
        cls.def("__getitem__", &PyImage::getSubImage);
        cls.def("__getitem__", &PyImage::getScalar);
        cls.def("__setitem__", &PyImage::setScalar);
        cls.def("copy", &PyImage::copy);
        cls.def_property_readonly("bbox", &PyImage::bbox);
        cls.def_property_readonly("dtype", &PyImage::dtype);
        cls.def_property("array", &PyImage::getArray, &PyImage::setArray);
    }

protected:

    static py::capsule convertArrayBase(ImageOwner owner) {
        return py::capsule(new ImageOwner(std::move(owner)), &deleteImageOwner);
    }

private:

    static void deleteImageOwner(void * p) {
        delete reinterpret_cast<ImageOwner*>(p);
    }

    template <typename T>
    static std::unique_ptr<PyImage> tryConstruct(IndexBox const & box, py::dtype dtype) {
        auto const & api = py::detail::npy_api::get();
        if (api.PyArray_EquivTypes_(dtype.ptr(), py::dtype::of<T>().ptr())) {
            return convert(Image<T>(box));
        }
        return nullptr;
    }

    template <typename T>
    class Impl;
};


template <typename T>
void setImageScalar(Image<T> const & image, Index2 const & index, py::object value) {
    if (!image.bbox().contains(index)) {
        throw py::index_error(fmt::format("Index {} out of range for image with bbox {}",
                                          index, image.bbox()));
    }
    image[index] = py::cast<T>(value);
}

template <typename T>
void setImageScalar(Image<T const> const & image, Index2 const & index, py::object value) {
    PyErr_SetString(PyExc_TypeError, "Image is not writeable.");
    throw py::error_already_set();
}

template <typename T>
class PyImage::Impl : public PyImage {
public:

    Impl(Image<T> && wrapped) : _wrapped(wrapped) {}

    py::object getScalar(Index2 const & index) const override {
        if (!_wrapped.bbox().contains(index)) {
            throw py::index_error(fmt::format("Index {} out of range for image with bbox {}",
                                              index, _wrapped.bbox()));
        }
        return py::cast(_wrapped[index]);
    }

    void setScalar(Index2 const & index, py::object value) const override {
        setImageScalar(_wrapped, index, value);
    }

    std::unique_ptr<PyImage> getSubImage(IndexBox const & box) const override {
        if (!_wrapped.bbox().contains(box)) {
            throw py::index_error(fmt::format("Subimage bbox {} out of range for image with bbox {}",
                                              box, _wrapped.bbox()));
        }
        return convert(_wrapped[box]);
    }

    py::array getArray() const override {
        py::array result(
            {py::ssize_t(_wrapped.bbox().height()), py::ssize_t(_wrapped.bbox().width())},
            {py::ssize_t(_wrapped.stride()*sizeof(T)), py::ssize_t(sizeof(T))},
            _wrapped.data(),
            convertArrayBase(_wrapped.owner())
        );
        if (std::is_const<T>::value) {
            py::detail::array_proxy(result.ptr())->flags &= ~py::detail::npy_api::NPY_ARRAY_WRITEABLE_;
        }
        return result;
    }

    void setArray(py::object value) const override {
        py::array array = getArray();
        py::object slice(py::reinterpret_steal<py::object>(PySlice_New(nullptr, nullptr, nullptr)));
        array[py::make_tuple(slice, slice)] = value;
    }

    py::dtype dtype() const override {
        return py::dtype::of<T>();
    }

    IndexBox bbox() const override {
        return _wrapped.bbox();
    }

    std::unique_ptr<PyImage> copy() const override {
        return convert(_wrapped.copy());
    }

private:
    Image<T> _wrapped;
};


template <typename T>
std::unique_ptr<PyImage> PyImage::convert(Image<T> && wrapped) {
    return std::unique_ptr<PyImage>(new Impl<T>(std::move(wrapped)));
}

template <typename T>
Image<T> * PyImage::extract() {
    Impl<T> * p = dynamic_cast<Impl<T>*>(this);
    if (p == nullptr && std::is_const<T>::value) {
        p = dynamic_cast<Impl<std::remove_const_t<T>>*>(this);
    }
    return p;
}

} // anonymous


utils::Deferrer pyImage(py::module & module) {
    utils::Deferrer helper;
    helper.add(
        py::class_<PyImage>(module, "Image"),
        &PyImage::wrap
    );
    return helper;
}

} // namespace cipells
