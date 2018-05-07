#include "pybind11/pybind11.h"
#include "pybind11/numpy.h"

#include <memory>
#include <type_traits>

#include "cipells/python.h"
#include "cipells/python/Image.h"
#include "cipells/Image.h"
#include "fmt/format.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace cipells {

namespace {

// Helper class for extracting a numpy array view from an Image<T>.
class ImageArrayHelper {
public:

    template <typename T>
    static py::object call(Image<T> const & self) {
        py::array result(
            {py::ssize_t(self.bbox().height()), py::ssize_t(self.bbox().width())},
            {py::ssize_t(self.stride()*sizeof(T)), py::ssize_t(sizeof(T))},
            self.data(),
            manager(self.owner())
        );
        if (std::is_const<T>::value) {
            py::detail::array_proxy(result.ptr())->flags &= ~py::detail::npy_api::NPY_ARRAY_WRITEABLE_;
        }
        return result;
    }


private:

    static void deleter(void * p) {
        delete reinterpret_cast<ImageOwner*>(p);
    }

    static py::capsule manager(ImageOwner owner) {
        return py::capsule(new ImageOwner(std::move(owner)), &deleter);
    }

};

// Python API for Image<T>.  This API is only used by the forwarding methods
// in PyImage's own wrappers (near the bottom of this file).
template <typename T, typename ...Args>
void wrapImage(py::class_<Image<T>, Args...> & cls) {
    using Mutable = Image<std::remove_cv_t<T>>;
    cls.def(
        "__getitem__",
        [](Image<T> const & self, Index2 const & index) {
            if (!self.bbox().contains(index)) {
                throw py::index_error(fmt::format("Index {} out of range for image with bbox {}",
                                                  index, self.bbox()));
            }
            return self[index];
        }
    );
    cls.def(
        "__setitem__",
        [](Mutable const & self, Index2 const & index, T value) {
            if (!self.bbox().contains(index)) {
                throw py::index_error(fmt::format("Index {} out of range for image with bbox {}",
                                                  index, self.bbox()));
            }
            return self[index] = value;
        }
    );
    cls.def(
        "__getitem__",
        [](Image<T> const & self, IndexBox const & box) {
            if (!self.bbox().contains(box)) {
                throw py::index_error(fmt::format("Subimage bbox {} out of range for image with bbox {}",
                                                  box, self.bbox()));
            }
            return self[box];
        }
    );
    cls.def("copy", &Image<T>::copy);
    cls.def_property_readonly(
        "bbox",
        [](Image<T> const & self) -> IndexBox { return self.bbox(); }  // lambda forces copy instead of ref
    );
    cls.def_property_readonly("array", &ImageArrayHelper::call<T>);
}

// Helper class for the only Python Image constructor - we compare the given
// np.dtype object against our supported types, and try to construct a
// PyImage with an internal Image<T> that matches it.
class PyImageInitHelper {
public:

    static PyImage call(IndexBox const & bbox, py::object dtype) {
        PyImageInitHelper helper(bbox, dtype);
        if (helper.attempt<float>()) return helper.finish();
        if (helper.attempt<std::complex<float>>()) return helper.finish();
        PyErr_SetString(PyExc_TypeError, "dtype not supported");
        throw py::error_already_set();
    }

private:

    PyImageInitHelper(IndexBox const & bbox, py::object dtype) :
        _bbox(bbox), _dtype(py::dtype::from_args(dtype)),
        _wrapped(), _np(py::detail::npy_api::get())
    {}

    PyImageInitHelper(PyImageInitHelper const &) = delete;
    PyImageInitHelper(PyImageInitHelper &&) = delete;

    PyImageInitHelper & operator=(PyImageInitHelper const &) = delete;
    PyImageInitHelper & operator=(PyImageInitHelper &&) = delete;

    template <typename T>
    bool attempt() {
        if (_np.PyArray_EquivTypes_(_dtype.ptr(), py::dtype::of<T>().ptr())) {
            _wrapped = py::reinterpret_steal<py::object>(
                py::detail::type_caster_base<Image<T>>::cast(
                    Image<T>(_bbox), py::return_value_policy::move, py::handle()
                )
            );
            return true;
        }
        return false;
    }

    PyImage finish() {
        return PyImage(std::move(_wrapped));
    }

    IndexBox _bbox;
    py::dtype _dtype;
    py::object _wrapped;
    py::detail::npy_api const & _np;
};


} // anonymous


utils::Deferrer pyImage(py::module & module) {
    utils::Deferrer helper;
    // Wrappers for PyImage, the public Python face of all Image<T> instantiations.
    helper.add(
        py::class_<PyImage>(module, "Image"),
        [](auto & cls) {
            cls.def(py::init(&PyImageInitHelper::call), "bbox"_a, "dtype"_a);
            cls.def_property(
                "array",
                [](PyImage & self) -> py::object {
                    if (!self.array) {
                        self.array = self.wrapped.attr("array");
                    }
                    return self.array;
                },
                [](PyImage & self, py::object value) -> void {
                    if (!self.array) {
                        self.array = self.wrapped.attr("array");
                    }
                    py::object slice(
                        py::reinterpret_steal<py::object>(PySlice_New(nullptr, nullptr, nullptr))
                    );
                    self.array[py::make_tuple(slice, slice)] = value;
                }
            );
            cls.def_property_readonly(
                "dtype",
                [](PyImage & self) -> py::object {
                    if (!self.array) {
                        self.array = self.wrapped.attr("array");
                    }
                    return self.array.attr("dtype");
                }
            );
            cls.def(
                "__getitem__",
                [](PyImage const & self, py::object key) {
                    return self.wrapped[key];
                }
            );
            cls.def(
                "__setitem__",
                [](PyImage const & self, py::object key, py::object value) {
                    self.wrapped[key] = value;
                }
            );
            cls.def("copy", [](PyImage const & self) { return self.wrapped.attr("copy")(); });
            cls.def_property_readonly(
                "bbox",
                [](PyImage const & self) { return self.wrapped.attr("bbox"); }
            );
        }
    );
    // Wrappers for the private Image<T> Python APIs.
    helper.add(
        py::class_<Image<float const>>(module, "_ImageFloatConst"),
        [](auto & cls) { wrapImage(cls); }
    );
    helper.add(
        py::class_<Image<float>, Image<float const>>(module, "_ImageFloat"),
        [](auto & cls) { wrapImage(cls); }
    );
    helper.add(
        py::class_<Image<std::complex<float> const>>(module, "_ImageComplexFloatConst"),
        [](auto & cls) { wrapImage(cls); }
    );
    helper.add(
        py::class_<Image<std::complex<float>>, Image<std::complex<float> const>>(
            module,
            "_ImageComplexFloat"
        ),
        [](auto & cls) { wrapImage(cls); }
    );
    return helper;
}

} // namespace cipells
