#include "pybind11/pybind11.h"
#include "pybind11/numpy.h"

#include "cipells/python.h"
#include "cipells/profiles.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace cipells {

utils::Deferrer pyProfiles(py::module & module) {
    utils::Deferrer helper;
    helper.add(
        py::class_<Gaussian>(module, "Gaussian"),
        [](auto & cls) {
            cls.def(py::init<Affine const &, Real>(), "transform"_a, "flux"_a=1);
            cls.def(
                "__call__",
                py::vectorize(
                    [](Gaussian const & self, Real x, Real y) {
                        return self(Real2(x, y));
                    }
                ),
                "x"_a, "y"_a
            );
            cls.def(
                "transformedBy",
                &Gaussian::transformedBy
            );
        }
    );
    return helper;
}

} // namespace cipells

