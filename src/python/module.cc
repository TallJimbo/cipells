#include "cipells/python.h"

PYBIND11_MODULE(_cipells, m) {
    auto pyXYTuple = cipells::pyXYTuple(m);
    auto pyInterval = cipells::pyInterval(m);
    auto pyBox = cipells::pyBox(m);
    auto pyTransforms = cipells::pyTransforms(m);
    auto pyImage = cipells::pyImage(m);
    auto pyInterpolant = cipells::pyInterpolant(m);
    auto pyKernel = cipells::pyKernel(m);
    auto pyProfiles = cipells::pyProfiles(m);
}
