#include "pybind11/pybind11.h"
#include "pybind11/numpy.h"

#include "cipells/python.h"
#include "cipells/Kernel.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace cipells {

utils::Deferrer pyKernel(py::module & module) {
    utils::Deferrer helper;
    helper.add(
        py::class_<Kernel>(module, "Kernel"),
        [](auto & cls) {
            cls.def(py::init<Image<float const> const &, Index, std::shared_ptr<Interpolant const>>(),
                    "image"_a, "upsampling"_a=1, "interpolant"_a=nullptr);
            cls.def_property_readonly("image", &Kernel::image);
            cls.def_property_readonly("upsampling", &Kernel::upsampling);
            cls.def_property_readonly("interpolant", &Kernel::interpolant);
            cls.def("resample", &Kernel::resample, "upsampling"_a, "interpolant"_a=nullptr);
            cls.def("warp", &Kernel::warp, "transform"_a, "bbox"_a, "upsampling"_a=1,
                    "interpolant"_a=nullptr);
        }
    );
    return helper;
}

} // namespace cipells

